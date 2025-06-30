#include "global.h"
#include "message_data_static.h"
#include "sfx.h"

extern u8 Message_ShouldAdvanceSilent2(PlayState* play);
        asm("Message_ShouldAdvanceSilent2 = 0x800d78f0");       

u8 Message_ShouldAdvanceSilent(PlayState* play) 
{
    return Message_ShouldAdvanceSilent2(play);
}