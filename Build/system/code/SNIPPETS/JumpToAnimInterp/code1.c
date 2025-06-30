#include "global.h"

extern void Patched_InterpFrameTable(s32 limbCount, Vec3s* dst, Vec3s* start, Vec3s* target, f32 weight);
        asm("Patched_InterpFrameTable = 0x80064d98 + 0x8");      

// 8008AFB8
void SkelAnime_InterpFrameTable(s32 limbCount, Vec3s* dst, Vec3s* start, Vec3s* target, f32 weight) 
{
   Patched_InterpFrameTable(limbCount, dst, start, target, weight);
}
