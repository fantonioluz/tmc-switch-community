#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
if [[ $# -ne 1 ]]; then
    echo "Usage: bash scripts/test_regressions.sh /path/to/your/baserom.gba" >&2
    exit 2
fi
ROM="$(python3 -c 'from pathlib import Path; import sys; print(Path(sys.argv[1]).resolve())' "$1")"
cd "$ROOT"
python3 scripts/verify_rom.py "$ROM"
mkdir -p .local/tests
cd source
gcc -std=gnu11 -DPC_PORT -DUSA -DENGLISH -DNON_MATCHING -I. -Iinclude -Iport -Ibuild/USA \
    -O1 -g -ffunction-sections -fdata-sections -Wl,--gc-sections -fsanitize=address,undefined \
    tools/tests/tile_interactions.c src/data/data_080046A4.c src/data/mapActTileToSurfaceType.c \
    port/port_gameplay_stubs.c src/physics.c -o ../.local/tests/tile_interactions
../.local/tests/tile_interactions "$ROM"
python3 tools/test_npc_regressions.py --emit ../.local/tests/npc_regressions.c
gcc -std=gnu11 -DPC_PORT -DUSA -DENGLISH -DNON_MATCHING -DSCENE_TRACE=0 \
    -I. -Iinclude -Iport -Ibuild/USA -O1 -g -fsanitize=address,undefined \
    ../.local/tests/npc_regressions.c -o ../.local/tests/npc_regressions
../.local/tests/npc_regressions "$ROM"
python3 tools/test_door_render_regressions.py --emit ../.local/tests/door_regressions.cpp
g++ -std=c++17 -O1 -fsanitize=address,undefined ../.local/tests/door_regressions.cpp -o ../.local/tests/door_regressions
../.local/tests/door_regressions "$ROM"
python3 tools/test_asset_paths.py --emit ../.local/tests/asset_paths.cpp
g++ -std=c++17 -O1 -fsanitize=address,undefined ../.local/tests/asset_paths.cpp -o ../.local/tests/asset_paths
../.local/tests/asset_paths
