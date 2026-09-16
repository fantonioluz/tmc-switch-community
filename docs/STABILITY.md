# Gatos, travamentos e relatórios de erro

---

## Índice

- [Introdução](#introdução)
- [Plano](#plano)
- [Locais do código](#locais-do-código)
- [TODO](#todo)
- [Limitações e bugs](#limitações-e-bugs)

## Introdução

Na quest do livro do Dr. Left, o mantenedor relatou congelamentos com música
tocando perto de um Minish e dos gatos, além de um crash nos gatos. A versão
**0.1.2-rc.1** corrige uma leitura inválida no ataque dos gatos e remove gravações
contínuas de diálogos no cartão. Ela também registra crashes e ausência de
progresso do jogo. **O novo percurso ainda precisa de teste no Switch.**

Os logs recebidos têm várias sessões anexadas e não contêm um dump de CPU.
O último callback registrado não prova que aquele NPC provocou a falha.

## Plano

### Ataque dos gatos

`gUnk_08111154` contém oito endereços GBA de 32 bits, mas `cat.c` declarava a
tabela como ponteiros nativos. No Switch, a primeira leitura juntava duas entradas
em `0x0811111c08111114`, um endereço inválido. Limitar o índice a oito entradas
não corrigia essa diferença de tamanho.

`Cat_GetAttackHitbox` agora lê quatro bytes por entrada, resolve o endereço na
ROM carregada e confere se o hitbox inteiro cabe nela. O teste usa a tabela real
do código e o dump USA local como referência: oito hitboxes, 14 combinações de
frame/direção, fim da animação e limites de memória. Nenhuma ROM integra o teste
distribuído; quem executa a suíte fornece seu próprio dump.

### Desempenho

O antigo `dialog_trace.log` usava um arquivo sem buffer a cada chamada de script.
O log enviado tinha cerca de 8,5 MB. Essa escrita durante o jogo foi substituída
por um histórico circular de 64 eventos em memória. Nenhuma gravação em cartão
ocorre nesse caminho. O ganho de FPS ou redução de engasgos ainda não foi medido
no console. A quantidade de trabalhadores de renderização permanece conforme a
base, pois o log com zero trabalhadores corresponde à configuração intencional.

### Relatórios em `SD:/switch/tmc/`

| Arquivo | Quando é gravado | Conteúdo |
|---|---|---|
| `diagnostics.log` | Uma vez ao iniciar | Versão e resultado da preparação dos relatórios/detector |
| `crash-last.log` | Exceção nativa de CPU | PC, LR, SP, FAR, registradores, base do módulo e contexto recente |
| `freeze-last.log` | Oito verificações de um segundo sem progresso | Última entidade, instrução de script e histórico de diálogos |

Os dois últimos arquivos podem existir vazios enquanto não houver incidente.
Um relatório anterior é preservado ao abrir novamente o jogo e substituído
somente por outro incidente do mesmo tipo. Há no máximo um relatório de
congelamento por execução. O detector deixa o jogo rodando e respeita a pausa
na chamada de HOME/suspensão. A escrita de crash usa arquivos abertos previamente,
buffers fixos e as funções de filesystem da libnx; não depende de `stderr`,
`malloc`, SDL ou da desmontagem normal do jogo.

O handler segue os pontos de extensão da
[libnx](https://github.com/switchbrew/libnx/blob/master/nx/source/runtime/init.c)
e seu [fluxo de exceções](https://github.com/switchbrew/libnx/blob/master/nx/source/runtime/exception.s).
Depois de tentar gravar o relatório, encerra o processo. Isso não promete um
segundo relatório do Atmosphère.

### Como testar

1. Faça backup do save e substitua apenas `switch/tmc/tmc.nro` pelo candidato.
2. Refaça a passagem pela lareira do Dr. Left, a casa amarela e o trecho dos gatos.
3. Experimente aproximar-se dos gatos pelas duas direções e provocar seus ataques.
4. Se congelar, aguarde pelo menos 12 segundos antes de fechar pelo HOME, se possível.
5. Envie `diagnostics.log`, `crash-last.log`, `freeze-last.log` e `tmc.log`, com
   o local e o que ocorreu. Informe se HOME respondeu e se a imagem ainda animava.

Para interpretar endereços de código, preserve o ELF exato do candidato. Subtraia
`module_base` de PC/LR e use `aarch64-none-elf-addr2line -f -C -e arquivo.elf`.
Não use símbolos de outra compilação. Os relatórios mostram números em hexadecimal.

## Locais do código

| Recurso | Local | Responsabilidade |
|---|---|---|
| Ataque dos gatos | `Cat_GetAttackHitbox` em `source/src/npc/cat.c` | Resolve tabela GBA de 32 bits e valida os limites |
| Histórico de diálogos | `source/port/port_dialog_trace.c` | Mantém os pontos de diagnóstico sem escrita por frame |
| Relatórios e detector | `source/platforms/switch/switch_diagnostics.c` | Histórico em RAM, exceções de CPU, detector e arquivos |
| Progresso e contexto | `source/port/port_bios.c`, `port_draw.c`, `source/src/script.c` | Informa frame, entidade e comando de script |
| HOME e suspensão | `source/platforms/switch/switch_applet.c` | Exclui a espera do sistema da detecção |
| Regressões | `source/tools/test_cat_regressions.py`, `test_diagnostics.py` | Código real com sanitizadores e serviços libnx simulados |

## TODO

- [ ] Confirmar ataques dos gatos e percurso completo no Switch.
- [ ] Medir engasgos no console depois de remover a escrita contínua.
- [ ] Confirmar gravação e preservação dos relatórios em hardware.
- [ ] Isolar a causa do congelamento perto do Minish se ele persistir.

## Limitações e bugs

- A correção dos gatos demonstra e elimina um acesso inválido específico; não
  comprova que todos os congelamentos relatados tinham a mesma causa.
- O detector identifica ausência de frames. Um jogo que continua gerando frames
  mas bloqueia controles por lógica de cutscene pode não gerar relatório.
- Uma espera legítima de mais de oito segundos fora da pausa monitorada pode
  gerar um falso positivo. Não há encerramento automático por congelamento.
- Remoção do cartão, falha de filesystem, falta de permissão para criar a thread,
  interrupção de energia ou travamento geral do sistema podem impedir o relatório.
- Os testes simulam os serviços libnx no computador; não substituem o teste real.
