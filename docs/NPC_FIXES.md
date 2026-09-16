# Biblioteca e Lake Hylia: scripts e sprites dos NPCs

---

## Índice

- [Introdução](#introdução)
- [Plano](#plano)
- [Locais do código](#locais-do-código)
- [TODO](#todo)
- [Limitações e bugs](#limitações-e-bugs)

## Introdução

O jogador relatou um Minish sem opção de conversa na estante da biblioteca e
um crash ao entrar em Lake Hylia. A investigação usou a ROM USA do mantenedor,
o fluxo de criação dos NPCs e as notas públicas da edição de referência.

### Biblioteca: diferença confirmada na entrada da entidade

A entrada de 16 bytes em `0xDB820` da ROM USA tem flags `0x4f` e script
`0x0800E6E8` (`script_TownMinish1`). O array nativo tinha flags `0x0f`:
faltava `0x40`, que faz o carregador iniciar o script. Esse script chama
`TownMinish_MakeInteractable`. O NPC aparecia, mas o caminho que habilita
a conversa não era iniciado. A correção restaura `0x4f`.

### Lake Hylia: caminhos de crash documentados

As notas [v1.3.3](https://github.com/Alexgg1014/The-Legend-of-Zelda-The-Minish-Cap-Alek-s-Ultimate-NX-Edition/blob/main/RELEASE_NOTES_v1.3.3.md)
descrevem um NPC tipo 64 acessando uma tabela de apenas 21 entradas. As notas
[v1.3.6](https://github.com/Alexgg1014/The-Legend-of-Zelda-The-Minish-Cap-Alek-s-Ultimate-NX-Edition/blob/main/RELEASE_NOTES_v1.3.6.md)
descrevem contextos de script ausentes ou desatualizados em transições.
O código desta base ainda tinha acessos equivalentes sem proteção. Adicionamos
limites às consultas de Townsperson/Kid e usamos a tabela nativa de contextos
de script de Festari/Stockwell, limpando-a ao excluir entidades.

Em 16/09/2026, o mantenedor testou e confirmou que a conversa na biblioteca
e o acesso a Lake Hylia funcionaram no Switch. Não houve captura do crash antigo;
essa confirmação valida o resultado observado, sem isolar qual dos caminhos
protegidos causava aquele crash.

## Plano

| Cenário | Comportamento esperado | Verificação |
| --- | --- | --- |
| Minish da biblioteca | Carregar o script da ROM e habilitar a conversa | Entrada comparada byte a byte; carregador nativo inicia o script |
| Tipo de Townsperson/Kid fora da tabela | Retornar NULL sem ler memória adjacente | Todos os 256 tipos de cada tabela, com ASan/UBSan |
| Festari com espelho de contexto inválido | Usar o contexto da tabela nativa ou aguardar | Contexto válido com espelho NULL; contexto ausente com espelho inválido |
| Entidade excluída | Remover contexto da tabela nativa | Revisão do caminho comum de exclusão e build Switch |
| Caminho `sdmc:` durante extração | Calcular nome relativo sem canonicalização | Cinco casos de caminho e build Switch |

O teste da biblioteca falha com a antiga flag `0x0f`. A remoção da proteção de
tipos reproduz leitura fora dos limites sob ASan/UBSan. Os testes passam com
as correções. O teste de contexto executa a função de produção de Festari;
Stockwell usa o mesmo acesso, mas não é simulado separadamente nessa suíte.

A edição de referência também registra um problema de canonicalização de
`sdmc:` nas [notas v0.1.1](https://github.com/Alexgg1014/The-Legend-of-Zelda-The-Minish-Cap-Alek-s-Ultimate-NX-Edition/blob/main/RELEASE_NOTES_v0.1.1.md).
O helper de caminhos foi corrigido nos dois pontos da montagem de assets;
nos outros sistemas operacionais, o comportamento anterior foi preservado.

## Locais do código

| Recurso | Local | Descrição |
| --- | --- | --- |
| Minish da estante | `gUnk_additional_a_TownMinishHoles_LibraryBookshelf` em `source/src/roomInit.c` | Flags conforme a ROM |
| Tabelas de sprites | `Townsperson_GetSpriteLoadData` / `Kid_GetSpriteLoadData` em `source/src/npc/` | Verificam tipos antes da indexação |
| Tamanho da tabela | `gTownspersonSpriteLoadDataCount` em `source/port/data_stubs_autogen.c` | Calculado a partir do array real |
| Contextos de script | `source/src/npc/festari.c`, `source/src/npc/stockwell.c`, `DeleteEntity` em `source/src/entity.c` | Consulta e limpeza da tabela nativa |
| Caminhos de assets | `RelativeAssetPath` em `source/port/port_asset_pipeline.cpp` | Cálculo lexical específico do Switch |
| Regressões | `source/tools/test_npc_regressions.py`, `source/tools/test_asset_paths.py` | Trechos reais de produção em testes nativos |

## TODO

- [x] Conversa com o Minish da estante confirmada pelo mantenedor em 16/09/2026.
- [x] Fix de Lake Hylia confirmado pelo mantenedor em 16/09/2026.
- [ ] Confirmar instalação nova com ROM USA e os assets incluídos; testar também a regeneração sem cache.
- [ ] Se o crash continuar, registrar passos exatos e analisar o log/crash dump antes de atribuir outra causa.

## Limitações e bugs

A proteção de índices evita a leitura inválida; ela não determina por que um
tipo inesperado pode chegar a essa tabela. Não foi incorporada uma migração
completa de todas as estruturas de NPCs da edição de referência. As duas correções foram
confirmadas pelo mantenedor no Switch. A campanha completa e uma instalação nova
continuam em validação.
