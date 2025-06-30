#include "global.h"
#include "../../../../actor/_custom-1.0/draw2D.h"

//800755c8
void Draw2DScaled(u8 RGBAType, int object, PlayState* playState, Gfx** gfxp, s16 centerX, s16 centerY, u8* source, u8* sourcePal, u32 width, u32 height, u32 drawWidth, u32 drawHeight, s16 alpha)
{
	int objectIndex = Object_GetIndex(&playState->objectCtx, object);
	
	if (objectIndex == -1)
		return;    
    
    u8* texture = VIRTUAL_TO_PHYSICAL(playState->objectCtx.status[objectIndex].segment) + source;
    u8* palette = VIRTUAL_TO_PHYSICAL(playState->objectCtx.status[objectIndex].segment) + sourcePal; 
    
    Draw2DInternal(RGBAType, texture, palette, gfxp, centerX, centerY, width, height, drawWidth, drawHeight, alpha);
}