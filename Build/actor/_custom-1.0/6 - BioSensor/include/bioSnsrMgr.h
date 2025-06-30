#ifndef _Z_BIOSENSOR_H_
#define _Z_BIOSENSOR_H_

#include <z64hdr/oot_u10/z64hdr.h>
#include "../../draw2D.h"
#include "../../holText.h"

#define BIOSENSOR_MEASURING 0xFFFF
#define INIT_OK 0
#define INIT_ERROR 1

#define BEAT_OFF 0
#define BEAT_DETECTED 1

#define BPS_SAMPLE_SIZE 20
#define BPS_SAMPLING_RATE 60 / R_UPDATE_RATE
#define BPS_MULTIPLIER 60 / BPS_SAMPLE_SIZE

#define SAVE_DEBUGMODE gSaveContext.isDoubleDefenseAcquired
#define OBJ_GRAPHICS_COMMON 3
#define HEARTS_OFFSET 0x6698
#define HEARTS_HEIGHT 16
#define HEARTS_WIDTH 16

typedef struct BioSnsrMgr
{
    Actor actor;
    u16 bpm;
    u16 posX;
    u16 posY;
    u8 invisible;
    
    u8 init;
    u16 curbps;
    u16 frameCount;
    u16 framesNoChange;
    u16 beats;
    u8 readptr;
    u8 beat;
    u8 beatLast;
    u16 bpsdata[BPS_SAMPLE_SIZE];
    u8 bpsPtr;
    s16 alpha;
    
    float curScale;

} BioSnsrMgr;

#endif
