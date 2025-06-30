#include "global.h"

extern Input* D_8012D1F8;
void Play_Draw2(PlayState* this);
void Play_Update(PlayState* this);
void DebugDisplay_Init(void);

void Play_Main(GameState* thisx)
{
    PlayState* this = (PlayState*)thisx;

    D_8012D1F8 = &this->state.input[0];

    DebugDisplay_Init();
    Play_Update(this);
    Play_Draw2(this);
}