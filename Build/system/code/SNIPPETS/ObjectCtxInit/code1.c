#include "global.h"
#include "message_data_static.h"
#include "sfx.h"
#include "../../../../actor/_custom-1.0/common.h"

extern void* THA_AllocTailAlign16(TwoHeadArena* tha, size_t size);
	asm("THA_AllocTailAlign16 = 0x800A01B8");

//800813D4
void Object_InitContext(PlayState* play, ObjectContext* objectCtx) 
{
    u32 spaceSize = 0x400000;

    objectCtx->num = objectCtx->unk_09 = 0;
    objectCtx->mainKeepIndex = objectCtx->subKeepIndex = 0;

    for (int i = 0; i < ARRAY_COUNT(objectCtx->status); i++) 
        objectCtx->status[i].id = OBJECT_INVALID;
    
    // The Wii U VC VESSEL emulator is unable to allocate anything across the two 4M banks
    if (*emuIdentifier == WIIU_CHECK_MAGIC)
        spaceSize = (u32)play->sceneSegment - 0x80400000;
    
    objectCtx->spaceStart = objectCtx->status[0].segment = THA_AllocTailAlign16(&play->state.tha, spaceSize);
    objectCtx->spaceEnd = (void*)((uintptr_t)objectCtx->spaceStart + spaceSize);

    objectCtx->mainKeepIndex = Object_Spawn(objectCtx, OBJECT_GAMEPLAY_KEEP);
    gSegments[4] = VIRTUAL_TO_PHYSICAL(objectCtx->status[objectCtx->mainKeepIndex].segment);
}