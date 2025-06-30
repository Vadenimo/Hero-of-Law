/**
 * File: voiceinit.c
 *
 * Initializes Voice Recognition System control structure and hardware
 */
#include "../../include/voice.h"

#include "ultra64.h"
#include "../../include/PR/controller_voice.h"
#include "../../include/PR/os_voice.h"
#include "ultra64/controller.h"
#include "macros.h"
#include "../../../is64Printf.h"

OSTimer __osVRUTimer;
OSMesgQueue __osVRUTimerQ __attribute__((aligned(8)));
OSMesg __osVRUTimerMsg;

static u8 sCmds[] = {
    0x1E, 0x6E, 0x08, 0x56, 0x03,
};

bool IsVRU(OSMesgQueue* mq, OSVoiceHandle* hd, int channel)
{
    s32 errorCode;
    hd->__channel = channel;
    hd->__mq = mq;
    hd->__mode = VOICE_HANDLE_MODE_0;   
    
    u8 status = 0;
    errorCode = __osVoiceGetStatus(mq, channel, &status);
    if (errorCode != 0) {
        return errorCode;
    }

    u8 dataUS[2];
    errorCode = __osVoiceContRead2(mq, channel, 0, dataUS);   

    if (errorCode == 0) {
        
        if (dataUS[0] == 0x01)
            return true;
    }   
    
    return false; 
}

s32 osVoiceInit(OSMesgQueue* mq, OSVoiceHandle* hd, int channel) {
    s32 errorCode;
    s32 i;
    u8 status = 0;
    u8 data[4];
   

    hd->__channel = channel;
    hd->__mq = mq;
    hd->__mode = VOICE_HANDLE_MODE_0;

    errorCode = __osVoiceGetStatus(mq, channel, &status);
    if (errorCode != 0) 
        return errorCode;

    if (__osContChannelReset(mq, channel) != 0) {
        return CONT_ERR_CONTRFAIL;
    }
    
    // Required for VRU init
    osCreateMesgQueue(&__osVRUTimerQ, &__osVRUTimerMsg, 1);
    osSetTimer(&__osVRUTimer, 0x23C346, 0, &__osVRUTimerQ, &__osVRUTimerMsg);
    osRecvMesg(&__osVRUTimerQ, NULL, OS_MESG_BLOCK);       
    
    for (i = 0; i < ARRAY_COUNT(sCmds); i++) 
    {
        errorCode = __osVoiceSetADConverter(mq, channel, sCmds[i]);
        
        if (errorCode != 0)
            return errorCode;
    }
    
    errorCode = __osVoiceGetStatus(mq, channel, &status);
    if (errorCode != 0) 
        return errorCode;
    
    if (status & 2) 
        return CONT_ERR_VOICE_NO_RESPONSE;

    /**
     * data[0] = 0
     * data[1] = 0
     * data[2] = 1
     * data[3] = 0
     */
    *(u32*)data = 0x100;
    
    errorCode = __osVoiceContWrite4(mq, channel, 0, data);
    
    if (errorCode != 0) 
        return errorCode;

    errorCode = __osVoiceCheckResult(hd, &status);
    
    if (errorCode & 0xFF00) 
        errorCode = CONT_ERR_INVALID;

    return errorCode;
}
