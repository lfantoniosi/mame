#!/usr/bin/env python3
"""Update MAME's WonderTANG Nextor ROM archive and ROM declaration."""

from __future__ import annotations

import hashlib
import os
from pathlib import Path
import re
import stat
import tempfile
import zipfile
import zlib


REPO_ROOT = Path(__file__).resolve().parent.parent
SOURCE_ROM = REPO_ROOT.parent / "fpga/kernel/Nextor/source/drivers/Nextor-3.0.0-beta1.WonderTANG.ROM"
ROM_ARCHIVE = REPO_ROOT / "roms/msx_cart_wondertang.zip"
DEVICE_SOURCE = REPO_ROOT / "src/devices/bus/msx/cart/slotexpander.cpp"
LEGACY_ROM_NAME = "nextor-2.1.1.wondertang.rom.bin"
EXPECTED_ROM_SIZE = 0x20000


def update_archive(rom_data: bytes) -> bool:
	"""Replace the archived Nextor ROM if its bytes (or name) changed."""
	with zipfile.ZipFile(ROM_ARCHIVE, "r") as archive:
		infos = archive.infolist()
		current = [info for info in infos if info.filename == SOURCE_ROM.name]
		legacy_present = any(info.filename == LEGACY_ROM_NAME for info in infos)
		if len(current) == 1 and archive.read(current[0]) == rom_data and not legacy_present:
			return False

		fd, temporary_name = tempfile.mkstemp(
			prefix=f".{ROM_ARCHIVE.name}.", suffix=".tmp", dir=ROM_ARCHIVE.parent
		)
		os.close(fd)
		temporary = Path(temporary_name)
		try:
			with zipfile.ZipFile(temporary, "w") as updated:
				updated.comment = archive.comment
				for info in infos:
					if info.filename not in (SOURCE_ROM.name, LEGACY_ROM_NAME):
						updated.writestr(info, archive.read(info))
				updated.write(SOURCE_ROM, SOURCE_ROM.name, compress_type=zipfile.ZIP_DEFLATED)

			mode = stat.S_IMODE(ROM_ARCHIVE.stat().st_mode)
			os.chmod(temporary, mode)
			os.replace(temporary, ROM_ARCHIVE)
		finally:
			temporary.unlink(missing_ok=True)
	return True


def update_device_source(crc: str, sha1: str) -> bool:
	contents = DEVICE_SOURCE.read_text()
	pattern = re.compile(
		r'(ROM_REGION\(0x20000, "dos", 0\)\s*\n\s*ROM_LOAD\(")[^"]+'
		r'(", 0, 0x20000, CRC\()[0-9a-fA-F]+(\) SHA1\()[0-9a-fA-F]+(\)\))'
	)
	replacement = rf'\g<1>{SOURCE_ROM.name}\g<2>{crc}\g<3>{sha1}\g<4>'
	updated, replacements = pattern.subn(replacement, contents, count=1)
	if replacements != 1:
		raise RuntimeError(f"could not find the WonderTANG Nextor ROM_LOAD in {DEVICE_SOURCE}")
	if updated == contents:
		return False
	DEVICE_SOURCE.write_text(updated)
	return True


def main() -> None:
	if not SOURCE_ROM.is_file():
		raise SystemExit(f"WonderTANG Nextor ROM not found: {SOURCE_ROM}")
	if not ROM_ARCHIVE.is_file():
		raise SystemExit(f"WonderTANG ROM archive not found: {ROM_ARCHIVE}")

	rom_data = SOURCE_ROM.read_bytes()
	if len(rom_data) != EXPECTED_ROM_SIZE:
		raise SystemExit(
			f"WonderTANG Nextor ROM is {len(rom_data):#x} bytes; expected {EXPECTED_ROM_SIZE:#x}"
		)
	crc = f"{zlib.crc32(rom_data) & 0xffffffff:08x}"
	sha1 = hashlib.sha1(rom_data).hexdigest()
	archive_changed = update_archive(rom_data)
	source_changed = update_device_source(crc, sha1)

	print(f"ROM:  {SOURCE_ROM.name}")
	print(f"CRC:  {crc}")
	print(f"SHA1: {sha1}")
	print(f"Archive: {'updated' if archive_changed else 'unchanged'}")
	print(f"Driver:  {'updated' if source_changed else 'unchanged'}")


if __name__ == "__main__":
	main()
