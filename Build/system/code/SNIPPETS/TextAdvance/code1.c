#include "global.h"
#include "message_data_static.h"
#include "sfx.h"

#define gPreventTxtboxAdvanceAndSounds  msgCtx->textboxBackgroundForeColorIdx

u8 Message_ShouldAdvance(PlayState* play) 
{
    MessageContext* msgCtx = &play->msgCtx;
    Input* input = &play->state.input[0];

    if ((CHECK_BTN_ALL(input->press.button, BTN_A) || 
        (CHECK_BTN_ALL(input->press.button, BTN_CRIGHT) && play->msgCtx.textboxEndType != TEXTBOX_ENDTYPE_2_CHOICE && play->msgCtx.textboxEndType != TEXTBOX_ENDTYPE_3_CHOICE)) && !gPreventTxtboxAdvanceAndSounds)
    {
        Audio_PlaySfxGeneral(NA_SE_SY_MESSAGE_PASS, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
        return true;
    }
    else
        return false;
}