import csv
import json
from pathlib import Path
import subprocess

SFX_CSV_PATH = Path('NPCMAKER/Dicts/SFX.csv')
ROOT_DIR = Path('..')
SEARCH_DIRS = [
    ROOT_DIR / 'Build',
    ROOT_DIR / 'Code',
    ROOT_DIR / 'Scripts',
]
BLACKLIST = [
    ROOT_DIR / 'Scripts/Dicts/SFX.csv',
    ROOT_DIR / 'Scripts/Old',
]


def main():
    with SFX_CSV_PATH.open('r', encoding='utf-8', newline='') as f:
        table = list(csv.reader(f))

    for row in table:
        row[0] = int(row[0])

    pattern = '|'.join(f'\\b{row[1]}\\b' for row in table)

    cmd = [
        'rg',
        '--json',
        pattern,
        *(str(p) for p in SEARCH_DIRS),
    ]

    where_found = {}

    proc = subprocess.Popen(cmd, encoding='utf-8', stdout=subprocess.PIPE)
    for line in proc.stdout:
        line = json.loads(line)
        if line['type'] != 'match':
            continue
        match = line['data']
        file = Path(match['path']['text'])
        matched_text = match['submatches'][0]['match']['text']

        skip = False
        for b in BLACKLIST:
            if file.is_relative_to(b):
                skip = True
                break
        if skip:
            continue

        where_found.setdefault(matched_text, []).append(match)

    proc.wait()

    for row in table:
        if row[1] in where_found:
            print(f'\nSFX {row[0]} ({row[1]}) found here:')
            for match in where_found[row[1]]:
                file = Path(match['path']['text']).relative_to(ROOT_DIR)
                line_no = match['line_number']
                line_contents = match['lines']['text']

                start = f'{file}:{line_no}'
                print(f'{start:<49} {line_contents.strip()}')


if __name__ == '__main__':
    main()
