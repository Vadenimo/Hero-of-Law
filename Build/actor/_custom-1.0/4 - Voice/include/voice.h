#ifndef _Z_VOICE_H_
#define _Z_VOICE_H_

#include <z64hdr/oot_u10/z64hdr.h>
#include "PR/controller_voice.h"
#include "z64voice.h"

#define PIF_ROM_START   0x1FC00000
#define PIF_ROM_END     0x1FC007BF
#define PIF_RAM_START   0x1FC007C0
#define PIF_RAM_END     0x1FC007FF

#define IO_READ(addr)       (*(vu32*)PHYS_TO_K1(addr))
#define IO_WRITE(addr,data) (*(vu32*)PHYS_TO_K1(addr)=(u32)(data))

typedef enum {
    /*  0 */ PADMGR_CONT_NONE,
    /*  1 */ PADMGR_CONT_NORMAL,
    /*  3 */ PADMGR_CONT_MOUSE = 3,
    /*  4 */ PADMGR_CONT_VOICE_PLUGGED, // VRU plugged but not initialized
    /*  5 */ PADMGR_CONT_VOICE,
    /* -1 */ PADMGR_CONT_UNK = 0xFF
} ControllerDeviceType;
    
extern OSPifRam __osContPifRam;
extern u8 __osContLastPoll;
extern u8 __osMaxControllers;
extern OSMesg __osEepromTimerMsg[];
extern OSPifRam __osPfsPifRam;
extern s32 __osPfsLastChannel;
s32 __osContChannelReset(OSMesgQueue* mq, s32 channel);
char* func_801A5A1C(s8* words);

typedef struct {
    /* 0x00 */ u8 txsize;
    /* 0x01 */ u8 rxsize;
    /* 0x02 */ u8 poll;
    /* 0x03 */ u8 typeh;
    /* 0x04 */ u8 typel;
    /* 0x05 */ u8 status;
} __OSContRequestHeaderAligned; // size = 0x6

typedef struct {
    /* 0x00 */ OSVoiceDictionary* dict;
    /* 0x04 */ s8 mode;
    /* 0x08 */ OSVoiceData* data;
    /* 0x0C */ u16 distance;
    /* 0x0E */ u16 answerNum;
    /* 0x10 */ u16 warning;
    /* 0x12 */ u16 voiceLevel;
    /* 0x14 */ u16 voiceRelLevel;
} OSVoiceContext; // size = 0x18

#endif
