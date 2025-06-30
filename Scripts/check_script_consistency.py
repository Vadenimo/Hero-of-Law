from __future__ import annotations

from dataclasses import dataclass, field
import json
from pathlib import Path
import sys


TESTIMONY_CROSS_EXAM_PAIRS = [
    ('SCENE_IMPA_1', 'SCENE_IMPA_CROSSEXAM'),
    ('SCENE_ZELDA_1', 'SCENE_ZELDA_1_CROSSEXAM'),
    ('SCENE_ZELDA_2', 'SCENE_ZELDA_2_CROSSEXAM'),
    ('SCENE_ZELDA_3', 'SCENE_ZELDA_3_CROSSEXAM'),
    ('SCENE_INGO_1', 'SCENE_INGO_1_CROSSEXAM'),
    ('SCENE_INGO_2', 'SCENE_INGO_2_CROSSEXAM'),
    ('SCENE_INGO_3', 'SCENE_INGO_3_CROSSEXAM'),
    ('SCENE_THIEF_SHOPK_TESTIMONY', 'SCENE_THIEF_SHOPK_CROSSEXAM'),
    ('SCENE_THIEF_OWL_TESTIMONY', 'SCENE_THIEF_OWL_CROSSEXAM'),
]

DIFFERENTLY_NAMED_TESTIMONY_CE_MESSAGE_PAIRS = {
    'SCENE_ZELDA_2': {'ZELDA4': 'ZELDA5'},
    'SCENE_INGO_1': {'INGO5': 'INGO6'},
}

IGNORE_TESTIMONY_TAGS = [
    '<DI>',
    '<DC>',
    '<PAUSE>',
    *(f'<PAUSE:{n}>' for n in range(20)),
    *(f'<SPEED:{n}>' for n in range(200)),
    '<SOUND:CLUE>',
    '<SOUND:EXPLOSION>',
    '<SOUND:HINT>',
    '<SOUND:STAB>',
    '<SOUND:WHAP>',
    '<SQUARE>',
]

@dataclass
class MessagePairDef:
    scene_1_name: str
    message_1_name: str
    scene_2_name: str
    message_2_name: str
    replacements: dict[str, str] = field(default_factory=dict)

MANUAL_MESSAGE_PAIRS = [
    # System
    MessagePairDef('SCENE_THIEF_TRSPAKCHECK', 'WiiVCNotice', 'SCENE_THIEF_TRSPAKCHECK', 'WiiUVCNotice',
        {'Wii': 'Wii U'}),
    MessagePairDef('SCENE_THIEF_TRSPAKCHECK', 'WiiVCNotice', 'SCENE_THIEF_TRSPAKCHECK', 'GCNNotice',
        {'Wii Virtual Console emulator': "<R>Legend of Zelda: Collector's\nEdition<W> emulator for Nintendo GameCube"}),
    MessagePairDef('SCENE_THIEF_TRSPAKCHECK', 'NoThiefContinue', 'SCENE_THIEF_TRSPAKCHECK', 'NoThiefNoSave',
        {'Save in slot $': 'Defaults'}),
    MessagePairDef('SCENE_THIEF_TRSPAKCHECK', 'NoPakContinue', 'SCENE_THIEF_TRSPAKCHECK', 'NoPak',
        {'\n<NEW_BOX>\nContinue anyway?\n(Default name will be used.)\n<TWO_CHOICES><G>Yes\nNo<W>': ''}),

    # Court Record
    MessagePairDef('OBJ_COURT_RECORD_DATA', 'AttorneyBadge', 'OBJ_COURT_RECORD_DATA_TRAILER', 'AttorneyBadge'),
    MessagePairDef('OBJ_COURT_RECORD_DATA', 'AttorneyBadge', 'OBJ_COURT_RECORD_DATA_THIEF', 'AttorneyBadge'),
    MessagePairDef('OBJ_COURT_RECORD_DATA', 'Judge', 'OBJ_COURT_RECORD_DATA_THIEF', 'Judge'),
    MessagePairDef('OBJ_COURT_RECORD_DATA', 'Dragmire', 'OBJ_COURT_RECORD_DATA_THIEF', 'Dragmire'),
    MessagePairDef('OBJ_COURT_RECORD_DATA_THIEF', 'Koholink', 'OBJ_COURT_RECORD_DATA_THIEF', 'Koholink2',
        {'THIEF': '$$$$$'}),
    MessagePairDef('OBJ_COURT_RECORD_DATA_THIEF', 'Owl', 'OBJ_COURT_RECORD_DATA_THIEF', 'Owl2',
        {'THIEF': '$$$$$'}),

    # Bad ending
    MessagePairDef('SCENE_ZELDAS_NEW_EVIDENCE', '1_DRAGMIRE60', 'SCENE_ZELDA_3_CROSSEXAM', 'DRAGMIRE40'),
    MessagePairDef('SCENE_ZELDAS_NEW_EVIDENCE', '1_DRAGMIRE70', 'SCENE_ZELDA_3_CROSSEXAM', 'DRAGMIRE50'),
    MessagePairDef('SCENE_ZELDAS_NEW_EVIDENCE', '1_JUDGE50', 'SCENE_ZELDA_3_CROSSEXAM', 'JUDGE50'),
    MessagePairDef('SCENE_ZELDAS_NEW_EVIDENCE', '1_LINK40', 'SCENE_ZELDA_3_CROSSEXAM', 'LINK30'),
    MessagePairDef('SCENE_ZELDAS_NEW_EVIDENCE', '1_LINK50', 'SCENE_ZELDA_3_CROSSEXAM', 'LINK40'),
    MessagePairDef('SCENE_ZELDAS_NEW_EVIDENCE', '1_JUDGE60', 'SCENE_ZELDA_3_CROSSEXAM', 'JUDGE60'),
    MessagePairDef('SCENE_ZELDAS_NEW_EVIDENCE', '1_JUDGE70', 'SCENE_ZELDA_3_CROSSEXAM', 'JUDGE70'),
    MessagePairDef('SCENE_ZELDAS_NEW_EVIDENCE', '1_DRAGMIRE80', 'SCENE_ZELDA_3_CROSSEXAM', 'DRAGMIRE60'),
    MessagePairDef('SCENE_ZELDAS_NEW_EVIDENCE', '1_DRAGMIRE85', 'SCENE_ZELDA_3_CROSSEXAM', 'DRAGMIRE70'),
    MessagePairDef('SCENE_ZELDAS_NEW_EVIDENCE', '1_DRAGMIRE90', 'SCENE_ZELDA_3_CROSSEXAM', 'DRAGMIRE80'),

    # Flashbacks
    MessagePairDef('SCENE_THIEF_TRIALSTART', 'JUDGE140', 'SCENE_THIEF_VERDICT', 'JUDGE_FLSH'),
    MessagePairDef('SCENE_THIEF_OWL_TESTIMONY', 'OWL3', 'SCENE_THIEF_OWL_CROSSEXAM', 'OWLFL10'),

    # Antipiracy
    MessagePairDef('SCENE_ANTIPIRACY_EDGB', 'JUDGE10', 'SCENE_ANTIPIRACY_EZJR', 'JUDGE10'),
    MessagePairDef('SCENE_ANTIPIRACY_EDGB', 'JUDGE20', 'SCENE_ANTIPIRACY_EZJR', 'JUDGE20'),
    MessagePairDef('SCENE_ANTIPIRACY_EDGB', 'JUDGE30', 'SCENE_ANTIPIRACY_EZJR', 'JUDGE30'),

    # Other
    MessagePairDef('SCENE_THINKING_RANCH', 'MustRethink', 'SCENE_THINKING_THIEF', 'MustRethink'),
]


class Message:
    scene: Scene
    data: dict

    def __init__(self, scene: Scene, data: dict):
        self.scene = scene
        self.data = data

    @property
    def name(self) -> str:
        return self.data['Name']

    @property
    def text_lines(self) -> list[str]:
        return self.data['MessageTextLines']

    def __str__(self) -> str:
        return '\n'.join(self.text_lines)


class Scene:
    data: dict

    def __init__(self, data: dict):
        self.data = data

    @property
    def name(self) -> str:
        return self.data['NPCName']

    _messages = None
    @property
    def messages(self) -> list[Message]:
        if self._messages is None:
            self._messages = [Message(self, m) for m in self.data['Messages']]
        return self._messages

    def get_message(self, name: str) -> Message:
        for message in self.messages:
            if message.name == name:
                return message

        raise KeyError(f'message "{name}" not found')


def print_message_names_line(
    message_1: Message,
    message_2: Message,
) -> None:
    if message_1.scene.name == message_2.scene.name:
        first_half = message_1.scene.name
    else:
        first_half = f'{message_1.scene.name} + {message_2.scene.name}'

    if message_1.name == message_2.name:
        second_half = message_1.name
    else:
        second_half = f'{message_1.name} + {message_2.name}'

    print(f'{first_half} / {second_half}')


def do_messages_match(
    message_1: Message,
    message_2: Message,
    replacements: dict[str, str] | None = None,
    *,
    is_testimony_and_ce: bool = False,
) -> bool:
    text_1 = str(message_1)
    text_2 = str(message_2)

    if replacements:
        for key, value in replacements.items():
            text_1 = text_1.replace(key, value)

    is_bad = False

    if is_testimony_and_ce:
        text_1_clean = text_1.removesuffix('<NS>')
        for to_remove in IGNORE_TESTIMONY_TAGS:
            text_1_clean = text_1_clean.replace(to_remove, '')
        text_1_clean = text_1_clean.replace('<W>', '<G>')

        text_2_clean = text_2

        if not text_2_clean.startswith(('<DI><G>', '<G><DI>')):
            is_bad = True
        text_2_clean = text_2_clean.removeprefix('<DI><G>').removeprefix('<G><DI>')

        if not text_2_clean.endswith('<PERSISTENT>'):
            is_bad = True
        text_2_clean = text_2_clean.removesuffix('<PERSISTENT>')

        text_1_clean = '\n'.join(line.strip() for line in text_1_clean.splitlines())
        text_2_clean = '\n'.join(line.strip() for line in text_2_clean.splitlines())

        if text_1_clean != text_2_clean:
            is_bad = True
    else:
        is_bad = text_1 != text_2

    return not is_bad


def print_message_comparison(
    title_1: str,
    message_1: Message,
    title_2: str,
    message_2: Message,
    *,
    indent: int = 0,
) -> None:
    WIDTH = 50
    for line in [
        f'{title_1}: '.ljust(WIDTH, '-'),
        *message_1.text_lines,
        f'{title_2}: '.ljust(WIDTH, '-'),
        *message_2.text_lines,
        '-' * WIDTH,
    ]:
        print((' ' * indent) + line)


def main() -> None:
    path = Path(sys.argv[1])

    with path.open('r', encoding='utf-8') as f:
        data = json.load(f)

    npcs_by_name = {npc['NPCName']: npc for npc in data['Entries']}

    # Testimony vs. cross-exams
    for testimony_name, ce_name in TESTIMONY_CROSS_EXAM_PAIRS:
        testimony = Scene(npcs_by_name[testimony_name])
        ce = Scene(npcs_by_name[ce_name])

        for testimony_msg in testimony.messages:
            ce_msg_name = DIFFERENTLY_NAMED_TESTIMONY_CE_MESSAGE_PAIRS \
                .get(testimony.name, {}) \
                .get(testimony_msg.name, testimony_msg.name)

            for ce_msg in ce.messages:
                if ce_msg.name == ce_msg_name:
                    is_title = testimony_msg.name == 'Title'

                    print_message_names_line(testimony_msg, ce_msg)

                    if not do_messages_match(
                        testimony_msg,
                        ce_msg,
                        is_testimony_and_ce=(not is_title),
                    ):
                        print_message_comparison('Testimony', testimony_msg, 'Cross-Exam', ce_msg, indent=4)

                    break

            else:
                raise ValueError(f"Couldn't find counterpart of {testimony_msg['Name']} in {ce_name}")

    # Everything else
    for pair in MANUAL_MESSAGE_PAIRS:
        scene_1 = Scene(npcs_by_name[pair.scene_1_name])
        scene_2 = Scene(npcs_by_name[pair.scene_2_name])
        message_1 = scene_1.get_message(pair.message_1_name)
        message_2 = scene_2.get_message(pair.message_2_name)

        print_message_names_line(message_1, message_2)

        if not do_messages_match(message_1, message_2, pair.replacements):
            print_message_comparison('Message 1', message_1, 'Message 2', message_2, indent=4)


if __name__ == '__main__':
    main()
