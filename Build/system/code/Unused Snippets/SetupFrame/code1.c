#include "global.h"
#include "../common.h"

extern Gfx sFillSetupDL[];
extern s32 gTransitionTileState;

#define R_PAUSE_BG_PRERENDER_STATE               SREG(94)

typedef enum PauseBgPreRenderState {
    /* 0 */ PAUSE_BG_PRERENDER_OFF, // Inactive, do nothing.
    /* 1 */ PAUSE_BG_PRERENDER_SETUP, // The current frame is only drawn for the purpose of serving as the pause background.
    /* 2 */ PAUSE_BG_PRERENDER_PROCESS, // The previous frame was PAUSE_BG_PRERENDER_SETUP, now apply prerender filters.
    /* 3 */ PAUSE_BG_PRERENDER_READY, // The pause background is ready to be used.
    /* 4 */ PAUSE_BG_PRERENDER_MAX
} PauseBgPreRenderState;

typedef enum TransitionTileState {
    /* 0 */ TRANS_TILE_OFF, // Inactive, do nothing
    /* 1 */ TRANS_TILE_SETUP, // Save the necessary buffers
    /* 2 */ TRANS_TILE_PROCESS, // Initialize the transition
    /* 3 */ TRANS_TILE_READY // The transition is ready, so will update and draw each frame
} TransitionTileState;

void Gfx_SetupFrame(GraphicsContext* gfxCtx, u8 r, u8 g, u8 b) 
{
    GraphicsContext* __gfxCtx = gfxCtx;
    
    // Set up the RDP render state for rectangles in FILL mode
    gSPDisplayList(POLY_OPA_DISP++, sFillSetupDL);
    gSPDisplayList(POLY_XLU_DISP++, sFillSetupDL);
    gSPDisplayList(OVERLAY_DISP++, sFillSetupDL);

    // Set the scissor region to the full screen
    gDPSetScissor(POLY_OPA_DISP++, G_SC_NON_INTERLACE, 0, 0, gScreenWidth, gScreenHeight);
    gDPSetScissor(POLY_XLU_DISP++, G_SC_NON_INTERLACE, 0, 0, gScreenWidth, gScreenHeight);
    gDPSetScissor(OVERLAY_DISP++, G_SC_NON_INTERLACE, 0, 0, gScreenWidth, gScreenHeight);

    // Set up the framebuffer, primitives will be drawn here
    gDPSetColorImage(POLY_OPA_DISP++, G_IM_FMT_RGBA, G_IM_SIZ_16b, gScreenWidth, gfxCtx->curFrameBuffer);
    gDPSetColorImage(POLY_OPA_DISP++, G_IM_FMT_RGBA, G_IM_SIZ_16b, gScreenWidth, gfxCtx->curFrameBuffer);
    gDPSetColorImage(POLY_XLU_DISP++, G_IM_FMT_RGBA, G_IM_SIZ_16b, gScreenWidth, gfxCtx->curFrameBuffer);
    gDPSetColorImage(OVERLAY_DISP++, G_IM_FMT_RGBA, G_IM_SIZ_16b, gScreenWidth, gfxCtx->curFrameBuffer);

    // Set up the z-buffer
    gDPSetDepthImage(POLY_OPA_DISP++, gZBuffer);
    gDPSetDepthImage(POLY_XLU_DISP++, gZBuffer);
    gDPSetDepthImage(OVERLAY_DISP++, gZBuffer);

    if ((R_PAUSE_BG_PRERENDER_STATE <= PAUSE_BG_PRERENDER_SETUP) && (gTransitionTileState <= TRANS_TILE_SETUP)) {
        s32 letterboxSize = Letterbox_GetSize();

#if DEBUG_FEATURES
        if (R_HREG_MODE == HREG_MODE_SETUP_FRAME) {
            if (R_SETUP_FRAME_INIT != HREG_MODE_SETUP_FRAME) {
                R_SETUP_FRAME_GET = (SETUP_FRAME_LETTERBOX_SIZE_FLAG | SETUP_FRAME_BASE_COLOR_FLAG);
                R_SETUP_FRAME_SET = (SETUP_FRAME_LETTERBOX_SIZE_FLAG | SETUP_FRAME_BASE_COLOR_FLAG);
                R_SETUP_FRAME_LETTERBOX_SIZE = 0;
                R_SETUP_FRAME_BASE_COLOR_R = 0;
                R_SETUP_FRAME_BASE_COLOR_G = 0;
                R_SETUP_FRAME_BASE_COLOR_B = 0;

                // these regs are not used in this mode
                HREG(87) = 0;
                HREG(88) = 0;
                HREG(89) = 0;
                HREG(90) = 0;
                HREG(91) = 0;
                HREG(92) = 0;
                HREG(93) = 0;
                HREG(94) = 0;

                R_SETUP_FRAME_INIT = HREG_MODE_SETUP_FRAME;
            }

            if (R_SETUP_FRAME_GET & SETUP_FRAME_LETTERBOX_SIZE_FLAG) {
                R_SETUP_FRAME_LETTERBOX_SIZE = letterboxSize;
            }

            if (R_SETUP_FRAME_GET & SETUP_FRAME_BASE_COLOR_FLAG) {
                R_SETUP_FRAME_BASE_COLOR_R = r;
                R_SETUP_FRAME_BASE_COLOR_G = g;
                R_SETUP_FRAME_BASE_COLOR_B = b;
            }

            if (R_SETUP_FRAME_SET & SETUP_FRAME_LETTERBOX_SIZE_FLAG) {
                letterboxSize = R_SETUP_FRAME_LETTERBOX_SIZE;
            }

            if (R_SETUP_FRAME_SET & SETUP_FRAME_BASE_COLOR_FLAG) {
                r = R_SETUP_FRAME_BASE_COLOR_R;
                g = R_SETUP_FRAME_BASE_COLOR_G;
                b = R_SETUP_FRAME_BASE_COLOR_B;
            }
        }
#endif

        // Set the whole z buffer to maximum depth
        // Don't bother with pixels that are being covered by the letterbox
        gDPSetColorImage(POLY_OPA_DISP++, G_IM_FMT_RGBA, G_IM_SIZ_16b, gScreenWidth, gZBuffer);
        gDPSetCycleType(POLY_OPA_DISP++, G_CYC_FILL);
        gDPSetRenderMode(POLY_OPA_DISP++, G_RM_NOOP, G_RM_NOOP2);
        gDPSetFillColor(POLY_OPA_DISP++, (GPACK_ZDZ(G_MAXFBZ, 0) << 16) | GPACK_ZDZ(G_MAXFBZ, 0));
        gDPFillRectangle(POLY_OPA_DISP++, 0, letterboxSize, gScreenWidth - 1, gScreenHeight - letterboxSize - 1);
        gDPPipeSync(POLY_OPA_DISP++);

        // Fill the whole screen with the base color
        // Don't bother with pixels that are being covered by the letterbox
        gDPSetColorImage(POLY_OPA_DISP++, G_IM_FMT_RGBA, G_IM_SIZ_16b, gScreenWidth, gfxCtx->curFrameBuffer);
        gDPSetCycleType(POLY_OPA_DISP++, G_CYC_FILL);
        gDPSetRenderMode(POLY_OPA_DISP++, G_RM_NOOP, G_RM_NOOP2);
        gDPSetFillColor(POLY_OPA_DISP++, (GPACK_RGBA5551(r, g, b, 1) << 16) | GPACK_RGBA5551(r, g, b, 1));
        gDPFillRectangle(POLY_OPA_DISP++, 0, letterboxSize, gScreenWidth - 1, gScreenHeight - letterboxSize - 1);
        gDPPipeSync(POLY_OPA_DISP++);

        // Draw the letterbox if applicable (uses the same color as the screen base)
        if (letterboxSize > 0) {
            gDPPipeSync(OVERLAY_DISP++);
            gDPSetCycleType(OVERLAY_DISP++, G_CYC_FILL);
            gDPSetRenderMode(OVERLAY_DISP++, G_RM_NOOP, G_RM_NOOP2);
            gDPSetFillColor(OVERLAY_DISP++, (GPACK_RGBA5551(r, g, b, 1) << 16) | GPACK_RGBA5551(r, g, b, 1));
            gDPFillRectangle(OVERLAY_DISP++, 0, 0, gScreenWidth - 1, letterboxSize - 1);
            gDPFillRectangle(OVERLAY_DISP++, 0, gScreenHeight - letterboxSize, gScreenWidth - 1, gScreenHeight - 1);
            gDPPipeSync(OVERLAY_DISP++);
        }
    }
}