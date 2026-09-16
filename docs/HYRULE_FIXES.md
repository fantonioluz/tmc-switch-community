# Hyrule: portas e interação com pisos

---

## Índice

- [Introdução](#introdução)
- [Plano e comportamento](#plano-e-comportamento)
- [Locais do código](#locais-do-código)
- [TODO](#todo)
- [Limitações e bugs](#limitações-e-bugs)

## Introdução

Havia dois defeitos distintos: portas fora de posição e pisos próximos às portas
que ficavam pretos enquanto uma espada surgia. As duas correções foram confirmadas
pelo mantenedor em hardware em 15/09/2026.

## Plano e comportamento

| Caso | Causa identificada | Comportamento corrigido |
| --- | --- | --- |
| Portas fora de posição | Uma tabela de propriedades era interrompida em fragmentos de assembly e ponteiros de script | O carregador lê a tabela completa a partir da ROM |
| Piso preto e espada | A tabela de interação tinha 84 pares sem terminador interno; o linker nativo descartava o zero declarado à parte | Um terminador dentro da tabela encerra a busca antes de outros dados |
| Leitura de superfície | A tabela de tipo de superfície também não tinha terminador interno | O port agora encerra a consulta dentro dessa tabela |

No ELF defeituoso, a busca terminava apenas na linha 517. Os bytes lidos fora da
tabela produziam o índice 256 numa tabela com 60 definições. Nessa região da
memória, os bytes eram interpretados como criação de `GROUND_ITEM`, tipo
`ITEM_SMITH_SWORD`, e substituição do piso por `0x100`. Esse tipo de piso não
existia no tileset de Hyrule; a conversão caía no índice zero e apagava o cenário.

O teste com a tabela antiga reproduziu `global-buffer-overflow`. Com a correção,
65.536 consultas coincidiram com a ROM USA, e 224 interações de pisos comuns
não criaram objetos nem modificaram o mapa. Corte de vegetação permaneceu válido.

A investigação consultou a [versão 1.1 do port de 3DS](https://github.com/EstebanPdN/zelda-tmc-3ds/releases/tag/v1.1),
que registra uma correção relacionada a pisos de portas tratados como vegetação.
O diagnóstico específico desta edição foi confirmado na memória do ELF local.

## Locais do código

| Recurso | Local | Descrição |
| --- | --- | --- |
| Propriedades das portas | `ResolveFragmentedRoomProperty` em `source/port/port_asset_loader.cpp` | Resolve a tabela fragmentada na ROM |
| Tabela de interação | `source/src/data/data_080046A4.c` | Terminador e quantidade real de definições |
| Superfícies | `source/src/data/mapActTileToSurfaceType.c` | Terminador da tabela de superfícies |
| Criação e substituição | `DoTileInteraction` em `source/port/port_gameplay_stubs.c` | Valida índice e ação antes de aplicar a interação |
| Consultas auxiliares | `sub_0806FC24` / `sub_0806FC50` em `source/src/physics.c` | Verificações equivalentes de limites |
| Testes | `source/tools/tests/tile_interactions.c` e `source/tools/test_door_render_regressions.py` | Protegem as correções |

## TODO

- [ ] Ampliar a validação de outras áreas durante a campanha.
- [ ] Registrar reproduções de outros defeitos antes de alterar tabelas.

## Limitações e bugs

Esses testes validam os caminhos descritos, não a campanha completa. O pacote
com os assets fornecidos exige uma validação adicional de primeira instalação no console.
