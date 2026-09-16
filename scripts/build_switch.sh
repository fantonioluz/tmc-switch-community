#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
: "${DEVKITPRO:?Set DEVKITPRO to your devkitPro installation}"
export PATH="$DEVKITPRO/tools/bin:$DEVKITPRO/devkitA64/bin:$PATH"
cd "$ROOT"
python3 source/platforms/switch/gen_sources.py
make -C source/platforms/switch -j"${JOBS:-4}" REGION=USA
python3 scripts/package_release.py
python3 scripts/validate_repository.py
