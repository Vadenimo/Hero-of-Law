import os
from pathlib import Path
import shutil
import subprocess


def copy_folders_by_number(source_folder: Path, destination_folder: Path, array_file: Path) -> None:
    try:
        folder_numbers_to_copy = array_file.read_text().split(',')
        folder_numbers_to_copy = [int(x) for x in folder_numbers_to_copy]
    except FileNotFoundError:
        raise FileNotFoundError(f'Array file not found: {array_file}')
    except Exception as e:
        raise Exception(f'Error reading array file: {e}')

    if not source_folder.is_dir():
        raise FileNotFoundError(f'Error: Source folder not found: {source_folder}')

    if not destination_folder.is_dir():
        try:
            print(f'Creating destination folder: {destination_folder}')
            destination_folder.mkdir(parents=True)  # Create the destination folder if it doesn't exist
        except Exception as e:
            raise Exception(f'Error creating destination folder: {e}')

    for item in source_folder.iterdir():
        if not item.is_dir():
            continue

        try:
            number_prefix = int(item.name.split('-')[0])  # Extract the number prefix
            if number_prefix in folder_numbers_to_copy:
                destination_path = destination_folder / item.name
                print(f'Copying folder: {item} to {destination_folder}')
                shutil.copytree(item, destination_path)  # Copy the folder
        except ValueError:
            print(f'Warning: Could not extract number from folder name: {item}. Skipping.')
        except Exception as e:
            raise Exception(f'Error copying folder {item}: {e}')


def main() -> None:
    os.chdir(Path(__file__).parent.resolve())

    if os.name == 'nt':  # Windows
        hol_path = Path('zzrtl-hol.exe')
    else:  # Linux
        hol_path = Path('zzrtl-hol')

    rtl_file = Path('oot_dump.rtl')

    with subprocess.Popen([str(hol_path.resolve()), str(rtl_file)],
                          stdin=subprocess.PIPE,
                          text=True) as process:
        process.communicate(input='\n')  # Send Enter key


    source_folder = Path('audio/audiotable/_vanilla-1.0')
    destination_folder = Path('audio/audiotable/_usedVanilla')
    array_file = Path('audiosfxVanilla.txt')

    copy_folders_by_number(source_folder, destination_folder, array_file)

    source_folder = Path('object/_vanilla-1.0')
    destination_folder = Path('object')
    array_file = Path('objectVanilla.txt')

    copy_folders_by_number(source_folder, destination_folder, array_file)

    source_folder = Path('actor/_vanilla-1.0')
    destination_folder = Path('actor/_custom-1.0')
    array_file = Path('actorVanilla.txt')

    copy_folders_by_number(source_folder, destination_folder, array_file)

    print('Cleaning up...')

    items_to_delete = [
        Path('actorVanilla.txt'),
        Path('audiosfxVanilla.txt'),
        Path('objectVanilla.txt'),
        Path('oot_dump.rtl'),
        Path('oot_names.tsv'),
        Path('prepare.py'),
    ]

    for item in items_to_delete:
        try:
            print(f'Deleting: {item}')
            if item.is_dir():
                shutil.rmtree(item)
            else:
                item.unlink()
        except Exception as e:
            print(f'Warning: Could not delete {item}: {e}')

    shutil.copy2('oot-1.0-dec.z64', 'assets/oot-1.0.z64')

    print('OK, done.')


if __name__ == '__main__':
    main()
