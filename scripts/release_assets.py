"""Validate the explicit set of runtime assets supplied for this release."""
import hashlib
import json
from pathlib import Path, PurePosixPath
import struct

ASSET_NAMES = (
    '.asset_build_state.json', 'animations.pak', 'area_room_headers.json',
    'area_room_maps.json', 'area_tables.json', 'area_tile_sets.json', 'area_tiles.json',
    'data.pak', 'gfx.pak', 'gfx_groups.json', 'maps.pak', 'misc.pak',
    'palette_groups.json', 'palettes.json', 'palettes.pak', 'room_props.pak',
    'sprite_ptrs.json', 'sprites.pak', 'texts.json', 'tilemaps.json', 'tilemaps.pak',
)


def fingerprint(path):
    data = path.read_bytes()
    return {'size': len(data), 'sha256': hashlib.sha256(data).hexdigest()}


def inspect_pak(path):
    data = path.read_bytes()
    if len(data) < 40:
        raise ValueError(f'Truncated PAK: {path.name}')
    magic, version, count, names, names_size, payload, size, flags, reserved = struct.unpack_from('<6IQ2I', data)
    if (magic != 0x50434D54 or version != 1 or flags != 1 or reserved != 0 or
            names != 40 + count * 24 or names + names_size > payload or payload + size != len(data)):
        raise ValueError(f'Invalid PAK header: {path.name}')
    previous = None
    for i in range(count):
        name_offset, name_size, offset, length, unused = struct.unpack_from('<IIQII', data, 40 + i * 24)
        if name_offset + name_size > names_size or offset < payload or offset + length > len(data) or unused:
            raise ValueError(f'Invalid PAK entry: {path.name} #{i}')
        name = data[names + name_offset:names + name_offset + name_size].decode('utf-8')
        entry = PurePosixPath(name)
        if (not name or '\\' in name or ':' in name or entry.is_absolute() or '..' in entry.parts or
                entry.suffix.lower() in {'.gba', '.gb', '.gbc', '.nds', '.sav'}):
            raise ValueError(f'Unexpected PAK path: {path.name} #{i}')
        if previous is not None and name <= previous:
            raise ValueError(f'Unsorted/duplicate PAK entry: {path.name} #{i}')
        previous = name
    return count


def asset_inventory(directory):
    actual = {p.name for p in directory.iterdir()}
    if actual != set(ASSET_NAMES):
        raise ValueError(f'Asset set mismatch: missing={set(ASSET_NAMES) - actual}, extra={actual - set(ASSET_NAMES)}')
    inventory = []
    for name in ASSET_NAMES:
        path = directory / name
        record = {'file': 'switch/tmc/assets/' + name, **fingerprint(path)}
        if path.suffix == '.pak':
            record['entries'] = inspect_pak(path)
        else:
            value = json.loads(path.read_text(encoding='utf-8'))
            if name == '.asset_build_state.json':
                if (value.get('format') != 'tmc_asset_build_state_v1' or value.get('builder_version') != 2 or
                        value.get('pack_format') != 'v1' or value.get('rom_size') != 16777216):
                    raise ValueError('Unexpected runtime cache state')
        inventory.append(record)
    return inventory
