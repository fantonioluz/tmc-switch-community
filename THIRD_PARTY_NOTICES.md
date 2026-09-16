# Componentes de terceiros

| Componente | Origem / licença | Arquivos locais |
| --- | --- | --- |
| Decompilação e port | Autores listados em [CREDITS.md](CREDITS.md); sem licença global na raiz da base | `source/` |
| agbplay | LGPL-3.0, ipatix e contribuidores | `source/libs/agbplay_core/`, `LICENSES/` |
| rcheevos | MIT, RetroAchievements.org | `source/libs/rcheevos/LICENSE` |
| fmt 10.2.1 | MIT, Victor Zverovich e contribuidores | Aviso no início de `source/platforms/switch/compat/fmt/format.h` |
| nlohmann/json 3.11.3 | MIT, Niels Lohmann | `LICENSES/nlohmann-json-MIT.txt`, cabeçalho `source/platforms/switch/compat/nlohmann/json.hpp` |
| VirtuaPPU / VirtuaAPU | Avisos e fontes upstream preservados; sem LICENSE separado nos snapshots | `source/libs/ViruaPPU/`, `source/libs/VirtuaAPU/` |
| Ferramentas auxiliares GBA | Licenças próprias preservadas | `source/tools/src/*/LICENSE` e `COPYING` |

O executável também utiliza as bibliotecas da toolchain devkitPro/libnx, SDL2,
Mesa/EGL, libdrm, curl, mbedTLS, zlib e libpng. As versões e textos disponíveis na
toolchain usada estão registrados em `LICENSES/TOOLCHAIN_NOTICES.md` e no diretório
`LICENSES/toolchain/`. Reutilize as licenças correspondentes à sua toolchain ao
gerar versões futuras.

Esta lista descreve a procedência dos componentes; ela não substitui os textos
de licença e não estende licenças de bibliotecas ao conteúdo do jogo original.
