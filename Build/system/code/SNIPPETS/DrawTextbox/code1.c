#include "global.h"
#include "../../../../actor/_custom-1.0/common.h"

typedef enum TextBoxType {
    /*  0 */ TEXTBOX_TYPE_BLACK,
    /*  1 */ TEXTBOX_TYPE_WOODEN,
    /*  2 */ TEXTBOX_TYPE_BLUE,
    /*  3 */ TEXTBOX_TYPE_OCARINA,
    /*  4 */ TEXTBOX_TYPE_NONE_BOTTOM,
    /*  5 */ TEXTBOX_TYPE_NONE_NO_SHADOW,
    /* 11 */ TEXTBOX_TYPE_CREDITS = 11
} TextBoxType;

int ugh = 0;
int ugh2 = 0;

void Message_DrawTextBox(PlayState* play, Gfx** p) 
{
    MessageContext* msgCtx = &play->msgCtx;
    Gfx* gfx = *p;

    gDPPipeSync(gfx++);
    gDPSetPrimColor(gfx++, 0, 0, msgCtx->textboxColorRed, msgCtx->textboxColorGreen, msgCtx->textboxColorBlue,
                    msgCtx->textboxColorAlphaCurrent);
                    
    int textboxTexWidth = R_TEXTBOX_TEXWIDTH;
    int textboxWidth = R_TEXTBOX_WIDTH;
    int textboxPosX = R_TEXTBOX_X;
    
    if (SAVE_WIDESCREEN)
    {
        textboxTexWidth = textboxTexWidth / 65 * 100;
        textboxWidth = textboxWidth * 73 / 100;
        textboxPosX += WIDESCREEN_TXBOX_OFFSX;        
    }
        

    if (!(msgCtx->textBoxType) || msgCtx->textBoxType == TEXTBOX_TYPE_BLUE)
    {
        gDPLoadTextureBlock_4b(gfx++, msgCtx->textboxSegment, G_IM_FMT_I, 128, 64, 0, G_TX_MIRROR, G_TX_NOMIRROR, 7, 0,
                               G_TX_NOLOD, G_TX_NOLOD);
    } 
    else 
    {
        gDPSetEnvColor(gfx++, 50, 20, 0, 255);
        gDPLoadTextureBlock_4b(gfx++, msgCtx->textboxSegment, G_IM_FMT_IA, 128, 64, 0, G_TX_MIRROR, G_TX_MIRROR, 7, 0,
                               G_TX_NOLOD, G_TX_NOLOD);
    }

    gSPTextureRectangle(gfx++, textboxPosX << 2, R_TEXTBOX_Y << 2, (textboxPosX + textboxWidth) << 2,
                        (R_TEXTBOX_Y + R_TEXTBOX_HEIGHT) << 2, G_TX_RENDERTILE, 0, 0, textboxTexWidth << 1,
                        R_TEXTBOX_TEXHEIGHT << 1);

    *p = gfx;
}

