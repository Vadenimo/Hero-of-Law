from __future__ import annotations

import argparse
from pathlib import Path


def extract_id_from_folder_name(name: str) -> id | None:
    # this is just mimicking zzrtl's logic
    int_len = 0
    while int_len < len(name) and name[int_len] in '0123456789':
        int_len += 1

    if int_len == 0:
        return None
    else:
        return int(name[:int_len])


def collect_instruments_from_folder(folder: Path, write_to: dict[int, Path]) -> None:
    for fp in folder.iterdir():
        if not fp.is_dir():
            continue
        id = extract_id_from_folder_name(fp.name)
        if id is not None:
            write_to[id] = fp


def main(argv=None):
    parser = argparse.ArgumentParser()

    parser.add_argument('--vanilla', type=Path, default=Path('../audio/audiotable/_usedVanilla'),
    #parser.add_argument('--vanilla', type=Path, default=Path('audio/audiotable/_vanilla-1.0'),
        help='vanilla samples directory')
    parser.add_argument('--custom', type=Path, default=Path('../audio/audiotable'),
        help='custom samples directory')
    parser.add_argument('--out-bin', type=Path, default=Path('../audio/audiotable/_vanilla-1.0/audiotable.bin'),
        help='output bin file')
    parser.add_argument('--out-tsv', type=Path, default=Path('../audio/audiotable/_vanilla-1.0/audiotable.tsv'),
        help='output tsv file')

    args = parser.parse_args(argv)

    id_to_folder = {}
    collect_instruments_from_folder(args.vanilla, id_to_folder)
    collect_instruments_from_folder(args.custom, id_to_folder)

    tsv_lines = ['start\tlength\tname']
    bin_data = bytearray()

    for i in range(max(id_to_folder) + 1):
        folder = id_to_folder.get(i)
        if folder is None:
            tsv_lines.append('NULL\tNULL\tNULL')
        else:
            raw_sound = (folder / 'RawSound.bin').read_bytes()
            tsv_lines.append(f'{len(bin_data):X}\t{len(raw_sound):X}\t{folder.name}')
            bin_data.extend(raw_sound)

            # padding after instead of before to match the original script's behavior
            while len(bin_data) % 0x10:
                bin_data.append(0)

    # this is also to match the original script's behavior
    tsv_lines.append('')

    args.out_tsv.write_text('\n'.join(tsv_lines), newline='\n')
    args.out_bin.write_bytes(bin_data)
    print('Audio built!')


if __name__ == '__main__':
    main()
