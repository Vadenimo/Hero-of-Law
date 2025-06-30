import csv
from pathlib import Path

INSTRUMENTS_FOLDER = Path('../Music/instruments')


def main():
    where_found = {}
    for fp in INSTRUMENTS_FOLDER.glob('*.tsv'):
        with fp.open('r', encoding='utf-8', newline='') as f:
            tsv = list(csv.reader(f, delimiter='\t'))

        if len(tsv) == 4:
            # regular instrument
            sample_1 = tsv[3][0]
            sample_2 = tsv[3][2]
            sample_3 = tsv[3][4]
            sample_1 = None if sample_1 == 'NULL' else int(sample_1)
            sample_2 = None if sample_2 == 'NULL' else int(sample_2)
            sample_3 = None if sample_3 == 'NULL' else int(sample_3)
            samples = set([sample_1, sample_2, sample_3])
            samples.discard(None)
        else:
            # drums instrument
            samples = set()
            for row in tsv[3:]:
                samples.add(int(row[0]))

        for s in samples:
            where_found.setdefault(s, set()).add(fp)

    for id in sorted(where_found):
        if len(where_found[id]) == 1:
            path = next(iter(where_found[id]))
            print(f'\nSample {id} is used by {path.name}')
        else:
            print(f'\nSample {id} is used by the following instruments:')
            for path in where_found[id]:
                print(f'  {path.name}')


if __name__ == '__main__':
    main()
