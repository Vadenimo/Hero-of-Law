import collections
import csv
from decimal import Decimal
import enum
from fractions import Fraction
from io import BytesIO, StringIO
import itertools
import math
from pathlib import Path
import random
from typing import List, TypeAlias

import braceexpand  # pip install braceexpand
import mido  # pip install mido

from lib_midi_controller_message_types import ControllerMsg


TimeSignature: TypeAlias = Fraction


OOT_INSTRUMENT_NAMES = {
    0: 'Accordion',
    1: 'Acoustic Guitar',
    2: 'Banjo',
    3: 'Bassoon',
    4: 'Beat Kit',
    5: 'Bell',
    6: 'Bell Pad',
    7: 'Bent Drum',
    8: 'Chant',
    9: 'Chant ALT',
    10: 'Clap',
    11: 'Clarinet',
    12: 'Clocktown Bell',
    13: 'Conga',
    14: 'Cowbell',
    15: 'Creaking',
    16: 'Cuica',
    17: 'Drum',
    18: 'Ethnic Drum',
    19: 'Ethnic Flute',
    20: 'Ethnic Flute ALT',
    21: 'Famous DC Sound',
    22: 'Famous Wood Chime',
    23: 'Female Choir',
    24: 'Fiddle',
    25: 'Flute',
    26: 'Glockenspiel',
    27: 'Gong / Wind Chimes',
    28: 'Guitar',
    29: 'Harmonica',
    30: 'Harp',
    31: 'Harpsichord',
    32: 'Horn',
    33: 'Jazz Guitar',
    34: 'Jazz Guitar ALT',
    35: 'Kalimba',
    36: 'Koto',
    37: 'Lute Kit',
    38: 'Male Choir',
    39: 'Malon',
    40: 'Marimba',
    41: 'Multipitch Flute',
    42: 'Noise',
    43: 'Oboe',
    44: 'Ocarina',
    45: 'Orchestra Kit',
    46: 'Organ',
    47: 'Pad',
    48: 'Piano',
    49: 'Pizz Strings',
    50: 'Steel Drum',
    51: 'Strings',
    52: 'Synth Strings',
    53: 'Tambourine Kit',
    54: 'Trombone',
    55: 'Trumpet',
    56: 'Tuba',
    57: 'Voice Pad',
    58: 'Wind',
    59: 'Wind Cricket',
    60: 'Wind Roar',
    61: 'Wood Chime / Shine',
}
INSTRUMENT_LIST_BLACKLIST = {45, 127}


class SeqId(enum.IntEnum):
    """
    https://github.com/zeldaret/oot/blob/87a6e752429a7821ae81dcf8bffc6cdf13803084/include/sequence.h
    """
    NA_BGM_GENERAL_SFX = 0         # General Sound Effects
    NA_BGM_NATURE_AMBIENCE = 1     # Environmental nature background sounds
    NA_BGM_FIELD_LOGIC = 2         # Hyrule Field
    NA_BGM_FIELD_INIT = 3          # Hyrule Field Initial Segment From Loading Area
    NA_BGM_FIELD_DEFAULT_1 = 4     # Hyrule Field Moving Segment 1
    NA_BGM_FIELD_DEFAULT_2 = 5     # Hyrule Field Moving Segment 2
    NA_BGM_FIELD_DEFAULT_3 = 6     # Hyrule Field Moving Segment 3
    NA_BGM_FIELD_DEFAULT_4 = 7     # Hyrule Field Moving Segment 4
    NA_BGM_FIELD_DEFAULT_5 = 8     # Hyrule Field Moving Segment 5
    NA_BGM_FIELD_DEFAULT_6 = 9     # Hyrule Field Moving Segment 6
    NA_BGM_FIELD_DEFAULT_7 = 10    # Hyrule Field Moving Segment 7
    NA_BGM_FIELD_DEFAULT_8 = 11    # Hyrule Field Moving Segment 8
    NA_BGM_FIELD_DEFAULT_9 = 12    # Hyrule Field Moving Segment 9
    NA_BGM_FIELD_DEFAULT_A = 13    # Hyrule Field Moving Segment 10
    NA_BGM_FIELD_DEFAULT_B = 14    # Hyrule Field Moving Segment 11
    NA_BGM_FIELD_ENEMY_INIT = 15   # Hyrule Field Enemy Approaches
    NA_BGM_FIELD_ENEMY_1 = 16      # Hyrule Field Enemy Near Segment 1
    NA_BGM_FIELD_ENEMY_2 = 17      # Hyrule Field Enemy Near Segment 2
    NA_BGM_FIELD_ENEMY_3 = 18      # Hyrule Field Enemy Near Segment 3
    NA_BGM_FIELD_ENEMY_4 = 19      # Hyrule Field Enemy Near Segment 4
    NA_BGM_FIELD_STILL_1 = 20      # Hyrule Field Standing Still Segment 1
    NA_BGM_FIELD_STILL_2 = 21      # Hyrule Field Standing Still Segment 2
    NA_BGM_FIELD_STILL_3 = 22      # Hyrule Field Standing Still Segment 3
    NA_BGM_FIELD_STILL_4 = 23      # Hyrule Field Standing Still Segment 4
    NA_BGM_DUNGEON = 24            # Dodongo's Cavern
    NA_BGM_KAKARIKO_ADULT = 25     # Kakariko Village (Adult)
    NA_BGM_ENEMY = 26              # Battle
    NA_BGM_BOSS = 27               # Boss Battle "NA_BGM_BOSS00"
    NA_BGM_INSIDE_DEKU_TREE = 28   # Inside the Deku Tree "NA_BGM_FAIRY_DUNGEON"
    NA_BGM_MARKET = 29             # Market
    NA_BGM_TITLE = 30              # Title Theme
    NA_BGM_LINK_HOUSE = 31         # House
    NA_BGM_GAME_OVER = 32          # Game Over
    NA_BGM_BOSS_CLEAR = 33         # Boss Clear
    NA_BGM_ITEM_GET = 34           # Obtain Item
    NA_BGM_OPENING_GANON = 35      # Enter Ganondorf
    NA_BGM_HEART_GET = 36          # Obtain Heart Container
    NA_BGM_OCA_LIGHT = 37          # Prelude of Light
    NA_BGM_JABU_JABU = 38          # Inside Jabu-Jabu's Belly "NA_BGM_BUYO_DUNGEON"
    NA_BGM_KAKARIKO_KID = 39       # Kakariko Village (Child)
    NA_BGM_GREAT_FAIRY = 40        # Great Fairy's Fountain "NA_BGM_GODESS"
    NA_BGM_ZELDA_THEME = 41        # Zelda's Theme "NA_BGM_HIME"
    NA_BGM_FIRE_TEMPLE = 42        # Fire Temple "NA_BGM_FIRE_DUNGEON"
    NA_BGM_OPEN_TRE_BOX = 43       # Open Treasure Chest
    NA_BGM_FOREST_TEMPLE = 44      # Forest Temple "NA_BGM_FORST_DUNGEON"
    NA_BGM_COURTYARD = 45          # Hyrule Castle Courtyard "NA_BGM_HIRAL_GARDEN"
    NA_BGM_GANON_TOWER = 46        # Ganondorf's Theme
    NA_BGM_LONLON = 47             # Lon Lon Ranch "NA_BGM_RONRON"
    NA_BGM_GORON_CITY = 48         # Goron City "NA_BGM_GORON"
    NA_BGM_FIELD_MORNING = 49      # Hyrule Field Morning Theme
    NA_BGM_SPIRITUAL_STONE = 50    # Spiritual Stone Get "NA_BGM_SPIRIT_STONE"
    NA_BGM_OCA_BOLERO = 51         # Bolero of Fire "NA_BGM_OCA_FLAME"
    NA_BGM_OCA_MINUET = 52         # Minuet of Forest "NA_BGM_OCA_WIND"
    NA_BGM_OCA_SERENADE = 53       # Serenade of Water "NA_BGM_OCA_WATER"
    NA_BGM_OCA_REQUIEM = 54        # Requiem of Spirit "NA_BGM_OCA_SOUL"
    NA_BGM_OCA_NOCTURNE = 55       # Nocturne of Shadow "NA_BGM_OCA_DARKNESS"
    NA_BGM_MINI_BOSS = 56          # Mini-Boss Battle "NA_BGM_MIDDLE_BOSS"
    NA_BGM_SMALL_ITEM_GET = 57     # Obtain Small Item "NA_BGM_S_ITEM_GET"
    NA_BGM_TEMPLE_OF_TIME = 58     # Temple of Time "NA_BGM_SHRINE_OF_TIME"
    NA_BGM_EVENT_CLEAR = 59        # Escape from Lon Lon Ranch
    NA_BGM_KOKIRI = 60             # Kokiri Forest
    NA_BGM_OCA_FAIRY_GET = 61      # Obtain Fairy Ocarina "NA_BGM_OCA_YOUSEI"
    NA_BGM_SARIA_THEME = 62        # Lost Woods "NA_BGM_MAYOIMORI"
    NA_BGM_SPIRIT_TEMPLE = 63      # Spirit Temple "NA_BGM_SOUL_DUNGEON"
    NA_BGM_HORSE = 64              # Horse Race
    NA_BGM_HORSE_GOAL = 65         # Horse Race Goal
    NA_BGM_INGO = 66               # Ingo's Theme
    NA_BGM_MEDALLION_GET = 67      # Obtain Medallion "NA_BGM_MEDAL_GET"
    NA_BGM_OCA_SARIA = 68          # Ocarina Saria's Song
    NA_BGM_OCA_EPONA = 69          # Ocarina Epona's Song
    NA_BGM_OCA_ZELDA = 70          # Ocarina Zelda's Lullaby
    NA_BGM_OCA_SUNS = 71           # Ocarina Sun's Song "NA_BGM_OCA_SUNMOON"
    NA_BGM_OCA_TIME = 72           # Ocarina Song of Time
    NA_BGM_OCA_STORM = 73          # Ocarina Song of Storms
    NA_BGM_NAVI_OPENING = 74       # Fairy Flying "NA_BGM_NAVI"
    NA_BGM_DEKU_TREE_CS = 75       # Deku Tree "NA_BGM_DEKUNOKI"
    NA_BGM_WINDMILL = 76           # Windmill Hut "NA_BGM_FUSHA"
    NA_BGM_HYRULE_CS = 77          # Legend of Hyrule "NA_BGM_HIRAL_DEMO"
    NA_BGM_MINI_GAME = 78          # Shooting Gallery
    NA_BGM_SHEIK = 79              # Sheik's Theme "NA_BGM_SEAK"
    NA_BGM_ZORA_DOMAIN = 80        # Zora's Domain "NA_BGM_ZORA"
    NA_BGM_APPEAR = 81             # Enter Zelda
    NA_BGM_ADULT_LINK = 82         # Goodbye to Zelda
    NA_BGM_MASTER_SWORD = 83       # Master Sword
    NA_BGM_INTRO_GANON = 84
    NA_BGM_SHOP = 85               # Shop
    NA_BGM_CHAMBER_OF_SAGES = 86   # Chamber of the Sages "NA_BGM_KENJA"
    NA_BGM_FILE_SELECT = 87        # File Select
    NA_BGM_ICE_CAVERN = 88         # Ice Cavern "NA_BGM_ICE_DUNGEON"
    NA_BGM_DOOR_OF_TIME = 89       # Open Door of Temple of Time "NA_BGM_GATE_OPEN"
    NA_BGM_OWL = 90                # Kaepora Gaebora's Theme
    NA_BGM_SHADOW_TEMPLE = 91      # Shadow Temple "NA_BGM_DARKNESS_DUNGEON"
    NA_BGM_WATER_TEMPLE = 92       # Water Temple "NA_BGM_AQUA_DUNGEON"
    NA_BGM_BRIDGE_TO_GANONS = 93   # Ganon's Castle Bridge "NA_BGM_BRIDGE"
    NA_BGM_OCARINA_OF_TIME = 94    # Ocarina of Time "NA_BGM_SARIA"
    NA_BGM_GERUDO_VALLEY = 95      # Gerudo Valley "NA_BGM_GERUDO"
    NA_BGM_POTION_SHOP = 96        # Potion Shop "NA_BGM_DRUGSTORE"
    NA_BGM_KOTAKE_KOUME = 97       # Kotake & Koume's Theme
    NA_BGM_ESCAPE = 98             # Escape from Ganon's Castle
    NA_BGM_UNDERGROUND = 99        # Ganon's Castle Under Ground
    NA_BGM_GANONDORF_BOSS = 100    # Ganondorf Battle
    NA_BGM_GANON_BOSS = 101        # Ganon Battle
    NA_BGM_END_DEMO = 102          # Seal of Six Sages
    NA_BGM_STAFF_1 = 103           # End Credits I
    NA_BGM_STAFF_2 = 104           # End Credits II
    NA_BGM_STAFF_3 = 105           # End Credits III
    NA_BGM_STAFF_4 = 106           # End Credits IV
    NA_BGM_FIRE_BOSS = 107         # King Dodongo & Volvagia Boss Battle "NA_BGM_BOSS01"
    NA_BGM_TIMED_MINI_GAME = 108   # Mini-Game
    NA_BGM_CUTSCENE_EFFECTS = 109  # A small collection of various cutscene sounds
    NA_BGM_NO_MUSIC = 0x7F         # No bgm music is played
    NA_BGM_NATURE_SFX_RAIN = 0x80  # Related to rain
    NA_BGM_DISABLED = 0xFFFF


def clamp(value, min, max):
    if value < min:
        return min
    elif value > max:
        return max
    else:
        return value


def musescore_note_name(note_value: int) -> str:
    return ['C','C#','D','D#','E','F','F#','G','G#','A','A#','B'][note_value % 12] \
        + str(note_value // 12 - 1)
assert musescore_note_name(0) == 'C-1'
assert musescore_note_name(59) == 'B3'
assert musescore_note_name(60) == 'C4'
assert musescore_note_name(127) == 'G9'


def track_list(midi_or_track: mido.MidiFile | mido.MidiTrack) -> list[mido.MidiTrack]:
    if isinstance(midi_or_track, mido.MidiFile):
        return midi_or_track.tracks
    else:
        return [midi_or_track]


def renumber_track_channels_sequentially(midi: mido.MidiFile) -> None:
    """
    Re-number all channels sequentially.
    This fixes some playback issues in-game.
    """
    for i, track in enumerate(midi.tracks):
        for msg in track:
            if hasattr(msg, 'channel'):
                msg.channel = i % 16


def assign_track_names(
    track_names: list[str],
    midi: mido.MidiFile,
) -> None:
    """
    Assign names to the tracks in the midi
    """
    # Check that track names look sane
    if len(midi.tracks) != len(track_names):
        print("MIDI tracks don't match expected track names! Here's how they currently align:")

        for track, name in itertools.zip_longest(midi.tracks, track_names):
            if track is None:
                track_name = None
            else:
                track_name = track.name.rstrip('\0')
            print(f'"{track_name}" -> "{name}"')

        raise ValueError(f'MIDI has {len(midi.tracks)} tracks, but we have {len(track_names)} track names')

    for track, name in zip(midi.tracks, track_names):
        track.name = name


def fix_drum_patch(midi_or_track: mido.MidiFile | mido.MidiTrack, drum_instruments: set[int]) -> set[int]:
    """
    Change all drums to instrument 127
    """
    # Note that iterating over the midi directly will cause changes to
    # events to be silently discarded for some reason, so we have to
    # iterate over the tracks
    found_drum_instruments = set()
    for track in track_list(midi_or_track):
        for msg in track:
            if msg.type == 'program_change' and msg.program in drum_instruments:
                found_drum_instruments.add(msg.program)
                msg.program = 127
    return found_drum_instruments


def add_loop_markers(midi: mido.MidiFile, loop_start_tick: int) -> None:
    """
    Add markers to the first track to indicate the loop start
    """
    midi.tracks[0] = mido.merge_tracks([
        midi.tracks[0],
        [
            mido.MetaMessage(type='marker', text='Section 0', time=0),
            mido.MetaMessage(type='marker', text='Section 1', time=loop_start_tick),
        ],
    ])

    # We also need to ensure that all the instruments and pan values get
    # reset appropriately at the beginning of the loop:
    for i, track in enumerate(midi.tracks):
        reset_msgs = get_state_msgs_at(track, loop_start_tick)
        reset_msgs[0].time = loop_start_tick
        midi.tracks[i] = mido.merge_tracks([
            midi.tracks[i],
            reset_msgs,
        ])


def normalize_velocities(midi_or_track: mido.MidiFile | mido.MidiTrack, **kwargs) -> None:
    """
    Scale all velocities so that the loudest is 127 (or some other
    maximum value)
    """
    def getter(msg):
        if msg.type == 'note_on':
            return msg.velocity

    def setter(msg, scaling_factor):
        if msg.type == 'note_on':
            msg.velocity = clamp(round(msg.velocity * scaling_factor), 0, 127)

    normalize_values(midi_or_track, getter, setter, **kwargs)


def normalize_cc7s(midi_or_track: mido.MidiFile | mido.MidiTrack, **kwargs) -> None:
    """
    Scale all CC 7 event values so that the loudest is 127 (or some
    other maximum value)
    """
    def getter(msg):
        if msg.type == 'control_change' and msg.control == ControllerMsg.CHANNEL_VOLUME_7.value:
            return msg.value

    def setter(msg, scaling_factor):
        if msg.type == 'control_change' and msg.control == ControllerMsg.CHANNEL_VOLUME_7.value:
            msg.value = clamp(round(msg.value * scaling_factor), 0, 127)

    normalize_values(midi_or_track, getter, setter, **kwargs)


def normalize_cc11s(midi_or_track: mido.MidiFile | mido.MidiTrack, **kwargs) -> None:
    """
    Scale all CC 11 event values so that the loudest is 127 (or some
    other maximum value)
    """
    def getter(msg):
        if msg.type == 'control_change' and msg.control == ControllerMsg.EXPRESSION_CONTROLLER_11.value:
            return msg.value

    def setter(msg, scaling_factor):
        if msg.type == 'control_change' and msg.control == ControllerMsg.EXPRESSION_CONTROLLER_11.value:
            msg.value = clamp(round(msg.value * scaling_factor), 0, 127)

    normalize_values(midi_or_track, getter, setter, **kwargs)


def normalize_values(midi_or_track: mido.MidiFile | mido.MidiTrack, getter, setter, *, scale_to: int = 127, verbose: bool = False) -> None:
    """
    Scale all velocities so that the loudest is 127 (or some other
    maximum value)
    """
    max_vel = 0
    found_any = False
    for track in track_list(midi_or_track):
        for msg in track:
            value = getter(msg)
            if value is not None:
                max_vel = max(max_vel, value)
                found_any = True

    if not found_any or max_vel == scale_to:
        return

    if verbose:
        print(f'Scaling maximum value from {max_vel} to {scale_to}')

    scaling_factor = scale_to / max_vel

    for track in track_list(midi_or_track):
        for msg in track:
            setter(msg, scaling_factor)


def create_instrument_list(midi_or_track: mido.MidiFile | mido.MidiTrack) -> str:
    """
    Make a list of instruments used in a MIDI, as a string
    """
    inst_ids = set()
    for msg in midi_or_track:
        if msg.type == 'program_change' and msg.program not in INSTRUMENT_LIST_BLACKLIST:
            inst_ids.add(msg.program)

    lines = []
    for id in sorted(inst_ids):
        lines.append(f'{id} = {OOT_INSTRUMENT_NAMES[id]}')

    return '\n'.join(lines)


def export_instrument_list(midi_or_track: mido.MidiFile | mido.MidiTrack, path: Path, *, and_print: bool = False) -> None:
    """
    Make a list of instruments used in a MIDI, and save it to a Path
    """
    s = create_instrument_list(midi_or_track)

    if and_print:
        print(s)

    path.write_text(s, encoding='utf-8')


def check_note_sanity(
    midi_or_track: mido.MidiFile | mido.MidiTrack,
    *,
    time_signatures: dict[int, TimeSignature] | None,
):
    """
    Check that all notes that are started, are eventually finished
    """
    if hasattr(midi_or_track, 'ticks_per_beat'):
        ticks_per_beat = midi_or_track.ticks_per_beat
    else:
        # reasonable default
        ticks_per_beat = 480

    if time_signatures is None:
        # reasonable default
        time_signatures = {1: TimeSignature(4, 4)}

    def local_ticks_to_measure_str(tick: int) -> str:
        measure = ticks_to_measure(ticks_per_beat, tick, time_signatures=time_signatures)
        if measure.is_integer():
            return str(measure.as_integer_ratio()[0])
        else:
            whole_part = math.floor(measure)
            frac_part = measure - whole_part
            return f'{whole_part} {frac_part} ({float(measure)})'

    for i, track in enumerate(track_list(midi_or_track)):
        active_notes = {}
        time = 0
        for msg in track:
            time += msg.time

            # making this a lambda so we don't run ticks_to_measure()
            # a billion times for no reason (it's not a cheap operation)
            pre = lambda: f'Track {i+1}, measure {local_ticks_to_measure_str(time)}'

            if msg.type == 'note_on' and msg.velocity > 0:
                if msg.note in active_notes:
                    raise ValueError(f'{pre()}: playing already-active note {msg.note} ({musescore_note_name(msg.note)}) (started at {local_ticks_to_measure_str(active_notes[msg.note])})')
                else:
                    active_notes[msg.note] = time

                    # oot-specific limitation
                    if len(active_notes) > 4:
                        raise ValueError(f'{pre()}: playing a fifth simultaneous note')

            elif msg.type == 'note_on' or msg.type == 'note_off':
                if msg.note not in active_notes:
                    raise ValueError(f'{pre()}: ending nonexistent note {msg.note} ({musescore_note_name(msg.note)})')
                else:
                    del active_notes[msg.note]

        if active_notes:
            new_list = []
            for val in sorted(active_notes):
                new_list.append(f'{val} ({musescore_note_name(val)}) (started at {local_ticks_to_measure_str(active_notes[val])})')
            raise ValueError(f'Track {i+1}: some notes were never ended: {", ".join(new_list)}')


def clean_up_track(track: mido.MidiTrack, *, force_channel: int | None = None) -> mido.MidiTrack:
    """
    Clean up a track by removing redundant and useless meta messages
    """
    new_track = mido.MidiTrack()

    if force_channel is not None:
        for msg in track:
            if hasattr(msg, 'channel'):
                msg.channel = force_channel

    metas = {}
    controls = {}
    program = None
    pitchwheel = 0
    have_track_name = False

    prev_msg_time = 0
    def add(msg):
        nonlocal new_track, time, prev_msg_time
        new_track.append(msg.copy(time=(time - prev_msg_time)))
        prev_msg_time = time

    time = 0
    for msg in track:
        time += msg.time

        if msg.is_meta:
            if msg.type == 'track_name':
                # only keep the first track name, because shrug
                if not have_track_name:
                    add(msg)
                    have_track_name = True
            elif msg.type == 'midi_port':
                # these are dumb and useless afaik -- remove
                pass
            else:
                if msg.type not in metas or msg.copy(time=0) != metas[msg.type].copy(time=0):
                    add(msg)
                    metas[msg.type] = msg.copy()
        elif msg.type == 'control_change':
            # ignoring channel number because they're all going to
            # get normalized per-track eventually anyway
            if controls.get(msg.control, -999) != msg.value:
                add(msg)
                controls[msg.control] = msg.value
        elif msg.type == 'program_change':
            # still ignoring channel number
            if program != msg.program:
                add(msg)
                program = msg.program
        elif msg.type == 'pitchwheel':
            # ditto
            if pitchwheel != msg.pitch:
                add(msg)
                pitchwheel = msg.pitch
        else:
            add(msg)

    return new_track


def clean_up_midi(midi: mido.MidiFile):
    for i in range(len(midi.tracks)):
        midi.tracks[i] = clean_up_track(midi.tracks[i])


def parse_position(input: int | float | str) -> Fraction:
    """
    Accepts:
    - int (5)
    - float (5.3)
    - int in string ('5')
    - float in string ('5.3')
    - fraction in string ('5/3')
    - mixed fraction in string ('2 5/3')
    And returns a matching Fraction.
    """
    if isinstance(input, (int, float)):
        return Fraction(input).limit_denominator(1000)

    if '.' in input:
        return Fraction(Decimal(input))
    elif '/' in input:
        split = input.split()
        if len(split) == 0:
            raise ValueError('empty position')
        elif len(split) == 1:
            return Fraction(input)
        elif len(split) == 2:
            return int(split[0]) + Fraction(split[1])
        else:
            raise ValueError(f'unparsable position string: {input!r}')
    else:
        return Fraction(input)


def measure_to_ticks(
    ticks_per_beat: int,
    value: float | Fraction,
    *,
    time_signatures: dict[int, TimeSignature],
) -> int:
    if not isinstance(value, Fraction):
        value = Fraction(value).limit_denominator(1000)

    num_beats = Fraction()
    time_sig = Fraction(4, 4)

    for measure_num in range(1, int(value)):
        time_sig = time_signatures.get(measure_num, time_sig)
        num_beats += time_sig * 4

    whole_part = math.floor(value)
    frac_part = value - whole_part

    time_sig = time_signatures.get(whole_part, time_sig)
    num_beats += frac_part * time_sig * 4

    return round(ticks_per_beat * num_beats)


def ticks_to_measure(
    ticks_per_beat: int,
    value: int,
    *,
    time_signatures: dict[int, TimeSignature],
) -> Fraction:

    beat_target = Fraction(value, ticks_per_beat)
    time_sig = Fraction(4, 4)
    measure_num = 1
    beat_pos = Fraction()

    while True:
        time_sig = time_signatures.get(measure_num, time_sig)
        end_beat_pos = beat_pos + time_sig * 4

        if end_beat_pos > beat_target:
            remaining = (beat_target - beat_pos) / (end_beat_pos - beat_pos)
            return measure_num + remaining

        beat_pos = end_beat_pos
        measure_num += 1


def rearrange_by_csv(
    midi: mido.MidiFile,
    csv_data: List[List[str]],
    *,
    time_signatures: dict[int, TimeSignature],
    delete_mode: bool = False,
) -> None:
    """
    Rearrange a MIDI (in-place) according to CSV contents (2D list).
    If delete_mode is True, this function does the opposite -- it
    *removes* sections from the input midi, and returns *that* instead
    of the new / condensed one. This is useful for checking for any
    notes you might've missed.
    """
    # Remove the first row (header)
    csv_data = csv_data[1:]

    # Check how many instruction rows we'll be using
    num_instruction_rows = 0
    while num_instruction_rows + 1 < len(csv_data) \
            and csv_data[num_instruction_rows + 1][0] == str(num_instruction_rows + 1):
        num_instruction_rows += 1

    # Remove the first column
    csv_data = [r[1:] for r in csv_data]

    # Get the time-labels row, and the instructions rows
    time_labels_row = csv_data.pop(0)
    while time_labels_row and not time_labels_row[-1]:
        time_labels_row.pop()
    time_labels = [
        measure_to_ticks(midi.ticks_per_beat, parse_position(s), time_signatures=time_signatures)
        for s in time_labels_row
    ]
    instruction_rows = csv_data[:num_instruction_rows]

    def get_track_for_instruction(instruction, start_tick, end_tick):
        if '{' in instruction:
            track_names = [n.strip() for n in braceexpand.braceexpand(instruction, escape=False)]

            if delete_mode:
                for n in track_names:
                    remove_inplace(get_track_by_name(midi, n), start_tick, end_tick)
            else:

                # The tricky bit here is panning. We want the combined track to
                # have the panning value of the first track name specified.
                tracks = [
                    extract(get_track_by_name(midi, n), start_tick, end_tick)
                    for n in track_names
                ]

                # Get the first track's state messages
                inst_pan = get_state_msgs_at(get_track_by_name(midi, track_names[0]), start_tick)

                # Wipe all pan messages from all tracks
                for track in tracks:
                    i = 0
                    while i < len(track):
                        if track[i].type == 'control_change' and track[i].control == 10:
                            del track[i]
                        else:
                            i += 1

                # Re-insert them
                inst_pan[0].time = start_tick
                tracks.append(inst_pan)

                return mido.merge_tracks(tracks)

        else:
            if delete_mode:
                remove_inplace(get_track_by_name(midi, instruction), start_tick, end_tick)
                return []
            else:
                return extract(get_track_by_name(midi, instruction), start_tick, end_tick)

    # Build the output midi tracks!
    new_tracks = []
    for track_i, track_instructions in enumerate(instruction_rows):
        new_track_components = []

        copy_name = None
        copy_start_tick = None

        for start_tick, instruction in zip(time_labels, track_instructions):

            if instruction not in {'', 'x'}:
                if copy_name is not None:
                    new_track_components.append(get_track_for_instruction(copy_name, copy_start_tick, start_tick))
                copy_name = instruction
                copy_start_tick = start_tick

        if copy_name is not None:
            new_track_components.append(get_track_for_instruction(copy_name, copy_start_tick, start_tick))

        if not delete_mode:
            new_track = mido.merge_tracks(new_track_components)
            new_track = clean_up_track(new_track, force_channel=track_i)
            new_track.name = f'Track {track_i+1}'
            new_tracks.append(new_track)

    if not delete_mode:
        midi.tracks = new_tracks


def suggest_better_csv_arrangement(
    csv_data: List[List[str]],
    *,
    loop_start_index: int,
    num_tracks: int = 16,
) -> None:
    """
    Suggest an equivalent CSV track arrangement that also preserves
    instruments across the loop point boundary.

    Prints the new instrument assignments and CSV output to stdout.
    """
    START_TIME = 1.0

    Session = collections.namedtuple('Session', 'instrument start_time')

    # Remove the first row and first column (headers)
    csv_data = csv_data[1:]
    csv_data = [r[1:] for r in csv_data]

    # Get the time-labels row, and the instructions rows
    time_labels_row = csv_data.pop(0)
    while time_labels_row and not time_labels_row[-1]:
        time_labels_row.pop()
    time_labels = [float(s) for s in time_labels_row]
    instruction_rows = csv_data[:num_tracks]

    def get_tracks_for_instruction(instruction):
        return [n.strip() for n in braceexpand.braceexpand(instruction, escape=False)]

    # Make a set of all "sessions" (contiguous time intervals when an
    # instrument is playing). These will be our Z3 variables.
    # Also keep track of their "x" patterns (which of their following
    # timestamps they are and aren't playing for) so we can preserve
    # that in the output
    all_sessions = set()
    session_x_patterns = {}
    for track_instructions in instruction_rows:
        current_x_pattern = []
        for start_time, instruction in zip(time_labels, track_instructions):
            if instruction in {'', 'x'}:
                current_x_pattern.append(instruction)
            else:
                session = Session(instruction, start_time)
                all_sessions.add(session)
                session_x_patterns[session] = current_x_pattern = []

    for pat in session_x_patterns.values():
        while pat and not pat[-1]:
            pat.pop()

    # Define some variables to store sets of pairs of sessions that
    # must, or must not, be on the same track as each other
    constraints_equal_pairs = set()
    constraints_unequal_pairs = set()

    # Collect "vertical" constraints ("this session and that session
    # must not overlap because they play at the same time")
    active_sessions = [Session(t[0], START_TIME) for t in instruction_rows]
    for i, start_time in enumerate(time_labels):
        # Update active sessions
        for j, row in enumerate(instruction_rows):
            instruction = row[i]
            if instruction not in {'', 'x'}:
                active_sessions[j] = Session(instruction, start_time)

        # Collect constraints
        # I really wish there was a better way to express "all of these
        # variables must have unique values" than this...
        for a, s1 in enumerate(active_sessions):
            for b in range(a + 1, len(active_sessions)):
                s2 = active_sessions[b]
                assert s1 != s2, f'session {s1} is playing on two different tracks at time {start_time}?'
                constraints_unequal_pairs.add((s1, s2))

    # Collect "horizontal" constraints ("this session must share the
    # same track as that session because they have an instrument in
    # common and it might hold a note across the division point")
    for track_instructions in instruction_rows:
        prev_instruction = track_instructions[0]
        current_session = prev_session = Session(prev_instruction, START_TIME)

        for start_time, instruction in zip(time_labels, track_instructions):
            if instruction not in {'', 'x'}:
                current_session = Session(instruction, start_time)

            if prev_session != current_session:
                if set(get_tracks_for_instruction(prev_instruction)) & set(get_tracks_for_instruction(instruction)):
                    # The previous and new sessions have one or more
                    # instruments in common. Conservatively assume
                    # that's on purpose.
                    constraints_equal_pairs.add((prev_session, current_session))

            if instruction not in {'', 'x'}:
                prev_instruction = instruction
                prev_session = current_session

    if loop_start_index is not None:
        # Separately, pair up matching instruments across the loop point
        # boundary
        loop_end_index = len(time_labels) - 1

        # First, figure out what sessions are active at those points
        active_sessions = [Session(t[0], START_TIME) for t in instruction_rows]
        active_sessions_at_loop_start = None
        active_sessions_at_loop_end = None
        for i, start_time in enumerate(time_labels):
            for j, row in enumerate(instruction_rows):
                instruction = row[i]
                if instruction not in {'', 'x'}:
                    active_sessions[j] = Session(instruction, start_time)

            if i == loop_start_index:
                active_sessions_at_loop_start = list(active_sessions)
            elif i == loop_end_index:
                active_sessions_at_loop_end = list(active_sessions)

        # And then just pair them up
        active_insts_to_sessions_at_loop_end = {s.instrument: s for s in active_sessions_at_loop_end}
        for s1 in active_sessions_at_loop_start:
            s2 = active_insts_to_sessions_at_loop_end.get(s1.instrument)
            if s2 is not None:
                constraints_equal_pairs.add((s1, s2))

    # ====

    # Now we convert this into a graph-coloring problem.

    # https://math.stackexchange.com/q/2511608

    # First we need to decide which nodes exist.
    # Basically, any sessions which are "equal" will just be a single
    # node.
    session_to_node = {s: s for s in all_sessions}
    for s1, s2 in constraints_equal_pairs:
        session_to_node[s2] = session_to_node[s1]
    all_nodes = set(session_to_node.values())

    all_edges = [(session_to_node[s1], session_to_node[s2]) for (s1, s2) in constraints_unequal_pairs]

    # Solve!
    print(f'graph coloring: {len(all_nodes)} nodes, {len(all_edges)} edges')

    # After implementing this with both Z3 and NetworkX, Z3 seems to
    # work better (probably mainly because NetworkX doesn't actually let
    # you specify a color limit and we have to work around that badly)
    node_to_color = graph_coloring_z3(all_nodes, all_edges, num_tracks)

    session_to_color = {s: node_to_color[session_to_node[s]] for s in all_sessions}

    # ====

    # Now we convert the "colors" to actual track values. I do it in a
    # way that keeps the initial instruments in the same order as in the
    # original csv, for aesthetic reasons :P
    color_to_track = {}
    for i, row in enumerate(instruction_rows):
        session = Session(row[0], START_TIME)
        color_to_track[session_to_color[session]] = i + 1

    session_to_track = {k: color_to_track[v] for k, v in session_to_color.items()}

    # First, print the assignments individually
    for start_time in time_labels:
        for s in sorted(all_sessions, key=lambda s: s.start_time):
            s_str = f'{s.instrument} @ {s.start_time}'
            print(f'{s_str:<40}-> {session_to_track[s]}')

    # Now make a CSV
    new_csv = []
    sessions_per_track = [set() for _ in range(num_tracks)]
    for s in all_sessions:
        sessions_per_track[session_to_track[s] - 1].add(s)
    for sessions in sessions_per_track:
        new_csv_row = []
        time_to_session = {s.start_time: s for s in sessions}
        current_pattern = []
        for start_time in time_labels:
            if current_pattern:
                new_csv_row.append(current_pattern.pop(0))
            elif start_time in time_to_session:
                session = time_to_session[start_time]
                new_csv_row.append(session.instrument)
                current_pattern = list(session_x_patterns[session])
            else:
                new_csv_row.append('')
        new_csv.append(new_csv_row)

    # sigh why is the csv module's api so bad
    tio = StringIO()
    csv_writer = csv.writer(tio)
    csv_writer.writerows(new_csv)
    tio.seek(0)
    print(tio.read())


def graph_coloring_z3(nodes: list, edges: list, num_colors: int) -> dict:
    """
    Graph-coloring using Z3.

    `nodes` is a list of any hashable objects, and `edges` is a list of
    2-tuples of things from `nodes`. The return value is a dict of
    things-from-`nodes` to color values (arbitrary ints, but at most
    `num_colors` unique ones).
    """
    import z3  # pip install z3-solver

    solver = z3.Solver()

    node_to_var = {}
    for n in nodes:
        var = z3.Int(str(n))
        node_to_var[n] = var
        solver.add(var >= 0)
        solver.add(var < num_colors)

    for n1, n2 in edges:
        solver.add(node_to_var[n1] != node_to_var[n2])

    if solver.check() == 'unsat':
        raise RuntimeError('z3 returned unsat')

    model = solver.model()
    return {n: model[node_to_var[n]].as_long() for n in nodes}


def graph_coloring_networkx(nodes: list, edges: list, num_colors: int) -> dict:
    """
    Graph-coloring using NetworkX.

    `nodes` is a list of any hashable objects, and `edges` is a list of
    2-tuples of things from `nodes`. The return value is a dict of
    things-from-`nodes` to color values (arbitrary ints, but at most
    `num_colors` unique ones).
    """
    import networkx as nx  # pip install networkx

    graph = nx.Graph()
    for n in nodes:
        graph.add_node(n)
    for n1, n2 in edges:
        graph.add_edge(n1, n2)

    # OK, so, a few things to keep in mind here:
    #
    # - There's no way to ask networkx to limit the number of colors to
    #   some value (in our case, a fixed number around 16)
    # - The coloring you get -- and the number of colors it uses -- is
    #   non-deterministic
    # - NetworkX *does* let you specify the "strategy", from a fixed
    #   list (see the documentation for the "greedy_color" function)
    #
    # So basically, we'll call it a bunch of times with randomly chosen
    # strategies, until either it gives us one using only 16 colors or
    # we give up.

    i = 0
    while True:
        strategy = random.choice([
            'largest_first',
            'random_sequential',
            'smallest_last',
            'independent_set',
            'connected_sequential_bfs',
            'connected_sequential_dfs',
            'saturation_largest_first'])
        coloring = nx.coloring.greedy_color(graph, strategy)
        if len(set(coloring.values())) <= num_colors:
            return coloring

        i += 1
        if i % 1000 == 0:
            print(f'tried {i} times')


def get_track_by_name(midi: mido.MidiFile, name: str) -> mido.MidiTrack:
    """
    Get the track with the given name
    """
    # support both MidiFile objects and lists
    if hasattr(midi, 'tracks'):
        tracks = midi.tracks
    else:
        tracks = midi

    for track in tracks:
        if track.name == name:
            return track

    raise ValueError(f'not found: {name}')


def get_track_start_meta_msgs(track: mido.MidiTrack) -> mido.MidiTrack:
    """
    Get all the meta messages at the start of a track, EXCEPT for
    instrument/pan/vibrato settings
    """
    new_track = mido.MidiTrack()

    for msg in track:
        if msg.time or msg.type == 'note_on':
            break
        elif msg.type == 'program_change':
            continue
        elif msg.type == 'control_change' and msg.control == ControllerMsg.PAN_10.value:
            continue
        elif msg.type == 'control_change' and msg.control == ControllerMsg.SOUND_CONTROLLER_8.value:
            continue

        new_track.append(msg.copy())

    return new_track


def get_state_msgs_at(track: mido.MidiTrack, tick: int = 0):
    """
    Create messages that indicate some important channel attributes
    (instrument, pan, vibrato, and channel-volume) at a certain time.
    The `time` attributes are guaranteed to be 0.
    """
    prg_msg = None
    pan_msg = None
    # Not all MIDIs will have channel-volume and vibrato events, so we
    # do include defaults for those
    cc11_msg = mido.Message(
        type='control_change',
        control=ControllerMsg.EXPRESSION_CONTROLLER_11.value,
        value=127,
    )
    vib_msg = mido.Message(
        type='control_change',
        control=ControllerMsg.SOUND_CONTROLLER_8.value,
        value=0,
    )

    time = 0
    for msg in track:
        time += msg.time

        if time > tick:
            break

        if msg.type == 'program_change':
            prg_msg = msg.copy(time=0)
        elif msg.type == 'control_change' and msg.control == ControllerMsg.SOUND_CONTROLLER_8.value:
            vib_msg = msg.copy(time=0)
        elif msg.type == 'control_change' and msg.control == ControllerMsg.PAN_10.value:
            pan_msg = msg.copy(time=0)
        elif msg.type == 'control_change' and msg.control == ControllerMsg.EXPRESSION_CONTROLLER_11.value:
            cc11_msg = msg.copy(time=0)

    if prg_msg is None or pan_msg is None:
        # Could use defaults here, but it's better to fail
        # loudly, because this *should* never happen
        raise RuntimeError(f"Couldn't find inst/pan messages in {track.name!r} at {tick}")

    return [prg_msg, pan_msg, cc11_msg, vib_msg]


def extract(track: mido.MidiTrack, start_tick: int, end_tick: int, *, name: str | None = None):
    """
    Create a new track which contains only the events between start_tick
    (inclusive) and end_tick (exclusive), along with extra meta messages
    added to get the right instrument and pan
    """
    track_meta_msgs = get_track_start_meta_msgs(track)
    tick_meta_msgs = get_state_msgs_at(track, start_tick)

    new_track = mido.MidiTrack()

    time = 0
    for msg in track:
        time += msg.time

        if time < start_tick:
            continue
        elif time >= end_tick:
            break

        if not new_track:
            new_track.extend(tick_meta_msgs)
            new_track.append(msg.copy(time=0))
            new_track[0].time = time
        else:
            new_track.append(msg.copy())

    # don't allow pitch bends to escape the extracted portion
    new_track.append(mido.Message(type='pitchwheel', pitch=0, time=0))

    new_track = mido.merge_tracks([
        track_meta_msgs,
        new_track,
    ])

    if name is not None:
        new_track.name = name

    return new_track


def remove(track: mido.MidiTrack, start_tick: int, end_tick: int, *, name: str | None = None):
    """
    Create a new track which removes events between start_tick
    (inclusive) and end_tick (exclusive).
    """
    track_meta_msgs = get_track_start_meta_msgs(track)

    new_track = mido.MidiTrack()

    first_msg_after_gap = True

    time = time_of_last_added_msg = 0
    for msg in track:
        time += msg.time

        if time < start_tick:
            new_track.append(msg.copy())
            time_of_last_added_msg = time
        elif time >= end_tick:
            if first_msg_after_gap:
                new_track.append(msg.copy(time=(time - time_of_last_added_msg)))
                time_of_last_added_msg = time
                first_msg_after_gap = False
            else:
                new_track.append(msg.copy())
                time_of_last_added_msg = time

    if start_tick == 0:
        # if start_tick > 0, we're already naturally preserving these
        # messages
        new_track = mido.merge_tracks([
            track_meta_msgs,
            new_track,
        ])

    if name is not None:
        new_track.name = name

    return new_track


def remove_inplace(track: mido.MidiTrack, start_tick: int, end_tick: int, *, name: str | None = None):
    """
    remove() that operates in-place.
    """
    removed = remove(track, start_tick, end_tick, name=name)
    track.clear()
    track.extend(removed)


def slice_merge(from_track: mido.MidiTrack, to_track: mido.MidiTrack, start_tick: int, end_tick: int, *, name: str | None = None):
    """
    Create a new track which is
    - from_track, up to start_tick (exclusive)
    - to_track, up to end_tick (exclusive)
    - from_track after that
    """

    extracted_src_track = extract(from_track, start_tick, end_tick)
    emptied_dst_track = remove(to_track, start_tick, end_tick)

    meta_restore_messages = get_state_msgs_at(to_track, end_tick)
    meta_restore_messages[0].time = end_tick

    new_track = mido.merge_tracks([extracted_src_track, meta_restore_messages, emptied_dst_track])

    if name is not None:
        new_track.name = name

    return new_track


def slice_merge_inplace(from_track: mido.MidiTrack, to_track: mido.MidiTrack, start_tick: int, end_tick: int):
    """
    In-place version of slice_merge: moves events from from_track to
    to_track, modifying both tracks in the process
    """
    from_track_name = from_track.name
    to_track_name = to_track.name

    meta_restore_messages = get_state_msgs_at(from_track, end_tick)
    meta_restore_messages[0].time = end_tick

    remaining_src_track = mido.merge_tracks([
        meta_restore_messages,
        remove(from_track, start_tick, end_tick, name=from_track_name),
    ])
    merged_track = slice_merge(from_track, to_track, start_tick, end_tick, name=to_track_name)

    from_track.clear()
    from_track.extend(remaining_src_track)

    to_track.clear()
    to_track.extend(merged_track)


def scale_panning(midi_or_track: mido.MidiFile | mido.MidiTrack, scale_factor: float) -> None:
    """
    Scale all pan values by some factor
    """
    for track in track_list(midi_or_track):
        for msg in track:
            if msg.type == 'control_change' and msg.control == ControllerMsg.PAN_10.value:
                msg.value = clamp(round((msg.value - 64) * scale_factor) + 64, 0, 127)


def scale_velocities(midi_or_track: mido.MidiFile | mido.MidiTrack, scale_factor: float) -> None:
    """
    Scale all velocities by some factor
    """
    for track in track_list(midi_or_track):
        i = 0
        have_deleted = set()
        while i < len(track):
            msg = track[i]
            if msg.type == 'note_on' and msg.velocity > 0:
                msg.velocity = clamp(round(msg.velocity * scale_factor), 0, 127)
                if msg.velocity == 0:
                    if i + 1 < len(track):
                        track[i + 1].time += msg.time
                    del track[i]
                    have_deleted.add(msg.note)
                    continue
            elif (msg.type == 'note_off' or (msg.type == 'note_on' and msg.velocity == 0)) and msg.note in have_deleted:
                del track[i]
                have_deleted.discard(msg.note)
                continue
            i += 1


def scale_cc11s(midi_or_track: mido.MidiFile | mido.MidiTrack, scale_factor: float) -> None:
    """
    Scale all CC 11 (channel colume) event values by some factor
    """
    for track in track_list(midi_or_track):
        for msg in track:
            if msg.type == 'control_change' and msg.control == ControllerMsg.EXPRESSION_CONTROLLER_11.value:
                msg.value = clamp(round(msg.value * scale_factor), 0, 127)


def switch_cc_11_to_cc_7(midi_or_track: mido.MidiFile | mido.MidiTrack) -> None:
    """
    Switch CC 11 channel-volume events (recognized by MuseScore but not
    seq64) to CC 7 (recognized by seq64 but not MuseScore)
    """
    for track in track_list(midi_or_track):
        for msg in track:
            if msg.type == 'control_change' and msg.control == ControllerMsg.EXPRESSION_CONTROLLER_11.value:
                msg.control = ControllerMsg.CHANNEL_VOLUME_7.value


def delete_chorus_events(midi_or_track: mido.MidiFile | mido.MidiTrack) -> None:
    """
    Delete chorus events (CC 93) (workaround for sauraen/seq64 issue #34)
    """
    for track in track_list(midi_or_track):
        i = 0
        while i < len(track):
            if track[i].type == 'control_change' and track[i].control == ControllerMsg.EFFECTS_3_DEPTH.value:
                del track[i]
            else:
                i += 1


def extract_metadata_tracks(midi: mido.MidiFile, num: int) -> mido.MidiTrack:
    """
    When exporting to MIDI, MuseScore includes a dummy "track 0" that
    just sets the tempo and stuff. FL Studio actually adds *two* such
    metadata tracks.
    This function removes these metadata track(s) and returns it/them as
    a single combined track, which you can then merge back into the
    first track. (If you're shuffling the other tracks around, you may
    want to wait to merge this in until after you're done with that.)
    """
    meta_track = None
    for _ in range(num):
        # Have to be careful to avoid inserting an "end of track"
        # message, since that could kill track 1 too early
        this = [e for e in midi.tracks[0] if e.type != 'end_of_track']

        if meta_track is None:
            meta_track = this
        else:
            meta_track = mido.merge_tracks([meta_track, this])

        del midi.tracks[0]

    meta_track = mido.MidiTrack(meta_track)
    meta_track.name = 'meta'
    return meta_track


def deep_copy_midi(midi: mido.MidiFile) -> mido.MidiFile:
    """
    Make a deep copy of a midi file.
    """
    bio = BytesIO()
    midi.save(file=bio)
    bio.seek(0)
    return mido.MidiFile(file=bio)
