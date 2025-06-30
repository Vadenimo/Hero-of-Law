#include "global.h"
#include "code1.h"

void Audio_ProcessSeqCmd(u32 cmd) 
{
    s32 priority;
    s32 channelMaskEnable;
    u16 channelMaskDisable;
    u16 fadeTimer;
    u16 val;
    u8 oldSpecId;
    u8 specId;
    u8 op;
    u8 subOp;
    u8 seqPlayerIndex;
    u8 seqId;
    u8 seqArgs;
    u8 found;
    u8 ioPort;
    u8 duration;
    u8 channelIndex;
    u8 i;
    f32 freqScaleTarget;
    s32 pad;

    op = cmd >> 28;
    seqPlayerIndex = (cmd & 0xF000000) >> 24;

    switch (op) {
        case SEQCMD_OP_PLAY_SEQUENCE:
            // Play a new sequence
            seqId = cmd & 0xFF;
            seqArgs = (cmd & 0xFF00) >> 8;
            // `fadeTimer` is only shifted 13 bits instead of 16 bits.
            // `fadeTimer` continues to be scaled in `Audio_StartSequence`
            fadeTimer = (cmd & 0xFF0000) >> 13;
            if (!gActiveSeqs[seqPlayerIndex].isWaitingForFonts && (seqArgs < 0x80)) {
                Audio_StartSequence(seqPlayerIndex, seqId, seqArgs, fadeTimer);
            }
            break;

        case SEQCMD_OP_STOP_SEQUENCE:
            // Stop a sequence and disable the sequence player
            fadeTimer = (cmd & 0xFF0000) >> 13;
            Audio_StopSequence(seqPlayerIndex, fadeTimer);
            break;

        case SEQCMD_OP_QUEUE_SEQUENCE:
            // Queue a sequence into `sSeqRequests`
            seqId = cmd & 0xFF;
            seqArgs = (cmd & 0xFF00) >> 8;
            fadeTimer = (cmd & 0xFF0000) >> 13;
            priority = seqArgs;

            // Checks if the requested sequence is first in the list of requests
            // If it is already queued and first in the list, then play the sequence immediately
            for (i = 0; i < sNumSeqRequests[seqPlayerIndex]; i++) {
                if (sSeqRequests[seqPlayerIndex][i].seqId == seqId) {
                    if (i == 0) {
                        Audio_StartSequence(seqPlayerIndex, seqId, seqArgs, fadeTimer);
                    }
                    return;
                }
            }

            // Searches the sequence requests for the first request that does not have a higher priority
            // than the current incoming request
            found = sNumSeqRequests[seqPlayerIndex];
            for (i = 0; i < sNumSeqRequests[seqPlayerIndex]; i++) {
                if (priority >= sSeqRequests[seqPlayerIndex][i].priority) {
                    found = i;
                    i = sNumSeqRequests[seqPlayerIndex]; // "break;"
                }
            }

            // Check if the queue is full
            if (sNumSeqRequests[seqPlayerIndex] < ARRAY_COUNT(sSeqRequests[seqPlayerIndex])) {
                sNumSeqRequests[seqPlayerIndex]++;
            }

            for (i = sNumSeqRequests[seqPlayerIndex] - 1; i != found; i--) {
                // Move all requests of lower priority backwards 1 place in the queue
                // If the queue is full, overwrite the entry with the lowest priority
                sSeqRequests[seqPlayerIndex][i].priority = sSeqRequests[seqPlayerIndex][i - 1].priority;
                sSeqRequests[seqPlayerIndex][i].seqId = sSeqRequests[seqPlayerIndex][i - 1].seqId;
            }

            // Fill the newly freed space in the queue with the new request
            sSeqRequests[seqPlayerIndex][found].priority = seqArgs;
            sSeqRequests[seqPlayerIndex][found].seqId = seqId;

            // The sequence is first in queue, so start playing.
            if (found == 0) {
                Audio_StartSequence(seqPlayerIndex, seqId, seqArgs, fadeTimer);
            }
            break;

        case SEQCMD_OP_UNQUEUE_SEQUENCE:
            // Unqueue sequence
            fadeTimer = (cmd & 0xFF0000) >> 13;

            found = sNumSeqRequests[seqPlayerIndex];
            for (i = 0; i < sNumSeqRequests[seqPlayerIndex]; i++) {
                seqId = cmd & 0xFF;
                if (sSeqRequests[seqPlayerIndex][i].seqId == seqId) {
                    found = i;
                    i = sNumSeqRequests[seqPlayerIndex]; // "break;"
                }
            }

            if (found != sNumSeqRequests[seqPlayerIndex]) {
                // Move all requests of lower priority forward 1 place in the queue
                for (i = found; i < sNumSeqRequests[seqPlayerIndex] - 1; i++) {
                    sSeqRequests[seqPlayerIndex][i].priority = sSeqRequests[seqPlayerIndex][i + 1].priority;
                    sSeqRequests[seqPlayerIndex][i].seqId = sSeqRequests[seqPlayerIndex][i + 1].seqId;
                }
                sNumSeqRequests[seqPlayerIndex]--;
            }

            // If the sequence was first in queue (it is currently playing),
            // Then stop the sequence and play the next sequence in the queue.
            if (found == 0) {
                Audio_StopSequence(seqPlayerIndex, fadeTimer);
                if (sNumSeqRequests[seqPlayerIndex] != 0) {
                    Audio_StartSequence(seqPlayerIndex, sSeqRequests[seqPlayerIndex][0].seqId,
                                        sSeqRequests[seqPlayerIndex][0].priority, fadeTimer);
                }
            }
            break;

        case SEQCMD_OP_SET_SEQPLAYER_VOLUME:
            // Transition volume to a target volume for an entire player
            duration = (cmd & 0xFF0000) >> 15;
            val = cmd & 0xFF;
            if (duration == 0) {
                duration++;
            }
            // Volume is scaled relative to 127
            gActiveSeqs[seqPlayerIndex].volTarget = (f32)val / 127.0f;
            if (gActiveSeqs[seqPlayerIndex].volCur != gActiveSeqs[seqPlayerIndex].volTarget) {
                gActiveSeqs[seqPlayerIndex].volStep =
                    (gActiveSeqs[seqPlayerIndex].volCur - gActiveSeqs[seqPlayerIndex].volTarget) / (f32)duration;
                gActiveSeqs[seqPlayerIndex].volTimer = duration;
            }
            break;

        case SEQCMD_OP_SET_SEQPLAYER_FREQ:
            // Transition freq scale to a target freq for all channels
            duration = (cmd & 0xFF0000) >> 15;
            val = cmd & 0xFFFF;
            if (duration == 0) {
                duration++;
            }
            // Frequency is scaled relative to 1000
            freqScaleTarget = (f32)val / 1000.0f;
            for (i = 0; i < SEQ_NUM_CHANNELS; i++) {
                gActiveSeqs[seqPlayerIndex].channelData[i].freqScaleTarget = freqScaleTarget;
                gActiveSeqs[seqPlayerIndex].channelData[i].freqScaleTimer = duration;
                gActiveSeqs[seqPlayerIndex].channelData[i].freqScaleStep =
                    (gActiveSeqs[seqPlayerIndex].channelData[i].freqScaleCur - freqScaleTarget) / (f32)duration;
            }
            gActiveSeqs[seqPlayerIndex].freqScaleChannelFlags = 0xFFFF;
            break;

        case SEQCMD_OP_SET_CHANNEL_FREQ:
            // Transition freq scale to a target for a specific channel
            duration = (cmd & 0xFF0000) >> 15;
            channelIndex = (cmd & 0xF000) >> 12;
            val = cmd & 0xFFF;
            if (duration == 0) {
                duration++;
            }
            // Frequency is scaled relative to 1000
            freqScaleTarget = (f32)val / 1000.0f;
            gActiveSeqs[seqPlayerIndex].channelData[channelIndex].freqScaleTarget = freqScaleTarget;
            gActiveSeqs[seqPlayerIndex].channelData[channelIndex].freqScaleStep =
                (gActiveSeqs[seqPlayerIndex].channelData[channelIndex].freqScaleCur - freqScaleTarget) / (f32)duration;
            gActiveSeqs[seqPlayerIndex].channelData[channelIndex].freqScaleTimer = duration;
            gActiveSeqs[seqPlayerIndex].freqScaleChannelFlags |= 1 << channelIndex;
            break;

        case SEQCMD_OP_SET_CHANNEL_VOLUME:
            // Transition volume to a target volume for a specific channel
            duration = (cmd & 0xFF0000) >> 15;
            channelIndex = (cmd & 0xF00) >> 8;
            val = cmd & 0xFF;

            // Volume is scaled relative to 127
            f32 valF = (f32)val / 127.0f;
            gActiveSeqs[seqPlayerIndex].channelData[channelIndex].volTarget = valF;

            if (duration == 0) {
                gActiveSeqs[seqPlayerIndex].channelData[channelIndex].volCur = valF;
                gActiveSeqs[seqPlayerIndex].channelData[channelIndex].volStep = 0.0f;
                gActiveSeqs[seqPlayerIndex].channelData[channelIndex].volTimer = 0;
                gActiveSeqs[seqPlayerIndex].volChannelFlags &= ~(1 << channelIndex);
                AUDIOCMD_CHANNEL_SET_VOL_SCALE(seqPlayerIndex, (u32)channelIndex, valF);
            } else if (gActiveSeqs[seqPlayerIndex].channelData[channelIndex].volCur !=
                gActiveSeqs[seqPlayerIndex].channelData[channelIndex].volTarget) {
                gActiveSeqs[seqPlayerIndex].channelData[channelIndex].volStep =
                    (gActiveSeqs[seqPlayerIndex].channelData[channelIndex].volCur -
                     gActiveSeqs[seqPlayerIndex].channelData[channelIndex].volTarget) /
                    (f32)duration;
                gActiveSeqs[seqPlayerIndex].channelData[channelIndex].volTimer = duration;
                gActiveSeqs[seqPlayerIndex].volChannelFlags |= 1 << channelIndex;
            }
            break;

        case SEQCMD_OP_SET_SEQPLAYER_IO:
            // Set global io port
            ioPort = (cmd & 0xFF0000) >> 16;
            val = cmd & 0xFF;
            AUDIOCMD_SEQPLAYER_SET_IO(seqPlayerIndex, ioPort, val);
            break;

        case SEQCMD_OP_SET_CHANNEL_IO:
            // Set io port if channel masked
            channelIndex = (cmd & 0xF00) >> 8;
            ioPort = (cmd & 0xFF0000) >> 16;
            val = cmd & 0xFF;
            if (!(gActiveSeqs[seqPlayerIndex].channelPortMask & (1 << channelIndex))) {
                AUDIOCMD_CHANNEL_SET_IO(seqPlayerIndex, (u32)channelIndex, ioPort, val);
            }
            break;

        case SEQCMD_OP_SET_CHANNEL_IO_DISABLE_MASK:
            // Disable channel io specifically for `SEQCMD_OP_SET_CHANNEL_IO`.
            // This can be bypassed by setting channel io through using `AUDIOCMD_CHANNEL_SET_IO` directly.
            // This is accomplished by setting a channel mask.
            gActiveSeqs[seqPlayerIndex].channelPortMask = cmd & 0xFFFF;
            break;

        case SEQCMD_OP_SET_CHANNEL_DISABLE_MASK:
            // Disable or Reenable channels

            // Disable channels
            channelMaskDisable = cmd & 0xFFFF;
            if (channelMaskDisable != 0) {
                // Apply channel mask `channelMaskDisable`
                AUDIOCMD_GLOBAL_SET_CHANNEL_MASK(seqPlayerIndex, channelMaskDisable);
                // Disable channels
                AUDIOCMD_CHANNEL_SET_MUTE(seqPlayerIndex, AUDIOCMD_ALL_CHANNELS, true);
            }

            // Reenable channels
            channelMaskEnable = (channelMaskDisable ^ 0xFFFF);
            if (channelMaskEnable != 0) {
                // Apply channel mask `channelMaskEnable`
                AUDIOCMD_GLOBAL_SET_CHANNEL_MASK(seqPlayerIndex, channelMaskEnable);
                // Enable channels
                AUDIOCMD_CHANNEL_SET_MUTE(seqPlayerIndex, AUDIOCMD_ALL_CHANNELS, false);
            }
            break;

        case SEQCMD_OP_TEMPO_CMD:
            // Update a tempo using a sub-command system.
            // Stores the cmd for processing elsewhere.
            gActiveSeqs[seqPlayerIndex].tempoCmd = cmd;
            break;

        case SEQCMD_OP_SETUP_CMD:
            // Queue a sub-command to execute once the sequence is finished playing
            subOp = (cmd & 0xF00000) >> 20;
            if (subOp != SEQCMD_SUB_OP_SETUP_RESET_SETUP_CMDS) {
                // Ensure the maximum number of setup commands is not exceeded
                if (gActiveSeqs[seqPlayerIndex].setupCmdNum < (ARRAY_COUNT(gActiveSeqs[seqPlayerIndex].setupCmd) - 1)) {
                    found = gActiveSeqs[seqPlayerIndex].setupCmdNum++;
                    if (found < ARRAY_COUNT(gActiveSeqs[seqPlayerIndex].setupCmd)) {
                        gActiveSeqs[seqPlayerIndex].setupCmd[found] = cmd;
                        // Adds a delay of 2 frames before executing any setup commands.
                        // This allows setup commands to be requested along with a new sequence on a seqPlayerIndex.
                        // This 2 frame delay ensures the player is enabled before its state is checked for
                        // the purpose of deciding if the setup commands should be run.
                        // Otherwise, the setup commands will be executed before the sequence starts,
                        // when the player is still disabled, instead of when the newly played sequence ends.
                        gActiveSeqs[seqPlayerIndex].setupCmdTimer = 2;
                    }
                }
            } else {
                // `SEQCMD_SUB_OP_SETUP_RESET_SETUP_CMDS`
                // Discard all setup command requests on `seqPlayerIndex`
                gActiveSeqs[seqPlayerIndex].setupCmdNum = 0;
            }
            break;

        case SEQCMD_OP_GLOBAL_CMD:
            // Apply a command that applies to all sequence players
            subOp = (cmd & 0xF00) >> 8;
            val = cmd & 0xFF;
            switch (subOp) {
                case SEQCMD_SUB_OP_GLOBAL_SET_SOUND_MODE:
                    // Set sound mode
                    AUDIOCMD_GLOBAL_SET_SOUND_MODE(gSoundModeList[val]);
                    break;

                case SEQCMD_SUB_OP_GLOBAL_DISABLE_NEW_SEQUENCES:
                    // Disable the starting of new sequences (except for the sfx player)
                    gStartSeqDisabled = val & 1;
                    break;
            }
            break;

        case SEQCMD_OP_RESET_AUDIO_HEAP:
            // Resets the audio heap based on the audio specifications and sfx channel layout
            specId = cmd & 0xFF;
            gSfxChannelLayout = (cmd & 0xFF00) >> 8;
            oldSpecId = gAudioSpecId;
            gAudioSpecId = specId;
            AudioThread_ResetAudioHeap(specId);
            func_800F71BC(oldSpecId);
            AUDIOCMD_GLOBAL_STOP_AUDIOCMDS();
            break;
    }
}