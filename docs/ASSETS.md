# Assets do pacote

---

## Índice

- [Introdução](#introdução)
- [Plano](#plano)
- [Locais do código](#locais-do-código)
- [TODO](#todo)
- [Limitações e bugs](#limitações-e-bugs)

## Introdução

A distribuição contém `tmc.nro` e os 21 arquivos de assets fornecidos pelo
mantenedor. Eles são copiados para `SD:/switch/tmc/assets/`. A ROM não acompanha
o pacote: cada jogador adiciona seu dump USA em `SD:/switch/tmc/baserom.gba`.
Os assets não substituem a ROM exigida pelo executável.

## Plano

Os nove PAKs e doze JSONs foram copiados sem alterar o conteúdo. Apenas o nome
`asset_build_state.json` foi ajustado para `.asset_build_state.json`, que é o
marcador procurado pelo carregador. Não remova o ponto inicial.

| Grupo | Arquivos |
| --- | --- |
| Arquivos compactados do port | `animations.pak`, `data.pak`, `gfx.pak`, `maps.pak`, `misc.pak`, `palettes.pak`, `room_props.pak`, `sprites.pak`, `tilemaps.pak` |
| Metadados de áreas | `area_room_headers.json`, `area_room_maps.json`, `area_tables.json`, `area_tile_sets.json`, `area_tiles.json` |
| Outros metadados | `gfx_groups.json`, `palette_groups.json`, `palettes.json`, `sprite_ptrs.json`, `texts.json`, `tilemaps.json` |
| Marcador do cache | `.asset_build_state.json` |

O marcador enviado declara cache `v1`, builder `2`, ROM de 16 MiB e `rom_mtime=0`.
No código atual, esse zero permite reutilizar o cache sem exigir a mesma data
de modificação da ROM do mantenedor. Esse marcador não verifica o SHA-1 da ROM;
use `scripts/verify_rom.py` para conferir seu dump USA. Não misture esse conjunto
com outra região ou ROM modificada.

O empacotador aceita somente os arquivos enumerados, valida cabeçalhos, índices
e limites dos nove PAKs, lê os JSONs e registra tamanho e SHA-256 de cada arquivo.
O `.gitignore` permite somente esses assets dentro da pasta de distribuição.
Os arquivos `.gba`, saves e dados pessoais continuam excluídos.

## Locais do código

| Recurso | Local | Descrição |
| --- | --- | --- |
| Assets distribuídos | `release/switch/tmc/assets/` | Conjunto fornecido pelo mantenedor |
| Lista e validação | `scripts/release_assets.py` | Formato PAK v1, JSON e hashes |
| ZIP e manifesto | `scripts/package_release.py` | Inclui NRO e assets externos |
| Reconhecimento do cache | `RuntimeAssetsUpToDateImpl` em `source/tools/src/assets_extractor/assets_extractor_api.cpp` | Procura o marcador com ponto inicial |
| Inicialização | `Port_EnsureAssetsReadyWithDisplay` em `source/port/port_asset_bootstrap.cpp` | Reutiliza ou regenera o cache |

## TODO

- [ ] Confirmar instalação nova desse conjunto com o NRO atualizado no Switch.
- [ ] Revalidar a biblioteca e Lake Hylia com o save do mantenedor.

## Limitações e bugs

A verificação dos arquivos confirma sua estrutura e integridade, não uma campanha
completa no console. Os assets são derivados do conteúdo do jogo original; sua
inclusão não transfere autoria ou concede uma nova licença sobre esse conteúdo.
Veja [Licenciamento](../LICENSE.md).
