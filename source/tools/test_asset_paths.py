"""Emit a native test of the Switch asset path helper (no ROM required)."""
import argparse
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def harness():
    source = (ROOT / 'port/port_asset_pipeline.cpp').read_text(encoding='utf-8')
    start = source.index('std::filesystem::path RelativeAssetPath(')
    end = source.index('\n}', start) + 2
    return '''#include <filesystem>
#include <cassert>
#include <cstdio>
#define __SWITCH__ 1
''' + source[start:end] + r'''
int main() {
    struct Case { const char* path; const char* root; const char* expected; };
    const Case cases[] = {
        {"sdmc:/switch/tmc/assets_src/maps/room.bin", "sdmc:/switch/tmc/assets_src", "maps/room.bin"},
        {"sdmc:/switch/tmc/assets_src/./maps/../tiles/a.bin", "sdmc:/switch/tmc/assets_src/", "tiles/a.bin"},
        {"assets_src/maps/room.bin", "assets_src", "maps/room.bin"},
        {"./assets_src/palette.json", "./assets_src", "palette.json"},
        {"/switch/tmc/assets_src/a.bin", "/switch/tmc/assets_src", "a.bin"},
    };
    for (const auto& c : cases) {
        auto result = RelativeAssetPath(c.path, c.root);
        assert(result.generic_string() == c.expected);
        assert((std::filesystem::path(c.root) / result).lexically_normal() ==
               std::filesystem::path(c.path).lexically_normal());
    }
    puts("PASS: 5 Switch asset path cases; no filesystem canonicalization required");
}
'''


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--emit', type=Path, required=True)
    args = parser.parse_args()
    args.emit.write_text(harness(), encoding='utf-8')
