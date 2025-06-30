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

def remove_tags(text: str) -> str:
    new = []
    active = True
    for c in text:
        if c == '<':
            active = False
        elif c == '>':
            active = True
        elif active:
            new.append(c)
    return ''.join(new)

for i, npc in enumerate(data['Entries']):
    num_messages = len(npc['Messages'])
    for j, message in enumerate(npc['Messages']):
        text = message['MessageTextLines']
        text_line = ' '.join(text)
        text_line = remove_tags(text_line)
        if text_line in {'...', '(...)'}:
            print(f'{npc["NPCName"]} / {message["Name"]}: {" ".join(text)}')
            regex = re.compile(r'\b' + re.escape(message['Name']) + r'\b')
            for script in npc['Scripts']:
                for line in script['TextLines']:
                    if regex.search(line):
                        if '::textboxSpeaker' in line:
                            warning = ''
                        else:
                            warning = ' <----------- !!!!!!!!!!!!!!!!!'
                        print(f'    {line.strip()}{warning}')
