#include "global.h"
#include "message_data_static.h"
#include "sfx.h"

#define gPreventTxtboxAdvanceAndSounds msgCtx->textboxBackgroundForeColorIdx
u8* useSquareEnd = (u8*)0x80112E40;

//800d78f0
u8 Message_ShouldAdvanceSilent(PlayState* play) 
{   
    MessageContext* msgCtx = &play->msgCtx;
    Input* input = &play->state.input[0];
    
    bool isChoice = (play->msgCtx.textboxEndType == TEXTBOX_ENDTYPE_2_CHOICE || play->msgCtx.textboxEndType == TEXTBOX_ENDTYPE_3_CHOICE);

    if ((CHECK_BTN_ALL(input->press.button, BTN_A) || 
        (CHECK_BTN_ALL(input->press.button, BTN_CRIGHT) && !isChoice)) && !gPreventTxtboxAdvanceAndSounds)
    {
        if (msgCtx->textboxEndType != TEXTBOX_ENDTYPE_HAS_NEXT)
        {
            u16 sfx = (isChoice || *useSquareEnd ) ? NA_SE_SY_DECIDE : NA_SE_SY_MESSAGE_PASS;
            Audio_PlaySfxGeneral(sfx, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);  
        }
        
        return true;
    }
    else
        return false;
}