# Estado da validação

## Corrigido e confirmado no Switch

- Portas de Hyrule fora de posição por leitura incompleta de uma tabela fragmentada.
- Pisos pretos e espadas surgindo perto de portas por leitura além do fim da tabela de interações.

Confirmação do mantenedor em 15/09/2026. Detalhes em [Correções de Hyrule](HYRULE_FIXES.md).

## Biblioteca e Lake Hylia confirmados no Switch

- Biblioteca: o mantenedor confirmou em 16/09/2026 que o Minish da estante permite conversar.
- Lake Hylia: o mantenedor confirmou em 16/09/2026 que o fix funcionou ao acessar a área.

Os testes de memória também passaram. A confirmação é do comportamento corrigido;
sem crash dump anterior, não isolamos qual proteção eliminou aquele crash.
Veja [Investigação dos NPCs](NPC_FIXES.md).

## Ícone da versão 0.1.1

O JPEG aprovado está embutido no NRO, e sua identidade foi verificada byte a byte.
A aparência do novo ícone no console e em um atalho ainda precisa de confirmação.
Veja [Ícone e atalhos](ICON.md).

## Quest do Dr. Left: candidato 0.1.2-rc.1

- Corrigida a leitura de ponteiros GBA no ataque dos gatos; regressão de memória aprovada.
- Removida a gravação contínua de diálogos no cartão; ganho de desempenho ainda não medido no Switch.
- Adicionados `crash-last.log`, `freeze-last.log` e `diagnostics.log`; testes simulados aprovados.
- O congelamento perto do Minish continua com causa não confirmada. O percurso e os relatórios precisam de validação no console.

Veja [investigação e instruções de teste](STABILITY.md). Esta versão é um candidato de teste.

## Fonte após os braceletes: diagnóstico 0.1.2-rc.2

O novo relatório confirmou ausência de frames ao voltar para Hyrule na rc.1.
A causa continua sem confirmação. A rc.2 acrescenta etapa de execução e captura
opcional de PC/LR para localizar o bloqueio, mantendo o histórico em memória.
Não é anunciada como correção da saída da fonte. Veja [Estabilidade](STABILITY.md).

## Limitações atuais

- A campanha completa ainda não foi validada nesta edição.
- A base de referência é USA; outras regiões e ROMs modificadas não foram validadas.
- Uma instalação nova do NRO atualizado com os assets fornecidos ainda precisa de confirmação em hardware.
- Se o cache estiver ausente ou incompatível, o jogo pode extraí-lo novamente; aguarde a conclusão.
- A compilação preserva avisos existentes no port; não houve revisão integral de todos os sistemas.

## Se algo falhar

Informe versão, local, passos para reproduzir, região da ROM e se a instalação
é nova ou reaproveita assets. Uma foto ajuda em problemas visuais. O build inicial
mantém diagnóstico; se `tmc.log` existir em `switch/tmc/`, confira seu conteúdo
antes de compartilhar. Não publique ROMs, dados de conta,
configurações com credenciais ou saves pessoais em Issues públicas.
