#include "global.h"
#include "../../../../actor/_custom-1.0/draw2D.h"
#include "../../../../actor/_custom-1.0/holText.h"

void* Gfx_Alloc(Gfx** gfxP, u32 size);
extern Input* D_8012D1F8;
void Play_Update(PlayState* this);
void Play_Draw(PlayState* this);
void DebugDisplay_Init(void);
extern uintptr_t sSysCfbEnd;

#define SAVE_SCREENXPOS gSaveContext.scarecrowLongSong[20]
#define SAVE_SCREENYPOS gSaveContext.scarecrowLongSong[21]
#define SAVE_SCREENSIZE gSaveContext.scarecrowLongSong[22]
#define SAVE_SCREENADJUSTING gSaveContext.scarecrowLongSong[23]

#ifndef MAX
    #define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
    #define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

void Play_Main2(PlayState* play)
{
    D_8012D1F8 = &play->state.input[0];

    DebugDisplay_Init();
    Play_Update(play);
    Play_Draw(play);
   
    // Reset to defaults if L + R + START is pressed.
    if (CHECK_BTN_ALL(play->state.input->cur.button, BTN_L | BTN_R | BTN_START))
    {
        SAVE_SCREENSIZE = 255;
        SAVE_SCREENXPOS = 0;
        SAVE_SCREENYPOS = 0;
    }
    
    // Failsafe
    if (SAVE_SCREENSIZE == 0)
        SAVE_SCREENSIZE = 255;
   
    float Scale = ((float)SAVE_SCREENSIZE / (float)255);
    int xPos = (s8)SAVE_SCREENXPOS;
    int yPos = (s8)SAVE_SCREENYPOS;
    
    // Failsafe
    if (xPos < -127 || xPos > 127 || yPos < -127 || yPos > 127)
    {
        xPos = 0;
        yPos = 0;
        SAVE_SCREENXPOS = 0;
        SAVE_SCREENYPOS = 0;
    }
    
    // If screen defaults are set, we're done.
    if (Scale == 1.0f && xPos == 0 && yPos == 0 && SAVE_SCREENADJUSTING == 0)
        return;

    GraphicsContext* __gfxCtx = play->state.gfxCtx;
    Gfx* gfxRef = play->state.gfxCtx->overlay.p;
    Gfx* gfx = gfxRef;
    
    // Image is located at the very end of RAM. 
    u32 screenSizeBytes = 2 * SCREEN_WIDTH * SCREEN_HEIGHT;
    uintptr_t frameBuf = sSysCfbEnd - screenSizeBytes;    
    
    int width = SCREEN_WIDTH;
    int height = SCREEN_HEIGHT;
    float x = 0;
    float y = 0;
    
    gSPLoadUcodeL(gfx++, gspS2DEX2d_fifo);

    // COPY FRAMEBUFFER  ->  IMAGE

    gDPPipeSync(gfx++);
    gDPSetOtherMode(gfx++,
                    G_AD_PATTERN | G_CD_MAGICSQ | G_CK_NONE | G_TC_CONV | G_TF_POINT | G_TT_NONE | G_TL_TILE |
                        G_TD_CLAMP | G_TP_NONE | G_CYC_COPY | G_PM_NPRIMITIVE,
                    G_AC_NONE | G_ZS_PIXEL | G_RM_NOOP | G_RM_NOOP2);
                      
    uObjBg* bg;
    bg = Gfx_Alloc(&gfx, sizeof(uObjBg));
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
    bg->b.imagePtr = (void*)__gfxCtx->curFrameBuffer;   
    guS2DInitBg(bg);
    
    gDPPipeSync(gfx++);
    gDPSetColorImage(gfx++, G_IM_FMT_RGBA, G_IM_SIZ_16b, width, (u16*)frameBuf);
    gDPSetScissor(gfx++, 0, 0, 0, width, height);
    gSPObjRenderMode(gfx++, G_OBJRM_ANTIALIAS | G_OBJRM_BILERP);
    gSPBgRectCopy(gfx++, bg);
    gDPPipeSync(gfx++);
    
    // ERASE FRAMEBUFFER, SET DRAW PARAMS

    gDPSetColorImage(gfx++, G_IM_FMT_RGBA, G_IM_SIZ_16b, width, (u16*)__gfxCtx->curFrameBuffer);
    gDPSetScissor(gfx++, 0, 0, 0, width, height);
    
    gDPPipeSync(gfx++);
    gDPSetOtherMode(gfx++,
                    G_AD_PATTERN | G_CD_MAGICSQ | G_CK_NONE | G_TC_CONV | G_TF_POINT | G_TT_NONE | G_TL_TILE |
                        G_TD_CLAMP | G_TP_NONE | G_CYC_FILL | G_PM_NPRIMITIVE,
                    G_AC_NONE | G_ZS_PIXEL | G_RM_NOOP | G_RM_NOOP2);
    gDPSetFillColor(gfx++, (GPACK_RGBA5551(0, 0, 0, 1) << 16) | GPACK_RGBA5551(0, 0, 0, 1));
    gDPFillRectangle(gfx++, 0, 0, width, height); 
    gDPPipeSync(gfx++);

    gsDPSetPrimColor(0, 0, 0, 0, 0, 0);
    gDPSetOtherMode(gfx++,
                    G_AD_DISABLE | G_CD_DISABLE | G_CK_NONE | G_TC_FILT | G_TF_BILERP | G_TT_NONE | G_TL_TILE |
                        G_TD_CLAMP | G_TP_NONE | G_CYC_1CYCLE | G_PM_NPRIMITIVE,
                    G_AC_THRESHOLD | G_ZS_PIXEL | AA_EN | CVG_DST_CLAMP | ZMODE_OPA | CVG_X_ALPHA | ALPHA_CVG_SEL |
                        GBL_c1(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_BL, G_BL_1MA) |
                        GBL_c2(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_BL, G_BL_1MA));
    gDPSetCombineLERP(gfx++, PRIMITIVE, TEXEL0, PRIM_LOD_FRAC, TEXEL0, 0, 0, 0, 1, PRIMITIVE, TEXEL0, PRIM_LOD_FRAC,
                      TEXEL0, 0, 0, 0, 1);  
                      
                      
    // DRAW IMAGE TO FRAMEBUFFER                  
                      
    x = (SCREEN_WIDTH - (SCREEN_WIDTH * Scale)) / 2;
    y = (SCREEN_HEIGHT - (SCREEN_HEIGHT * Scale)) / 2;
    x += xPos - 0.15;
    y += yPos - 0.15;

    uObjBg* bg2;
    bg2 = Gfx_Alloc(&gfx, sizeof(uObjBg));
    bg2->b.imageX = 0;
    bg2->b.imageW = width << 2;
    bg2->b.frameX = x * 4;
    bg2->b.frameW = width << 2;
    bg2->b.imageY = 0;
    bg2->b.imageH = height << 2;
    bg2->b.frameY = y * 4;
    bg2->b.frameH = height << 2;
    bg2->b.imageLoad = G_BGLT_LOADTILE;
    bg2->b.imageFmt = G_IM_FMT_RGBA;
    bg2->b.imageSiz = G_IM_SIZ_16b;
    bg2->b.imagePal = 0;
    bg2->b.imageFlip = 0;
    bg2->b.imagePtr = (void*)frameBuf;   
    bg2->s.scaleW = (s32)((1 << 10) / Scale);
    bg2->s.scaleH = (s32)((1 << 10) / Scale);
    bg2->s.imageYorig = bg2->b.imageY;      
    
    gDPPipeSync(gfx++);
    gDPSetColorImage(gfx++, G_IM_FMT_RGBA, G_IM_SIZ_16b, width, (u16*)__gfxCtx->curFrameBuffer);
    gDPSetScissor(gfx++, 0, 0, 0, width, height);
    gSPObjRenderMode(gfx++, G_OBJRM_ANTIALIAS | G_OBJRM_BILERP);
    gSPBgRect1Cyc(gfx++, bg2);
    gDPPipeSync(gfx++); 
    
    gSPLoadUcode(gfx++, SysUcode_GetUCode(), SysUcode_GetUCodeData());

    play->state.gfxCtx->overlay.p = gfx;   
}