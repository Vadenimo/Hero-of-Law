#include "global.h"
#include "../../../../actor/_custom-1.0/common.h"

extern uintptr_t sSysCfbFbPtr[2];
extern uintptr_t sSysCfbEnd;
u32* memSize = (u32*)0x80000318;

void SysCfb_Init(s32 n64dd) 
{
    u32 screenSize = SCREEN_WIDTH * SCREEN_HEIGHT;
    
    if (*memSize >= 0x800000) 
        sSysCfbEnd = 0x80800000 - AudioHeapSize;
    else
        sSysCfbEnd = 0x80400000 - AudioHeapSize;
    
    sSysCfbEnd &= ~0x3F; 
    
    sSysCfbFbPtr[0] = sSysCfbEnd - (screenSize * 4);
    sSysCfbFbPtr[1] = sSysCfbEnd - (screenSize * 2);
}