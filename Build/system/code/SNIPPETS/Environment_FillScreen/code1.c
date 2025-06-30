#include "global.h"

void Environment_FillScreen(GraphicsContext* __gfxCtx, u8 red, u8 green, u8 blue, u8 alpha, u8 drawFlags) 
{
    if (alpha != 0) 
    {
        if (drawFlags & FILL_SCREEN_OPA) 
        {
            POLY_OPA_DISP = Gfx_SetupDL_57(POLY_OPA_DISP);
            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, red, green, blue, alpha);
            gDPSetAlphaDither(POLY_OPA_DISP++, G_AD_DISABLE);
            gDPSetColorDither(POLY_OPA_DISP++, G_CD_DISABLE);
            gDPFillRectangle(POLY_OPA_DISP++, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        }

        if (drawFlags & FILL_SCREEN_XLU) 
        {
            POLY_XLU_DISP = Gfx_SetupDL_57(POLY_XLU_DISP);
            gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, red, green, blue, alpha);

            if ((u32)alpha == 255)
                gDPSetRenderMode(POLY_XLU_DISP++, G_RM_OPA_SURF, G_RM_OPA_SURF2);
            
            gDPSetAlphaDither(POLY_XLU_DISP++, G_AD_DISABLE);
            gDPSetColorDither(POLY_XLU_DISP++, G_CD_DISABLE);
            gDPFillRectangle(POLY_XLU_DISP++, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        }
    }
}