import struct
import json
from nro_branding import check_branding, check_jpeg
import tempfile
from pathlib import Path
import unittest

from package_release import ROOT, inspect_nro
from verify_rom import inspect_rom
from release_assets import asset_inventory, inspect_pak


class PackageChecks(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.work = ROOT / '.local' / 'tool-tests'
        cls.work.mkdir(parents=True, exist_ok=True)

    def sample(self, romfs=False):
        data = bytearray(0x80 + 56 + (4 if romfs else 0))
        data[0x10:0x14] = b'NRO0'
        struct.pack_into('<I', data, 0x18, 0x80)
        data[0x80:0x84] = b'ASET'
        if romfs:
            struct.pack_into('<QQ', data, 0x80 + 40, 56, 4)
        return data

    def check_file(self, data):
        with tempfile.TemporaryDirectory(dir=self.work) as directory:
            path = Path(directory) / 'sample.nro'
            path.write_bytes(data)
            return inspect_nro(path)

    def test_rejects_embedded_romfs(self):
        with self.assertRaisesRegex(ValueError, 'RomFS'):
            self.check_file(self.sample(True))

    def test_accepts_no_romfs(self):
        self.assertEqual(self.check_file(self.sample())['romfs_size'], 0)

    def test_rejects_truncated_nro(self):
        with self.assertRaises(ValueError):
            self.check_file(self.sample()[:100])

    def test_rejects_invalid_asset_bounds(self):
        data = self.sample()
        struct.pack_into('<QQ', data, 0x80 + 8, 10000, 100)
        with self.assertRaisesRegex(ValueError, 'outside'):
            self.check_file(data)

    def test_short_rom_is_read_safely(self):
        with tempfile.TemporaryDirectory(dir=self.work) as directory:
            path = Path(directory) / 'short.gba'
            path.write_bytes(b'not a cartridge dump')
            result = inspect_rom(path)
            self.assertEqual(result['header'], '')
            self.assertEqual(path.read_bytes(), b'not a cartridge dump')

    def test_rejects_truncated_pak(self):
        with tempfile.TemporaryDirectory(dir=self.work) as directory:
            path = Path(directory) / 'bad.pak'
            path.write_bytes(b'TMCP')
            with self.assertRaisesRegex(ValueError, 'Truncated'):
                inspect_pak(path)

    def test_rejects_pak_entry_outside_payload(self):
        with tempfile.TemporaryDirectory(dir=self.work) as directory:
            path = Path(directory) / 'bad.pak'
            header = struct.pack('<6IQ2I', 0x50434D54, 1, 1, 64, 5, 69, 1, 1, 0)
            path.write_bytes(header + struct.pack('<IIQII', 0, 5, 69, 100, 0) + b'a.bin' + b'x')
            with self.assertRaisesRegex(ValueError, 'Invalid PAK entry'):
                inspect_pak(path)

    def test_asset_allowlist_rejects_rom_filename(self):
        with tempfile.TemporaryDirectory(dir=self.work) as directory:
            folder = Path(directory)
            (folder / 'baserom.gba').write_bytes(b'test only, not a ROM')
            with self.assertRaisesRegex(ValueError, 'Asset set mismatch'):
                asset_inventory(folder)


    def branded_nro(self, version='0.1.1', icon=None):
        if icon is None:
            icon = (ROOT / 'branding/icon.jpg').read_bytes()
        nacp = bytearray(0x4000)
        expected = json.loads((ROOT / 'version.json').read_text())
        for offset, value in ((0, expected['name']), (0x200, expected['author']), (0x3060, version)):
            encoded = value.encode('utf-8')
            nacp[offset:offset + len(encoded)] = encoded
        base = bytearray(0x80)
        base[0x10:0x14] = b'NRO0'
        struct.pack_into('<I', base, 0x18, 0x80)
        header = b'ASET' + struct.pack('<I6Q', 0, 56, len(icon), 56 + len(icon), len(nacp), 0, 0)
        return base + header + icon + nacp

    def check_branded_file(self, data):
        with tempfile.TemporaryDirectory(dir=self.work) as directory:
            path = Path(directory) / 'sample.nro'
            path.write_bytes(data)
            return check_branding(path, ROOT / 'branding/icon.jpg', json.loads((ROOT / 'version.json').read_text()))

    def test_accepts_embedded_project_icon_and_version(self):
        current = json.loads((ROOT / 'version.json').read_text())['version']
        self.assertEqual(self.check_branded_file(self.branded_nro(current))['width'], 256)

    def test_rejects_old_embedded_version(self):
        with self.assertRaisesRegex(ValueError, 'version differs'):
            self.check_branded_file(self.branded_nro('0.0.0'))

    def test_rejects_different_embedded_icon(self):
        icon = bytearray((ROOT / 'branding/icon.jpg').read_bytes())
        # Change a byte inside a quantization table, retaining a valid JPEG header.
        table = icon.index(b'\xff\xdb')
        icon[table + 5] ^= 1
        with self.assertRaisesRegex(ValueError, 'different icon'):
            self.check_branded_file(self.branded_nro(icon=icon))

    def test_rejects_wrong_icon_dimensions(self):
        icon = bytearray((ROOT / 'branding/icon.jpg').read_bytes())
        frame = icon.index(b'\xff\xc0')
        struct.pack_into('>H', icon, frame + 5, 128)
        with self.assertRaisesRegex(ValueError, '256x256'):
            check_jpeg(icon)


if __name__ == '__main__':
    unittest.main()
