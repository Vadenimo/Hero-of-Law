import json
from pathlib import Path
import re
import sys

path = Path(sys.argv[1])

with path.open('r', encoding='utf-8') as f:
    start = f.read(1000)
newline = '\n' if '\r\n' not in start else '\r\n'

with path.open('r', encoding='utf-8') as f:
    data = json.load(f)

def find_or_max(npc_name, needle, haystack):
    match = re.search(r'\b' + re.escape(needle) + r'\b', haystack)
    if match is None:
        print(f'Unused textbox? {npc_name}/{needle}')
        return 99999999999999999999999
    else:
        return match.start(0)

for npc in data['Entries']:
    if any(x in npc['NPCName'] for x in ['COURT_RECORD_DATA', 'OBJ_CASINOCONTROLLER', 'OBJ_TALON_TALONGAME', '_NOSORT']):
        continue

    scripts = []
    for script in npc['Scripts']:
        scripts.append('\n'.join(line.split('//')[0] for line in script['TextLines']))
    script = '\n'.join(scripts)

    npc['Messages'].sort(key=lambda message: find_or_max(npc['NPCName'], message['Name'], script))

if len(sys.argv) > 2:
    out_path = Path(sys.argv[2])
else:
    out_path = path
out_path.write_text(json.dumps(data, indent=2, ensure_ascii=False).replace('\n', newline), encoding='utf-8')
