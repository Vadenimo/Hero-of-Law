#include "global.h"
#include "message_data_static.h"
#include "sfx.h"
#include "../../../../actor/_custom-1.0/draw2D.h"

#define CHARSIZEX 16
#define CHARSIZEY 16

//800D7B00
void DrawCharTexture(PlayState* play, Gfx** gfxp, u8* texture, s32 x, s32 y, float scale, bool loadGfx, s16 alpha, 
                     Color_RGB8 Color, Color_RGB8 ShadowColor, bool drawShadow, s16 shadowAlpha, u8 shadowOffsetX, u8 shadowOffsetY)
{
    Gfx* gfx = *gfxp;

    s32 sCharTexScale = 1024.0f / (scale / 100.0f);
    s32 sCharTexSize = (scale / 100.0f) * 16.0f;

    if (loadGfx)
    {
        gDPLoadTextureBlock_4b(gfx++, texture, G_IM_FMT_I, CHARSIZEX, CHARSIZEY, 0, G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMIRROR | G_TX_CLAMP,
                            G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
    }
    
    if (drawShadow)
    {
        gDPPipeSync(gfx++);
        gDPSetPrimColor(gfx++, 0, 0, ShadowColor.r, ShadowColor.g, ShadowColor.b, shadowAlpha);
        gSPScisTextureRectangle (gfx++, (x + shadowOffsetX) << 2, ((y + shadowOffsetY) << 2), ((x + shadowOffsetX) + sCharTexSize) << 2, (((y + shadowOffsetY) + sCharTexSize) << 2), G_TX_RENDERTILE, 0, 0, sCharTexScale, sCharTexScale);
    }
    
    gDPPipeSync(gfx++);
    gDPSetPrimColor(gfx++, 0, 0, Color.r, Color.g, Color.b, alpha);    
    gSPScisTextureRectangle (gfx++, x << 2, (y << 2), (x + sCharTexSize) << 2, ((y + sCharTexSize) << 2), G_TX_RENDERTILE, 0, 0, sCharTexScale, sCharTexScale);

    *gfxp = gfx;
}