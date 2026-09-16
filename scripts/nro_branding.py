"""Check the actual JPEG icon and display metadata embedded in a Switch NRO.

Layout references: https://switchbrew.org/wiki/NRO0 and https://switchbrew.org/wiki/NACP
"""
import hashlib
import struct


def check_jpeg(data):
    if not data.startswith(b'\xff\xd8') or not data.endswith(b'\xff\xd9'):
        raise ValueError('Icon must be a complete JPEG')
    if len(data) > 128 * 1024:
        raise ValueError('Icon JPEG exceeds 128 KiB')
    cursor = 2
    dimensions = None
    while cursor < len(data) - 2:
        if data[cursor] != 0xff:
            raise ValueError('Invalid JPEG marker')
        while cursor < len(data) and data[cursor] == 0xff:
            cursor += 1
        marker = data[cursor]
        cursor += 1
        if cursor + 2 > len(data):
            raise ValueError('Truncated JPEG marker')
        length = struct.unpack_from('>H', data, cursor)[0]
        if length < 2 or cursor + length > len(data):
            raise ValueError('Truncated JPEG segment')
        if marker == 0xda:
            break
        if marker in (0xc0, 0xc1, 0xc2, 0xc3, 0xc5, 0xc6, 0xc7, 0xc9, 0xca, 0xcb, 0xcd, 0xce, 0xcf):
            if marker != 0xc0 or length < 8:
                raise ValueError('Icon must use baseline JPEG encoding')
            precision, height, width, components = struct.unpack_from('>BHHB', data, cursor + 2)
            if (precision, width, height, components) != (8, 256, 256, 3):
                raise ValueError('Icon must be an 8-bit, 256x256, three-component JPEG')
            dimensions = {'width': width, 'height': height}
        cursor += length
    if dimensions is None:
        raise ValueError('Missing JPEG frame')
    return {**dimensions, 'size': len(data), 'sha256': hashlib.sha256(data).hexdigest()}


def embedded_metadata(path):
    data = path.read_bytes()
    if len(data) < 0x80 or data[0x10:0x14] != b'NRO0':
        raise ValueError('Invalid NRO header')
    base = struct.unpack_from('<I', data, 0x18)[0]
    if base < 0x80 or base + 56 > len(data) or data[base:base + 4] != b'ASET':
        raise ValueError('Missing NRO asset header')

    def section(field):
        offset, length = struct.unpack_from('<QQ', data, base + field)
        if offset < 56 or length == 0 or base + offset + length > len(data):
            raise ValueError('Missing or invalid NRO metadata section')
        return data[base + offset:base + offset + length]

    icon = section(8)
    nacp = section(24)
    if len(nacp) != 0x4000:
        raise ValueError('NACP must be 0x4000 bytes')
    text = lambda start, end: nacp[start:end].split(b'\0', 1)[0].decode('utf-8')
    return {'icon': check_jpeg(icon), 'name': text(0, 0x200),
            'author': text(0x200, 0x300), 'version': text(0x3060, 0x3070)}


def check_branding(nro, icon, version):
    expected_icon = check_jpeg(icon.read_bytes())
    actual = embedded_metadata(nro)
    if actual['icon'] != expected_icon:
        raise ValueError('NRO contains a different icon from branding/icon.jpg')
    for key in ('name', 'author', 'version'):
        if actual[key] != version[key]:
            raise ValueError(f'NRO {key} differs from version.json')
    return expected_icon
