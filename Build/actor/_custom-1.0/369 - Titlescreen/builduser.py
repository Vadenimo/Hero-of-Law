import datetime
import getpass
from pathlib import Path

# Uncomment for final game build
'''

MAIN = Path('actor.zovl')
BACKUP = Path('actor.zovl.bk')

PLACEHOLDER = b'BUILDUSERBUILDUSERBUILDUSERBUILDUSE'


if BACKUP.is_file():
   data = BACKUP.read_bytes()
else:
   data = MAIN.read_bytes()
   BACKUP.write_bytes(data)

date = datetime.datetime.now().replace(microsecond=0)
new_string = f'{date.isoformat()} {getpass.getuser()}'
new_string = new_string.encode('ascii').ljust(len(PLACEHOLDER), b'\0')

new_data = data.replace(PLACEHOLDER, new_string)

MAIN.write_bytes(new_data)

'''