# 2025-06-23
# Hero of Law xdelta Patch Creator
# (based on the Newer DS one)

import concurrent.futures
import hashlib
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile


SEVENZ_COMMAND = '7z'
XDELTA_EXE = Path('tools/xdelta3-3.1.0-i686.exe')
IN_ROMS_DIR = Path('in_roms')
IN_PDFS_DIR = Path('in_pdfs')
INPUT_ROM_Z64 = Path('../Build/build-yaz.z64')
INPUT_ROM_WAD = Path('../Build/build-yaz.wad')
OUT_PATCHES_WEB_DIR = Path('HeroOfLaw_web_patches')
ARCHIVE_INTERNAL_NAME = Path('Hero of Law')
OUT_7Z = Path('HeroOfLaw.7z')

NUM_THREADS = os.cpu_count() - 1


def efprint(*args, **kwargs) -> None:
    """print(..., end='', flush=True)"""
    print(*args, end='', flush=True, **kwargs)


def run_windows_exe(cmd, *args, **kwargs) -> subprocess.CompletedProcess:
    """Run a Windows exe, using wine only if necessary"""
    if sys.platform != 'win32':
        cmd = ['wine'] + cmd
    return subprocess.run(cmd)


def make_xdelta(base: Path, patched: Path, out: Path, *, secondary_compression: str) -> None:
    """Create an xdelta patch"""
    run_windows_exe([
        str(XDELTA_EXE),
        '-e',  # "encode" (make patch)
        '-9',  # compression level
        '-A',  # don't include appdata header (string showing the input file paths, what the heck)
        '-S', secondary_compression,  # options: "none", "lzma" (default), "djw", "fgk"
        '-s', str(base),  # "source file"
        str(patched),
        str(out),
    ])


def apply_xdelta(base: Path, patch: Path, out: Path) -> None:
    """Apply an xdelta patch"""
    run_windows_exe([
        str(XDELTA_EXE),
        '-d',  # "decode" (apply patch)
        '-s', str(base),  # "source file"
        str(patch),
        str(out),
    ])


def check_7z_available() -> bool:
    """Check whether 7-Zip is available"""
    try:
        subprocess.run([SEVENZ_COMMAND], stdout=subprocess.DEVNULL)
        return True
    except FileNotFoundError:
        return False


def make_7z(paths: list[Path], out: Path) -> None:
    """Create a 7-zip archive"""
    subprocess.run([
        SEVENZ_COMMAND,
        'a',  # "add files to archive"
        '-mx9',  # compression level 9
        str(out),
        *(str(p) for p in paths),
    ])


def main() -> None:
    print(f'(running with {NUM_THREADS} threads)')

    efprint('checking environment...')
    if not XDELTA_EXE.is_file():
        raise FileNotFoundError(f'{XDELTA_EXE} does not exist')
    if not INPUT_ROM_Z64.is_file():
        raise FileNotFoundError(f'{INPUT_ROM_Z64} does not exist')
    if not INPUT_ROM_WAD.is_file():
        raise FileNotFoundError(f'{INPUT_ROM_WAD} does not exist')
    if not IN_ROMS_DIR.is_dir():
        raise FileNotFoundError(f'{IN_ROMS_DIR} does not exist')
    if not IN_PDFS_DIR.is_dir():
        raise FileNotFoundError(f'{IN_PDFS_DIR} does not exist')
    if not check_7z_available():
        raise RuntimeError('7z is not available')
    print(' done')

    efprint('clearing existing output directories...')
    shutil.rmtree(OUT_PATCHES_WEB_DIR, ignore_errors=True)
    OUT_PATCHES_WEB_DIR.mkdir()
    print(' done')

    efprint('hashing target roms (for testing the patches)...')
    rom_hashes = {}
    for patched in [INPUT_ROM_Z64, INPUT_ROM_WAD]:
        rom_hashes[patched.name] = hashlib.sha1(patched.read_bytes()).digest()
    print(' done')

    def run_one(base: Path) -> None:
        if NUM_THREADS == 1:
            efprint(f'{base!s:<29} |')
        else:
            print(f'generating patches for {base}')

        if base.suffix in {'.z64', '.n64'}:
            patched = INPUT_ROM_Z64
        elif base.suffix == '.wad':
            patched = INPUT_ROM_WAD
        else:
            raise ValueError(f'Unknown file extension on {base}')

        with tempfile.TemporaryDirectory() as temp_dir:
            temp_file = Path(temp_dir) / 'temp.bin'

            if NUM_THREADS == 1:
                efprint(f' generating patch...')

            # Note: patches with secondary LZMA compression are smaller,
            # but they end up resulting in larger downloads overall when
            # packed into .7z archives. And the web patcher we're
            # currently using doesn't support secondary compression at
            # all. So we have to disable it here.

            patch = OUT_PATCHES_WEB_DIR / base.with_suffix('.xdelta').name
            make_xdelta(base, patched, patch, secondary_compression='none')

            if NUM_THREADS == 1:
                efprint(' checking...')

            apply_xdelta(base, patch, temp_file)
            reconstructed_hash = hashlib.sha1(temp_file.read_bytes()).digest()
            assert rom_hashes[patched.name] == reconstructed_hash

        if NUM_THREADS == 1:
            print()

    if NUM_THREADS == 1:
        for base in sorted(IN_ROMS_DIR.iterdir()):
            run_one(base)
    else:
        with concurrent.futures.ThreadPoolExecutor(max_workers=NUM_THREADS) as executor:
            for _ in executor.map(run_one, IN_ROMS_DIR.iterdir()):
                pass

    efprint(f'preparing "{ARCHIVE_INTERNAL_NAME}" directory for 7z...')
    shutil.rmtree(ARCHIVE_INTERNAL_NAME, ignore_errors=True)
    shutil.copytree(OUT_PATCHES_WEB_DIR, ARCHIVE_INTERNAL_NAME)
    for child in IN_PDFS_DIR.iterdir():
        shutil.copy2(child, ARCHIVE_INTERNAL_NAME / child.name)
    print(' done')

    print(f'packing {OUT_7Z}...')
    OUT_7Z.unlink(missing_ok=True)
    make_7z([ARCHIVE_INTERNAL_NAME], OUT_7Z)
    shutil.rmtree(ARCHIVE_INTERNAL_NAME, ignore_errors=True)

    print('done')


if __name__ == '__main__':
    main()
