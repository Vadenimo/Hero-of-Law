#!/bin/env python3

import argparse
import csv
import dataclasses
import itertools
import json
from pathlib import Path
import shutil
import subprocess
from typing import Any
import sys

import mido

import lib_oot_midi_preprocessing as lib
from lib_midi_controller_message_types import ControllerMsg


"""
MUSESCORE EXPORTING PROCEDURE (yes, this entire process every time you export):

- Make sure there's no loop marker at the end of the song
- Close MuseScore

- Re-open MuseScore and the project file
- View -> Synthesizer -> Dynamics
- "Velocity only" (ignore the Advanced Settings buttons)
- "Save to Score"
- File -> Export... -> MIDI -> Export... -> "<name>_velocity_only.mid"
- (No need to save)
- Close Musescore

- Re-open MuseScore and the project file
- View -> Synthesizer -> Dynamics
- "CC events only", "CC 11" (ignore the Advanced Settings buttons)
- "Save to Score"
- File -> Export... -> MIDI -> Export... -> "<name>_cc11_only.mid"
- ABSOLUTELY DO NOT SAVE
- Close Musescore
"""


SEQ64_CONSOLE = Path('../Tool/seq64_v2.4.0/seq64_console')
BUILD_FOLDER = Path('../Build')
MASTER_INSTRUMENTS_DIR = Path('instruments')

WHISTLE_ID = 62
DEFAULT_DRUM_INSTRUMENT_ID = 45


def read_vanilla_sequence_bank_mapping() -> dict[int, int]:
    """
    Create a {sequence ID: bank ID} mapping from the vanilla
    sequence-table data
    """
    sequence_table_path = BUILD_FOLDER / 'audio/sequences/_vanilla-1.0/sequencetable.tsv'
    seq_to_bank = {}
    with sequence_table_path.open('r', encoding='utf-8') as f:
        for i, row in enumerate(csv.reader(f, delimiter='\t')):
            # skip header row
            if i == 0:
                continue

            seq_to_bank[int(row[0], 16)] = int(row[4], 16)

    return seq_to_bank


@dataclasses.dataclass
class Seq64Output:
    stdout: str
    stderr: str

    def converted_ppqn_from_to(self) -> tuple[int, int] | None:
        for line in self.stdout.splitlines():
            if line.startswith('Converting ') and line.endswith(' ppqn'):
                line = line.split()
                return int(line[1]), int(line[-2])
        return None


def convert_with_seq64(in_path: Path, out_path: Path, daw: str, *, smart_loop: bool = True) -> Seq64Output:
    out_path.unlink(missing_ok=True)
    EXE_EXTENSION = '.exe' if sys.platform == 'win32' else ''
    res = subprocess.run(
        [
            str(SEQ64_CONSOLE.resolve()) + EXE_EXTENSION,
            f'--in={in_path}',
            f'--out={out_path}',
            '--game=zelda',
            '--dialect=community-music',
            *(['--smartloop=false'] if not smart_loop else []),
            *(['--flstudio=true'] if daw == 'fl_studio' else []),
        ],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )

    stdout = res.stdout.decode('utf-8')
    stderr = res.stderr.decode('utf-8')

    if not out_path.is_file():
        print(stdout)
        print(stderr)
        raise RuntimeError('seq64 failed')

    return Seq64Output(stdout, stderr)


def isolate_track(midi, track_num_or_nums):
    """
    Silence all but one track. Useful for debugging.
    """
    # The idea is to delete all note_on/note_off events, but preserve
    # everything else (such as tempo changes)

    if isinstance(track_num_or_nums, int):
        track_nums = {track_num_or_nums}
    else:
        track_nums = track_num_or_nums

    for i, track in enumerate(midi.tracks):
        if i in track_nums:
            continue

        j = 0
        while j < len(track):
            if track[j].type in {'note_on', 'note_off'}:
                track[j + 1].time += track[j].time
                del track[j]
            else:
                j += 1

    return track


def crop_beginning(midi: mido.MidiFile, ticks: int) -> None:
    """
    Skip past the first "ticks" ticks of the midi. Useful for debugging.
    """
    # Find tempo
    tempo_msg = None
    time = 0
    for msg in midi.tracks[0]:
        time += msg.time

        if time > ticks:
            break

        if msg.type == 'set_tempo':
            tempo_msg = msg.copy(time=0)

    # Delete the beginning of each track
    for i, track in enumerate(midi.tracks):
        midi.tracks[i] = track = lib.extract(track, ticks, 99999999999, name=track.name)

        to_delete = []

        abs_time = 0
        for j, event in enumerate(track):
            prev_abs_time = abs_time
            new_abs_time = prev_abs_time + event.time
            event.time = max(new_abs_time - max(prev_abs_time, ticks), 0)
            if new_abs_time >= ticks:
                break
            abs_time = new_abs_time

            if event.type == 'set_tempo' and event.time == 0:
                to_delete.append(j)

        for j in reversed(to_delete):
            del track[j]

    # Re-add the tempo event if needed
    if tempo_msg is not None:
        midi.tracks[0].insert(0, tempo_msg)


def add_track_sync_notes(
    midi: mido.MidiFile,
    *,
    instrument: int,
    velocity: int = 96,
    length_in_beats: float = 4.0,
):
    """
    Add a note to the beginning of every track, so they can be
    accurately synced if recorded separately
    """
    for track in midi.tracks:
        start_meta = lib.get_inst_pan_and_cc_11_msgs_at(track, 0)

        # find where stuff starts, and remove any preceding instrument
        # changes because OoT (or maybe seq64) is dumb and can only
        # handle one of those per tick
        i = 0
        while i < len(track):
            msg = track[i]
            if msg.time != 0 or msg.type == 'note_on':
                break
            if msg.type == 'program_change':
                del track[i]
            else:
                i += 1
        first_message_with_time = i

        track[first_message_with_time:first_message_with_time] = [
            mido.Message('program_change', program=instrument),  # instrument
            mido.Message('control_change', control=10, value=64),  # pan
            mido.Message('control_change', control=11, value=127),  # channel volume
            mido.Message('note_on', note=65, velocity=velocity, time=1),
            mido.Message('note_on', note=65, velocity=0, time=round(midi.ticks_per_beat * length_in_beats)-1),
            *start_meta,
        ]


def determine_drum_instruments() -> set[int]:
    """
    Figure out which instruments are drums
    """
    drum_instruments = set()
    for i in range(255):
        try:
            path = get_instrument_tsv_by_id(i)
        except FileNotFoundError:
            continue
        if path.stat().st_size > 500:
            drum_instruments.add(i)
    return drum_instruments


def get_instrument_tsv_by_id(id: int) -> Path:
    """
    Return the path to the instrument tsv file with the given ID
    """
    for path in MASTER_INSTRUMENTS_DIR.glob('*.tsv'):
        if path.name.startswith(f'{id} '):
            return path
    raise FileNotFoundError(f"Couldn't find a file for instrument {id}")


def add_vibrato_by_json(
    metadata: dict,
    midi: mido.MidiFile,
    *,
    cc: ControllerMsg | int = ControllerMsg.SOUND_CONTROLLER_8,
) -> None:
    """
    If the metadata indicates that certain tracks should have vibrato
    applied, do that
    """
    if hasattr(cc, 'value'):
        cc = cc.value

    for track, track_meta in zip(midi.tracks, metadata.get('tracks', [])):
        amount = track_meta.get('vibrato')
        if amount:
            msg = mido.Message(type='control_change', control=cc, value=amount)
            track.insert(0, msg)


VELOCITIES = 1
CC11S = 2
def scale_volumes_by_json(
    metadata: dict,
    midi: mido.MidiFile,
    type: int,
) -> None:
    """
    If the metadata indicates that certain tracks should have their
    volumes scaled, do that
    """
    all_track_metas = metadata.get('tracks', [])
    if 'velocity_scale' in metadata:
        raise ValueError('it\'s "volume_scale", not "velocity_scale"')
    master_volume_scale = metadata.get('volume_scale')
    for i, track in enumerate(midi.tracks):
        if i < len(all_track_metas):
            track_meta = all_track_metas[i]
        else:
            track_meta = {}
        if 'velocity_scale' in track_meta:
            raise ValueError('it\'s "volume_scale", not "velocity_scale"')
        volume_scale = track_meta.get('volume_scale')
        if master_volume_scale is not None or volume_scale is not None:
            a = master_volume_scale if master_volume_scale is not None else 1.0
            b = volume_scale if volume_scale is not None else 1.0
            if type == VELOCITIES:
                lib.scale_velocities(track, a * b)
            else:
                lib.scale_cc11s(track, a * b)


def scale_panning_by_json(
    metadata: dict,
    midi: mido.MidiFile,
) -> None:
    """
    If the metadata indicates that certain tracks should have their
    panning scaled, do that
    """
    all_track_metas = metadata.get('tracks', [])
    # MuseScore MIDIs are exported with panning that isn't wide enough
    master_pan_default = 2.0 if metadata['daw'] == 'musescore' else 1.0
    master_pan_scale = metadata.get('pan_scale', master_pan_default)
    for i, track in enumerate(midi.tracks):
        if i < len(all_track_metas):
            track_meta = all_track_metas[i]
        else:
            track_meta = {}
        pan_scale = track_meta.get('pan_scale')
        b = pan_scale if pan_scale is not None else 1.0
        lib.scale_panning(track, master_pan_scale * b)


def force_instrument_ids_by_json(
    metadata: dict,
    midi: mido.MidiFile,
) -> None:
    """
    If the metadata indicates that certain tracks should have their
    instrument IDs forced to certain values, do that
    """
    for track, track_meta in zip(midi.tracks, metadata.get('tracks', [])):
        new_inst_id = track_meta.get('force_instrument_id')
        if new_inst_id:
            for msg in track:
                if msg.type == 'program_change':
                    msg.program = new_inst_id


def force_exact_length(
    midi_or_track: mido.MidiFile | mido.MidiTrack,
    length: int,
) -> None:
    """
    Force every track in the midi to be an exact number of ticks long,
    lengthening any that are too short and truncating any that are too
    long
    """
    target_time = length

    for track in lib.track_list(midi_or_track):
        new_events = []
        time = 0
        for msg in track:
            new_events.append(msg)
            time += msg.time
            if time > target_time:
                time -= msg.time
                msg.time = target_time - time
                time = target_time
                break

        if time < target_time:
            if new_events and new_events[-1].type == 'end_of_track':
                new_events[-1].time += target_time - time
            else:
                new_events.append(
                    mido.MetaMessage('end_of_track', time = target_time - time)
                )

        track.clear()
        track.extend(new_events)


def get_time_signatures_from_json(metadata: dict) -> dict[int, lib.TimeSignature]:
    """
    Return the list of time signature changes from the JSON metadata.
    """
    if 'time_signatures' in metadata:
        time_signatures = {}

        for measure, ts in metadata['time_signatures'].items():
            a, b = ts.split('/')
            time_signatures[int(measure)] = lib.TimeSignature(int(a), int(b))

        return time_signatures

    else:
        return {'1': lib.TimeSignature(4, 4)}


def merge_midis_by_json(
    metadata: dict,
    vel_midi: mido.MidiFile,
    cc11_midi: mido.MidiFile,
    time_signatures: dict[int, lib.TimeSignature],
) -> mido.MidiFile:
    """
    Merge the velocity-only and CC-11-only MIDIs according to the
    "cc11_regions" metadata
    """
    track_names = [t['name'] for t in metadata.get('tracks', [])]
    midi = lib.deep_copy_midi(vel_midi)

    for region_def in metadata.get('cc11_regions', []):
        track_idx = track_names.index(region_def['track'])
        start_tick = lib.measure_to_ticks(
            vel_midi.ticks_per_beat,
            lib.parse_position(region_def['start']),
            time_signatures=time_signatures,
        )
        end_tick = lib.measure_to_ticks(
            vel_midi.ticks_per_beat,
            lib.parse_position(region_def['end']),
            time_signatures=time_signatures,
        )

        midi.tracks[track_idx] = mido.merge_tracks([
            # The current version of the track (mostly velocity-based)
            lib.remove(midi.tracks[track_idx], start_tick, end_tick),

            # The extracted CC 11 version
            lib.extract(cc11_midi.tracks[track_idx], start_tick, end_tick),

            # An extra event to reset CC 11 to 127 at the end of the CC 11 part
            [
                mido.Message(
                    'control_change',
                    channel=0,
                    control=ControllerMsg.EXPRESSION_CONTROLLER_11.value,
                    value=127,
                    time=end_tick,
                ),
            ],
        ])

    return midi


def change_channel_volumes_for_ost(
    midi_or_track: mido.MidiFile | mido.MidiTrack,
) -> None:
    """
    Delete any existing CC7 events, convert all CC11 events to CC7
    events, and set initial CC7 values to 127 explicitly.
    """
    for track in lib.track_list(midi_or_track):
        new_events = []
        for msg in track:
            add = True

            if msg.type == 'control_change':
                if msg.control == ControllerMsg.CHANNEL_VOLUME_7.value:
                    add = False
                elif msg.control == ControllerMsg.EXPRESSION_CONTROLLER_11.value:
                    msg.control = ControllerMsg.CHANNEL_VOLUME_7.value

            if add:
                new_events.append(msg)

        new_events.insert(0, mido.Message('control_change', channel=0, control=7, value=127, time=0))

        track.clear()
        track.extend(new_events)


def set_initial_pitch_bends_for_ost(
    midi_or_track: mido.MidiFile | mido.MidiTrack,
) -> None:
    """
    Explicitly set the initial pitch-bend value to 0 for all tracks.
    """
    for track in lib.track_list(midi_or_track):
        track.insert(0, mido.Message('pitchwheel', channel=0, pitch=0, time=0))


@dataclasses.dataclass
class SeqioEvent:
    counter: int  # unique per-instance ID that can be referenced with "{__COUNTER__}"
    time: int  # in units of *input midi* ticks, not *output sequence* ticks
    commands: list[str]

    def iter_output_commands(self):
        for cmd in self.commands:
            cmd = cmd.replace("{__COUNTER__}", str(self.counter))
            yield f'    {cmd.strip()}'


@dataclasses.dataclass
class MidiPreprocessingResult:
    midi: mido.MidiFile
    seqio_events: list[SeqioEvent]
    drum_instrument_id: int | None
    daw: str
    smart_loop: bool
    ingo_whistle_hack: bool


def load_and_preprocess_midi(name: str, composer: str, *, is_for_ost: bool = False) -> MidiPreprocessingResult:
    """
    Load the MIDI, JSON, and CSV files with this name, and Do All The
    Things to get a fully pre-processed MIDI.
    """
    midi_path = Path(f'{composer}/{name}.mid')                    # required for non-musescore
    vel_midi_path = Path(f'{composer}/{name}_velocity_only.mid')  # required for musescore
    cc11_midi_path = Path(f'{composer}/{name}_cc11_only.mid')     # optional
    json_path = Path(f'{composer}/{name}.json')                   # required
    csv_path = Path(f'{composer}/{name}_scheduling.csv')          # optional

    # Open midis
    if vel_midi_path.is_file():
        vel_midi = mido.MidiFile(vel_midi_path) # musescore
    else:
        vel_midi = mido.MidiFile(midi_path) # non-musescore
    if cc11_midi_path.is_file():
        cc11_midi = mido.MidiFile(cc11_midi_path)
        assert len(vel_midi.tracks) == len(cc11_midi.tracks)
    else:
        cc11_midi = None

    # Load metadata JSON file
    metadata = json.loads(json_path.read_text(encoding='utf-8'))
    time_signatures = get_time_signatures_from_json(metadata)

    # Check which DAW-specific hacks to enable
    daw = metadata.get('daw')
    if daw is None:
        raise ValueError(f'{name}: Field "daw" must exist')
    if daw not in {'musescore', 'fl_studio'}:
        raise ValueError(f'{name}: Unknown daw "{daw}" (expected "musescore" or "fl_studio")')

    # Isolate the metadata track(s)
    num_meta_tracks = 2 if daw == 'fl_studio' else 1
    metadata_track = lib.extract_metadata_tracks(vel_midi, num_meta_tracks)
    if cc11_midi is not None:
        # (We intentionally discard this one since it should just be a
        # duplicate of the other one)
        lib.extract_metadata_tracks(cc11_midi, num_meta_tracks)

    # Extract the seqio track if present
    seqio_track_index = None
    for i, t in enumerate(metadata.get('tracks', [])):
        if t.get('seqio', False):
            if seqio_track_index is None:
                seqio_track_index = i
            else:
                raise ValueError('Multiple seqio tracks')

    if seqio_track_index is None:
        seqio_track = None
    else:
        seqio_track = vel_midi.tracks[seqio_track_index]
        del vel_midi.tracks[seqio_track_index]
        if cc11_midi is not None:
            del cc11_midi.tracks[seqio_track_index]
        del metadata['tracks'][seqio_track_index]

    # Figure out seqio events
    seqio_events = []
    seqio_counter = 0
    if seqio_track is not None:
        time = 0
        used_keys = set()
        for msg in seqio_track:
            time += msg.time
            if msg.type == 'note_on' and msg.velocity > 0:
                key = str(msg.note)
                event = metadata.get('seqio', {}).get(key)
                if event is None:
                    raise RuntimeError(f'Unknown seqio event "{key}" at time {time}. '
                        f'Be sure to define this event in your metadata json! ({{"seqio": {{"{key}": [<seqio commands here>], ...}}}})')
                seqio_events.append(SeqioEvent(seqio_counter, time, event))
                seqio_counter += 1
                used_keys.add(key)
        unused_keys = metadata.get('seqio', {}).keys() - used_keys
        if unused_keys:
            raise RuntimeError(f'Not all seqio events defined in the json were used by the midi: {sorted(unused_keys)}')

    # Assign track names
    if 'tracks' in metadata:
        track_names = [t['name'] for t in metadata.get('tracks', [])]
        lib.assign_track_names(track_names, vel_midi)
        if cc11_midi is not None:
            lib.assign_track_names(track_names, cc11_midi)

        # Ensure no duplicate track names
        seen = set()
        for track in vel_midi.tracks:
            if track.name in seen:
                raise RuntimeError(f'Duplicate track name "{track.name}"')
            seen.add(track.name)

    # Add vibrato to certain channels if needed
    vibrato_cc = ControllerMsg.MODULATION_WHEEL_1 if is_for_ost else ControllerMsg.SOUND_CONTROLLER_8
    add_vibrato_by_json(metadata, vel_midi, cc=vibrato_cc)
    if cc11_midi is not None:
        add_vibrato_by_json(metadata, cc11_midi, cc=vibrato_cc)

    if not is_for_ost:
        # Normalize velocities, CC 7s, and CC 11s
        lib.normalize_velocities(vel_midi)
        lib.normalize_cc7s(vel_midi)
        lib.normalize_cc11s(vel_midi)
        if cc11_midi is not None:
            lib.normalize_velocities(cc11_midi)
            lib.normalize_cc7s(cc11_midi)
            lib.normalize_cc11s(cc11_midi)

        # Scale volumes according to the metadata JSON
        scale_volumes_by_json(metadata, vel_midi, VELOCITIES)
        if cc11_midi is not None:
            scale_volumes_by_json(metadata, cc11_midi, CC11S)

    # Scale panning according to the metadata JSON
    scale_panning_by_json(metadata, vel_midi)
    if cc11_midi is not None:
        scale_panning_by_json(metadata, cc11_midi)

    # Force certain instrument IDs in certain channels if needed
    force_instrument_ids_by_json(metadata, vel_midi)
    if cc11_midi is not None:
        force_instrument_ids_by_json(metadata, cc11_midi)

    # Tweaks for Ingo's whistle
    ingo_whistle_hack = metadata.get('ingo_whistle_hack', False)
    if ingo_whistle_hack:
        for track in vel_midi.tracks:
            if track.name == 'whistle':
                # Set all note velocities to max
                lib.scale_velocities(track, 99999)
                for msg in track:
                    # Set channel volume to max
                    if msg.type == 'control_change' and msg.control == ControllerMsg.CHANNEL_VOLUME_7.value:
                        msg.value = 127
                break
        else:
            raise RuntimeError('whistle not found')

    # Merge the two MIDIs
    if cc11_midi is None:
        midi = vel_midi
    else:
        assert metadata is not None
        midi = merge_midis_by_json(metadata, vel_midi, cc11_midi, time_signatures)

    if is_for_ost:
        change_channel_volumes_for_ost(midi)
        set_initial_pitch_bends_for_ost(midi)

        # Merge the metadata track (tempo changes) into track 1
        midi.tracks[0] = mido.merge_tracks([metadata_track, midi.tracks[0]])

        lib.renumber_track_channels_sequentially(midi)

        return MidiPreprocessingResult(
            midi,
            seqio_events,
            None,
            daw,
            metadata['loop_start'] is not None,
            ingo_whistle_hack,
        )

    # If a CSV file is available, use it for rearrangement
    if csv_path.is_file():
        with csv_path.open('r', encoding='utf-8') as csv_file:
            csv_file = list(csv.reader(csv_file))
        lib.rearrange_by_csv(
            midi,
            csv_file,
            time_signatures=time_signatures,
        )
        # Uncomment this to activate the CSV rearrangement tool
        # if name == 'Title':
        #     lib.suggest_better_csv_arrangement(
        #         csv_file,
        #         loop_start_index=0,
        #         num_tracks=13,
        #     )
        #     raise

    # Figure out which drums the song uses, and replace the ID with 127
    all_drum_instruments = determine_drum_instruments()
    found_drum_instruments = lib.fix_drum_patch(midi, all_drum_instruments)
    if len(found_drum_instruments) == 0:
        drum_instrument_id = None
    elif len(found_drum_instruments) == 1:
        drum_instrument_id = found_drum_instruments.pop()
    else:
        raise RuntimeError(f'Found multiple drum instruments in {name}:'
            f' {", ".join(str(id) for id in sorted(found_drum_instruments))}')

    # Merge the metadata track (tempo changes) into track 1
    midi.tracks[0] = mido.merge_tracks([metadata_track, midi.tracks[0]])

    # crop_beginning(
    #     midi,
    #     lib.measure_to_ticks(
    #         midi.ticks_per_beat,
    #         lib.parse_position("66"),
    #         time_signatures=time_signatures,
    #     ),
    # )

    # Various cleanup tasks
    lib.renumber_track_channels_sequentially(midi)
    lib.clean_up_midi(midi)
    lib.check_note_sanity(midi, time_signatures=time_signatures)
    lib.delete_chorus_events(midi)  # (workaround for sauraen/seq64 issue #34)

    # Add loop markers
    loop_start = metadata['loop_start']
    if loop_start is None:
        smart_loop = False
    else:
        smart_loop = True
        lib.add_loop_markers(
            midi,
            lib.measure_to_ticks(
                midi.ticks_per_beat,
                lib.parse_position(loop_start),
                time_signatures=time_signatures,
            ),
        )

    forced_end = metadata.get('force_end')
    if forced_end is not None:
        force_exact_length(
            midi,
            lib.measure_to_ticks(
                midi.ticks_per_beat,
                lib.parse_position(forced_end),
                time_signatures=time_signatures,
            ),
        )

    # # Add sync clap to all tracks
    # add_track_sync_notes(midi, instrument=10)

    return MidiPreprocessingResult(
        midi,
        seqio_events,
        drum_instrument_id,
        daw,
        smart_loop,
        ingo_whistle_hack,
    )


def preprocess_s_file(
    path_in: Path,
    path_out: Path,
    seqio_events: list[SeqioEvent],
    ppqn_scale_factor: float,
    ingo_whistle_hack: bool,
) -> None:
    """
    Perform any preprocessing on a song .s file
    """
    text = path_in.read_text(encoding='utf-8')
    text = text.replace(' cexp ', ' cvol ')

    if seqio_events:
        text = text.splitlines()

        accumulated_time = 0

        # First, read existing "occurrences" (non-delay events in the
        # sequence's _start section, and the ticks at which they happen)
        is_active = False
        section_start_time = accumulated_time
        current_section_time = 0
        section_occurrences_dict = {}
        start_start_line_num = start_end_line_num = None
        for i, line in enumerate(text):
            if line == f'_start:':
                start_start_line_num = i + 1
                is_active = True
                continue
            elif is_active and not line:
                start_end_line_num = i
                break

            if is_active:
                if ' delay ' in line:
                    current_section_time += int(line.split()[-1])
                else:
                    section_occurrences_dict \
                        .setdefault(current_section_time, []) \
                        .append(line)
        section_length = current_section_time
        if start_start_line_num is None or start_end_line_num is None:
            raise RuntimeError(f'unable to determine the start and/or end of _start block')

        # Add the seqio events as new occurrences
        for event in seqio_events:
            event_seq_time = round(event.time * ppqn_scale_factor) - section_start_time
            if 0 <= event_seq_time < section_length:
                section_occurrences_dict \
                    .setdefault(event_seq_time, []) \
                    .extend(event.iter_output_commands())

        # Convert that back to a linear list of lines, recomputing
        # all the delays between each occurrence
        current_section_time = 0
        new_section_lines = []
        for time in sorted(section_occurrences_dict):
            if time > current_section_time:
                new_section_lines.append(f'    delay {time - current_section_time}')
            new_section_lines.extend(section_occurrences_dict[time])
            current_section_time = time
        if section_length > current_section_time:
            new_section_lines.append(f'    delay {section_length - current_section_time}')

        # Insert this back into the sequence
        text[start_start_line_num:start_end_line_num] = new_section_lines

        accumulated_time += section_length

    if ingo_whistle_hack:
        # We take this opportunity to raise the volume of Ingo's whistle
        # above what MIDI allows
        NEW_WHISTLE_VOLUME = 200

        if isinstance(text, str):
            text = text.splitlines()

        changed_count = 0
        for i, line in enumerate(text):
            if line == f'    instr {WHISTLE_ID}':
                for j in range(i - 1, 0, -1):
                    if text[j] == '    cvol 127':
                        text[j] = f'    cvol {NEW_WHISTLE_VOLUME}'
                        changed_count += 1
                        break
                    elif not text[j]:
                        raise RuntimeError("didn't find cvol event for Ingo's whistle")

        print(f"Edited {changed_count} cvol events for Ingo's whistle")

    if isinstance(text, list):
        text = '\n'.join(text)
    path_out.write_text(text, encoding='utf-8')



def main():
    parser = argparse.ArgumentParser(
        description='HoL music converter and importer')

    parser.add_argument('name', nargs='?',
        help='specific song name to import (default: re-import all songs)')

    parser.add_argument('--ost', action='store_true',
        help='generate a MIDI suitable for OST preparation, rather than actually importing')

    args = parser.parse_args()

    song_list = json.loads(Path('songs.json').read_text(encoding='utf-8'))

    # Show a nice error message if the song doesn't exist
    if args.name is not None:
        for entry in song_list:
            if entry['name'] == args.name:
                break
        else:
            raise ValueError(f'Unknown song "{args.name}"')

    # Read the bank IDs from the sequence table
    seq_to_bank = read_vanilla_sequence_bank_mapping()

    # Process each song
    used_bank_slots = set()
    for entry in song_list:
        name = entry['name']
        composer = entry['composer']

        if args.name is not None and name != args.name:
            continue

        seq_slot_name = entry.get('seq_slot_name')
        seq_slot_num = entry.get('seq_slot_num')
        if seq_slot_name is None and seq_slot_num is None:
            raise ValueError(f'"{name}": must have "seq_slot_name" and/or "seq_slot_num"')
        if seq_slot_name is not None:
            try:
                enum_member = lib.SeqId[seq_slot_name]
            except KeyError:
                raise ValueError(f'"{name}": unknown sequence slot name "{seq_slot_name}"')

            if seq_slot_num is not None:
                if enum_member.value != seq_slot_num:
                    raise ValueError(f'"{name}": sequence slot "{seq_slot_name}" has value {enum_member.value}, not {seq_slot_num}')
            else:
                seq_slot_num = enum_member.value
        else:
            seq_slot_name = lib.SeqId(seq_slot_num).name

        temp_dir = Path(f'{composer}/tmp')
        temp_dir.mkdir(exist_ok=True)

        print()
        print('=' * 96)
        print(f'Processing {name} (target music slot: {seq_slot_num} / {seq_slot_name})')

        # Get the bank slot and make sure there are no conflicts
        bank_slot = seq_to_bank[seq_slot_num]
        if bank_slot in used_bank_slots:
            raise ValueError(f'conflict for bank {bank_slot}')
        used_bank_slots.add(bank_slot)

        # Figure out various output paths
        preprocessed_midi_path = temp_dir / f'{name}_1.mid'
        preprocessed_midi_txt_path = temp_dir / f'{name}_1.mid.txt'
        s_path_1 = temp_dir / f'{name}_2.s'
        s_path_2 = temp_dir / f'{name}_3.s'
        bin_path = temp_dir / f'{name}_4.bin'
        final_bin_path = BUILD_FOLDER / 'audio' / 'sequences' / f'{seq_slot_num} - {name}.bin'
        bank_folder = BUILD_FOLDER / 'audio' / 'banks' / f'{bank_slot} - {name}'

        # ---- Sequence ----

        res = load_and_preprocess_midi(name, composer, is_for_ost=args.ost)

        if args.ost:
            res.midi.save(f'ost_{name}.mid')
            continue

        res.midi.save(preprocessed_midi_path)

        with preprocessed_midi_txt_path.open('w', encoding='utf-8') as f:
            lines = str(res.midi).splitlines()
            track_num = 1
            for i, line in enumerate(lines):
                if line.strip() == 'MidiTrack([':
                    lines[i] = f'{line}  # Track {track_num}'
                    track_num += 1
            f.write('\n'.join(lines))

        seq64_output_main = convert_with_seq64(preprocessed_midi_path, s_path_1, res.daw, smart_loop=res.smart_loop)

        if res.seqio_events:
            ppqn_from_to = seq64_output_main.converted_ppqn_from_to()
            if ppqn_from_to is None:
                raise ValueError("seq64 didn't print what PPQN values it used")
            ppqn_scale_factor = ppqn_from_to[1] / ppqn_from_to[0]
        else:
            ppqn_scale_factor = 1.0

        preprocess_s_file(
            s_path_1,
            s_path_2,
            res.seqio_events,
            ppqn_scale_factor,
            res.ingo_whistle_hack,
        )

        convert_with_seq64(s_path_2, bin_path, res.daw)

        # copy to output folder
        shutil.copy2(bin_path, final_bin_path)

        # ---- Bank ----

        # Get instrument list (except drums)
        inst_ids = set()
        for msg in res.midi:
            if msg.type == 'program_change':
                inst_ids.add(msg.program)
        inst_ids.discard(127)  # drums are handled separately

        # Find them
        bank_files = [get_instrument_tsv_by_id(id) for id in sorted(inst_ids)]

        # Create the actual folder
        shutil.rmtree(bank_folder, ignore_errors=True)
        bank_folder.mkdir()
        for file in bank_files:
            shutil.copy2(file, bank_folder / file.name)

        # Handle drums
        drums_id = res.drum_instrument_id or DEFAULT_DRUM_INSTRUMENT_ID
        drums_file_dst = bank_folder / '_drums.tsv'
        shutil.copy2(get_instrument_tsv_by_id(drums_id), drums_file_dst)


if __name__ == '__main__':
    main()
