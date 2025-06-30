#include "global.h"
#include "message_data_static.h"
#include "sfx.h"

#define objectTable (*(RomFile(*)[]) 0x800F8FF8)

//8005BD78
void Font_LoadFont(Font* font)
{
    RomFile* fnt = &objectTable[9];
    DmaMgr_SendRequest1(&font->fontBuf, fnt->vromStart, fnt->vromEnd - fnt->vromStart);
}