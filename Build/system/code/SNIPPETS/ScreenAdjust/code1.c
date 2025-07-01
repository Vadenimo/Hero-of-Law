#include "global.h"
#include "../../../../actor/_custom-1.0/common.h"
#include "../../../../actor/_custom-1.0/draw2D.h"
#include "../../../../actor/_custom-1.0/holText.h"
#include "../../../../actor/_custom-1.0/is64Printf.h"

extern ViMode sViMode;
extern PreNmiBuff* gAppNmiBufferPtr;

void Minimap_Draw(PlayState* play)
{
}

void Screen_Adjust(GameState* state, View* view)
{
    if (SAVE_WIDESCREEN)
    {   
        float aspect = 16.0f/9.0f;
        guPerspective(view->projectionPtr, &view->normal, view->fovy, aspect, view->zNear, view->zFar, view->scale);     
    }    

    bool preNMIDone = false;
    
    // Reset Animation
    if (gAppNmiBufferPtr->resetting)
    {
        Gfx* gfxRef = state->gfxCtx->overlay.p;
        Gfx* gfx = gfxRef;
        
        gAppNmiBufferPtr->resetCount++;
        
        if (gAppNmiBufferPtr->resetCount >= 8)
            preNMIDone = true;       
        
        gDPPipeSync(gfx++);
        gDPSetCycleType(gfx++, G_CYC_FILL);
        gDPSetRenderMode(gfx++, G_RM_NOOP, G_RM_NOOP2);
        gDPSetFillColor(gfx++, (GPACK_RGBA5551(0, 0, 0, 1) << 16) | GPACK_RGBA5551(0, 0, 0, 1));
       
        gDPFillRectangle(gfx++, 0, 0, SCREEN_WIDTH, 25 * gAppNmiBufferPtr->resetCount);  
        gDPFillRectangle(gfx++, 0, SCREEN_HEIGHT - 25 * gAppNmiBufferPtr->resetCount, SCREEN_WIDTH, SCREEN_HEIGHT);  
        
        state->gfxCtx->overlay.p = gfx; 
    }
    else
        gAppNmiBufferPtr->resetCount = 0;

    bool upd = false;
   
    // Reset to defaults if L + R + Z is pressed.
    if (CHECK_BTN_ALL(state->input->cur.button, BTN_L | BTN_R | BTN_Z))
    {
        SAVE_SCREENSIZEX = SCREENSIZE_DEFAULT;
        SAVE_SCREENSIZEY = SCREENSIZE_DEFAULT;
        SAVE_SCREENXPOS = SCREENPOSX_DEFAULT;
        SAVE_SCREENYPOS = SCREENPOSY_DEFAULT;
        
        if (SAVE_DEBUGMODE)
            SAVE_WIDESCREEN = SAVE_WIDESCREEN ? 0 : 1;
        else
            SAVE_WIDESCREEN = WIDESCREEN_DEFAULT;
        
        upd = true;
    }
    
    // Failsafe
    if (SAVE_SCREENSIZEX == 0 || SAVE_SCREENSIZEY == 0)
    {
        SAVE_SCREENSIZEX = SCREENSIZE_DEFAULT;
        SAVE_SCREENSIZEY = SCREENSIZE_DEFAULT;
        upd = true;
    }

    int xPos = (s8)SAVE_SCREENXPOS;
    int yPos = (s8)SAVE_SCREENYPOS;
    
    // Failsafe
    if (xPos < -127 || xPos > 127 || yPos < -127 || yPos > 127)
    {
        xPos = SCREENPOSX_DEFAULT;
        yPos = SCREENPOSY_DEFAULT;
        SAVE_SCREENXPOS = SCREENPOSX_DEFAULT;
        SAVE_SCREENYPOS = SCREENPOSY_DEFAULT;
        upd = true;
    }
    
    if (xPos == 0 && yPos == 0 && SAVE_SCREENSIZEX == SCREENSIZE_DEFAULT && SAVE_SCREENSIZEY == SCREENSIZE_DEFAULT && !upd)
        return;
    
    float sizeOffs = SCREENSIZE_DEFAULT - SAVE_SCREENSIZEX;
    float sizeOffsY = SCREENSIZE_DEFAULT - SAVE_SCREENSIZEY;
    
    int centerOffs = sizeOffs / 2;
    int centerOffsY = sizeOffsY / 2;
    
    int leftAdjust = xPos + centerOffs;
    int rightAdjust = xPos - sizeOffs + centerOffs;
    int upperAdjust = yPos + centerOffsY;
    int lowerAdjust = yPos - sizeOffsY + centerOffsY;
    
    int viType = OS_VI_NTSC_LAN1;
    int tvType = osTvType;
    float scale = 1.0f;
    
    if (*hzChoice == 1)
    {
        viType = OS_VI_FPAL_LAN1;
        tvType = OS_TV_PAL;
        scale = 0.833f;
    }
    else
    {
        if (osTvType == OS_TV_MPAL)
        {
            viType = OS_VI_MPAL_LAN1;
            tvType = OS_TV_MPAL;
        }
        else
        {
            viType = OS_VI_NTSC_LAN1;
            tvType = OS_TV_NTSC;
        }            
    }
    
    // Reset screen size to default before reset or everything can explode
    if (preNMIDone)
    {
        viType = OS_VI_NTSC_LAN1;
        tvType = OS_TV_NTSC;
        leftAdjust = 0;
        rightAdjust = 0;
        upperAdjust = 0;
        lowerAdjust = 0;
        scale = 1.0f;
    }
    
    ViMode_Configure(&sViMode, viType, tvType, 1, 0, 1, 1, SCREEN_WIDTH, SCREEN_HEIGHT, leftAdjust, rightAdjust, upperAdjust, lowerAdjust);
    gViConfigMode = sViMode.customViMode;
    osViSetYScale(scale);
    return;
}

