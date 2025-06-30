#include "global.h"
#include "message_data_static.h"
#include "sfx.h"

#define TEXTBOX_POS_Y_TOP 18
#define TEXTBOX_POS_Y_BOTTOM 168
#define TEXTBOX_POS_Y_MIDDLE 90

//800D67E8
void Message_GrowTextbox(MessageContext* msgCtx)
{
    u32* cutsceneBarSize = (u32*)0x800FE478;

    // Adjust textbox position to bottom/top of screen
    switch (msgCtx->textBoxPos)
    {
        case 0: R_TEXTBOX_Y = TEXTBOX_POS_Y_BOTTOM - *cutsceneBarSize;  break;
        case 1: R_TEXTBOX_Y = TEXTBOX_POS_Y_TOP + *cutsceneBarSize; break;
        case 2: R_TEXTBOX_Y = TEXTBOX_POS_Y_MIDDLE; break;
        case 3: R_TEXTBOX_Y = TEXTBOX_POS_Y_BOTTOM - *cutsceneBarSize;
    }
    
    R_TEXTBOX_WIDTH = 256;
    R_TEXTBOX_TEXWIDTH = 512;
    R_TEXTBOX_TEXHEIGHT = 512;
    R_TEXTBOX_HEIGHT = 64;

    R_TEXTBOX_X = (32 + R_TEXTBOX_WIDTH_TARGET) - (R_TEXTBOX_WIDTH / 2);
    R_TEXT_INIT_XPOS = 63;
    R_TEXT_CHOICE_XPOS = 46;

    // Continue arrow position
    R_TEXTBOX_END_YPOS = R_TEXTBOX_Y + 58;
    R_TEXTBOX_END_XPOS = R_TEXTBOX_X + R_TEXTBOX_WIDTH / 2 - 6;
    
    if (msgCtx->textBoxType == TEXTBOX_TYPE_WOODEN)
        msgCtx->textboxColorAlphaTarget = 255;
    
    msgCtx->textboxColorAlphaCurrent += msgCtx->textboxColorAlphaTarget / 4;
    msgCtx->stateTimer += 1;    
    
    if (msgCtx->stateTimer == 4) 
    {
        msgCtx->msgMode = MSGMODE_TEXT_STARTING;
        msgCtx->textboxColorAlphaCurrent = msgCtx->textboxColorAlphaTarget;
    }    
}