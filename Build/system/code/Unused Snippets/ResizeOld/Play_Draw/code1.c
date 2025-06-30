#include "global.h"
#include "../../../../actor/_custom-1.0/draw2D.h"
#include "../../../../actor/_custom-1.0/holText.h"

void Play_Draw(PlayState* this);
void SetFramebuffer(PlayState* play, Gfx** gfxP, u16* buf);
void* Gfx_Alloc(Gfx** gfxP, u32 size);

u16* frameBuf = (u16*)0x80680000;
int* xPosOffs = (int*)0x80670000;
float* Scale = (float*)0x80670004;


#define G_IM_SIZ_16b_BYTES      2
#define G_IM_SIZ_16b_TILE_BYTES G_IM_SIZ_16b_BYTES
#define G_IM_SIZ_16b_LINE_BYTES G_IM_SIZ_16b_BYTES

void Play_Draw2(PlayState* play)
{
    Play_Draw(play);
    
    if (*xPosOffs < -240)
        *xPosOffs = 0;
    
    if (*Scale == 0)
        *Scale = 1.0f;
        
    if (CHECK_BTN_ALL(play->state.input->press.button, BTN_CUP))
        *Scale += 0.01;
    
    if (CHECK_BTN_ALL(play->state.input->press.button, BTN_CDOWN))
        *Scale -= 0.01;    
    
    if (CHECK_BTN_ALL(play->state.input->press.button, BTN_CRIGHT))
        *xPosOffs += 1;    

    if (CHECK_BTN_ALL(play->state.input->press.button, BTN_CLEFT))
        *xPosOffs -= 1;        
    
    if (*Scale > 1.0f)
        *Scale = 1.0f;
    
    if (*Scale < 0.5f)
        *Scale = 0.5f;
    
    if (*xPosOffs < -240)
        *xPosOffs = -240;
    
    if (*xPosOffs > 320)
        *xPosOffs = 320;

    if ((*Scale == 1.0f && *xPosOffs == 0))
        return;
    

    GraphicsContext* __gfxCtx = play->state.gfxCtx;

    Gfx* gfxRef = play->state.gfxCtx->overlay.p;
    Gfx* gfx = gfxRef;
    
    s32 rowsRemaining;
    s32 curRow;
    s32 nRows;
    
    gDPPipeSync(gfx++);
    gDPSetOtherMode(gfx++,
                    G_AD_DISABLE | G_CD_DISABLE | G_CK_NONE | G_TC_FILT | G_TF_POINT | G_TT_NONE | G_TL_TILE |
                        G_TD_CLAMP | G_TP_NONE | G_CYC_1CYCLE | G_PM_NPRIMITIVE,
                    G_AC_NONE | G_ZS_PRIM | G_RM_OPA_SURF | G_RM_OPA_SURF2);
    gDPSetEnvColor(gfx++, 255, 255, 255, 255);    
    gDPSetCombineLERP(gfx++, 0, 0, 0, TEXEL0, 0, 0, 0, 1, 0, 0, 0, TEXEL0, 0, 0, 0, 1);
    gDPSetCombineLERP(gfx++, TEXEL0, 0, ENVIRONMENT, 0, 0, 0, 0, 1, TEXEL0, 0, ENVIRONMENT, 0, 0, 0, 0, 1);
    gDPSetColorImage(gfx++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WIDTH, frameBuf);
    gDPSetScissor(gfx++, G_SC_NON_INTERLACE, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);   

    nRows = TMEM_SIZE / (SCREEN_WIDTH * G_IM_SIZ_16b_BYTES);
    
    rowsRemaining = SCREEN_HEIGHT;
    curRow = 0;
    while (rowsRemaining > 0) {
        s32 uls = 0;
        s32 lrs = SCREEN_WIDTH - 1;
        s32 ult;
        s32 lrt;

        if (nRows > rowsRemaining)
            nRows = rowsRemaining;

        ult = curRow;
        lrt = curRow + nRows - 1;

        gDPLoadTextureTile(gfx++, __gfxCtx->curFrameBuffer, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WIDTH, SCREEN_HEIGHT, uls, ult, lrs, lrt, 0,
                           G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                           G_TX_NOLOD);

        gSPTextureRectangle(gfx++, uls << 2, ult << 2, (lrs + 1) << 2, (lrt + 1) << 2, G_TX_RENDERTILE, uls << 5,
                            ult << 5, 1 << 10, 1 << 10);

        curRow += nRows;
        rowsRemaining -= nRows;
    }
    
    

    gDPPipeSync(gfx++);
    gDPSetColorImage(gfx++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WIDTH, __gfxCtx->curFrameBuffer);
    
    gDPPipeSync(gfx++);
    gDPSetCycleType(gfx++, G_CYC_FILL);
    gDPSetRenderMode(gfx++, G_RM_NOOP, G_RM_NOOP2);
    gDPSetFillColor(gfx++, (GPACK_RGBA5551(0, 0, 0, 1) << 16) | GPACK_RGBA5551(0, 0, 0, 1));
    gDPFillRectangle(gfx++, 0, 0, SCREEN_WIDTH - 1 , SCREEN_HEIGHT - 1);

/*
    uObjBg* bg;
    bg = Gfx_Alloc(&gfx, sizeof(uObjBg));


    int width = SCREEN_WIDTH;
    int height = SCREEN_HEIGHT;
    int x = (SCREEN_WIDTH - (SCREEN_WIDTH * (*Scale))) / 2;
    int y = (SCREEN_HEIGHT - (SCREEN_HEIGHT * (*Scale))) / 2;
    
    x += *xPosOffs;

    // Set up BG
    bg->b.imageX = 0;
    bg->b.imageW = width << 2;
    bg->b.frameX = x * 4;
    bg->b.frameW = width << 2;
    bg->b.imageY = 0;
    bg->b.imageH = height << 2;
    bg->b.frameY = y * 4;
    bg->b.frameH = height << 2;
    bg->b.imageLoad = G_BGLT_LOADTILE;
    bg->b.imageFmt = G_IM_FMT_RGBA;
    bg->b.imageSiz = G_IM_SIZ_16b;
    bg->b.imagePal = 0;
    bg->b.imageFlip = 0;
    bg->b.imagePtr = (void*)frameBuf;   
    bg->s.scaleW = (s32)((1 << 10) / (*Scale));
    bg->s.scaleH = (s32)((1 << 10) / (*Scale));
    bg->s.imageYorig = bg->b.imageY;    
    
    

    gSPLoadUcodeL(gfx++, gspS2DEX2d_fifo);
    
    gDPPipeSync(gfx++);
    gSPObjRenderMode(gfx++, G_OBJRM_ANTIALIAS | G_OBJRM_BILERP);
    gSPBgRect1Cyc(gfx++, bg);

    gSPLoadUcode(gfx++, SysUcode_GetUCode(), SysUcode_GetUCodeData());
*/
    
    Draw2DInternal(RGBA16_Setup39, (u8*)frameBuf, NULL, &gfx, SCREEN_WIDTH / 2 + *xPosOffs, SCREEN_HEIGHT / 2, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_WIDTH * (*Scale), SCREEN_HEIGHT * (*Scale), 255);


    play->state.gfxCtx->overlay.p = gfx;   
}
