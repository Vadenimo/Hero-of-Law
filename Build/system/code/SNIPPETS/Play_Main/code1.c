#include "global.h"

extern Input* D_8012D1F8;
void Play_Main2(PlayState* this);
void Play_Update(PlayState* this);
void DebugDisplay_Init(void);

void Play_Main(GameState* thisx)
{
    PlayState* this = (PlayState*)thisx;
    Play_Main2(this);
}