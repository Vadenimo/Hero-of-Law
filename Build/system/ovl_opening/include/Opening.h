#ifndef _Z_OPENING_H
#define _Z_OPENING_H

#include "../../../actor/_custom-1.0/holText.h"
#include "../../../actor/_custom-1.0/draw2D.h"
#include "../../../actor/_custom-1.0/is64Printf.h"
#include <z64hdr/oot_u10/z64hdr.h>
#include "sfx.h"

extern void* THA_AllocTailAlign16(TwoHeadArena* tha, size_t size);
	asm("THA_AllocTailAlign16 = 0x800A01B8");

extern void Sram_InitDefSave();
	asm("Sram_InitDefSave = 0x800900EC");
	
extern Gfx* func_8009411C(Gfx* gfx);
	asm("func_8009411C = 0x8007E008");	

extern void func_80095248(GraphicsContext* gfxCtx, u8 r, u8 g, u8 b);
	asm("func_80095248 = 0x8007EE5C");

extern void Environment_FillScreen(GraphicsContext* gfxCtx, u8 red, u8 green, u8 blue, u8 alpha, u8 drawFlags);
	asm("Environment_FillScreen = 0x800625B0");

extern void Yaz0_Decompress(uintptr_t romStart, u8* dst, u32 size);
	asm("Yaz0_Decompress = 0x80001254");
  
extern void View_SetPerspective(View* view, f32 fovy, f32 zNear, f32 zFar);
    asm("View_SetPerspective = 0x80091A34");

extern void Gfx_SetupDL_39Opa(GraphicsContext* gfxCtx);
    asm("Gfx_SetupDL_39Opa = 0x8007E5E4");

float Math_PowF(f32 base, s32 exp);
    asm("Math_PowF = 0x800A45D4");
    
#define objectTable (*(RomFile(*)[]) 0x800F8FF8)
   
#define NES_FONT_STATIC 0x928000
#define N64_LOGO_TEXT ((u8*)0x01000000)
#define N64_LOGO_DLIST ((u8*)0x010027A0)
#define N64_LOGO_SHEEN ((u8*)0x01001800)
#define CONTROLLER_GRAPHIC_X 160
#define CONTROLLER_GRAPHIC_Y 160
#define NUM_CHARS 139
#define CONSOLE_LOGO_SIZE 0x2E50    

#define HZSWITCH

#define AVAL(base,type,offset)  (*(type*)((u8*)(base)+(offset)))

typedef struct 
{
    s16 ult;
    s16 uls;
    int spinSpeed;
    int logoRot;
    int coverAlpha;
    int addAlpha; 
    int visibleDuration; 
    int logoState;
    int controllerInfoState;
    int fade;
    int timer;
    int hzChoice;
    int hzChoiceTimer;
    bool hzChoiceMade;
    bool hzChoiceForce;
    s16 hzChoiceAlpha;
    float logoScale;
    float colorInterpolationFraction;
    Vec3f logoPosOffs;
    Vec3f logoPosMoveVec;
    float logoPosMoveDiff;
    Color_RGBA8 n64TextColorPrim;
    Color_RGBA8 n64TextColorEnv;    
    
    u8* fontGfx;
    u8* controllerInfoGfx;
    u8 segBuf[CONSOLE_LOGO_SIZE] __attribute__ ((aligned (16)));

} OpeningData;


u32* memSize = (u32*)0x80000318;
u32* hzChoice = (u32*)0x80198950;

OpeningData* that;

Color_RGBA8 primBlue = (Color_RGBA8){255, 255, 255, 255};
Color_RGBA8 envBlue = (Color_RGBA8){0, 0, 255, 128};

Color_RGBA8 primGold = (Color_RGBA8){244, 255, 0, 255};
Color_RGBA8 envGold = (Color_RGBA8){244, 122, 0, 128};

Color_RGBA8 primRed = (Color_RGBA8){196, 170, 170, 255};
Color_RGBA8 envRed = (Color_RGBA8){255, 0, 0, 128};

Color_RGB8 noExpPakShadowColor = (Color_RGB8){50, 0, 0};
Color_RGB8 noExpPakFontColor = (Color_RGB8){255, 0, 0};

Color_RGB8 controllerInfoShadowColor = (Color_RGB8){150, 150, 150};
Color_RGB8 controllerInfoFontColor = (Color_RGB8){0, 0, 0};

Color_RGB8 hzSwitchFontColor = (Color_RGB8){255, 255, 255};
Color_RGB8 hzSwitchShadowFontColor = (Color_RGB8){10, 10, 10};

char* noExpPakLine1 = "- EXPANSION PAK NOT DETECTED -";
char* noExpPakLine2 = "This game requires expanded memory to run.";
char* noExpPakLine3 = "Please put an Expansion Pak into your console";
char* noExpPakLine4 = "or adjust your emulator settings.";
            
char* noControllerLine1 = "- NO CONTROLLER -";
char* noControllerLine2 = "Turn the power OFF, and plug a controller into";
char* noControllerLine3 = "Controller Port 1 or adjust your emulator settings.";

            
char* controllerInfoLine1 = "If you're playing with an official N64 controller,";
char* controllerInfoLine2 = "please hold it like this.";

char* pickHzLine1 = "This game is best played in 60 Hz mode.";
char* pickHzLine2 = "If your TV does not display correctly in 60 Hz mode,";
char* pickHzLine3 = "please select 50 Hz mode.";

char* hzOption1 = "60 Hz";
char* hzOption2 = "50 Hz";
char* hzCursor = ">         <";

char* debugMusicString = "MUSIC, INNIT";
char* debugMusicStringControls = "A - Fast Forward, B - REALLY Fast Forward";

typedef enum 
{
    STATE_INIT,
    STATE_FADEIN_WHITE,
    STATE_FADEIN_GRAPHICS,
    STATE_FADEOUT_GRAPHICS,
    STATE_FADEOUT_WHITE,
	STATE_EXIT,
	STATE_NO_EXPANSION_PAK,
} OpeningState;

typedef enum 
{
    LOGOSTATE_INIT,
    LOGOSTATE_MEMORY_MISSING_MOVE_UP,
    LOGOSTATE_MEMORY_MISSING,
    LOGOSTATE_NO_CONTROLLER_MOVE_UP,
    LOGOSTATE_NO_CONTROLLER,
    LOGOSTATE_PICK6050HZ_MOVE_UP,
    LOGOSTATE_PICK6050HZ,
    LOGOSTATE_PICK6050HZ_MOVE_DOWN,
    LOGOSTATE_SPEEDUP,
    LOGOSTATE_MOVE,
    LOGOSTATE_DONE,
    
} TitleState;


#define REFRESH_RATE_PAL 50
#define REFRESH_RATE_MPAL 60
#define REFRESH_RATE_NTSC 60

#define REFRESH_RATE_DEVIATION_PAL 1.001521f
#define REFRESH_RATE_DEVIATION_MPAL 0.99276f
#define REFRESH_RATE_DEVIATION_NTSC 1.00278f

#define SEQCMD_RESET_AUDIO_HEAP(sfxChannelLayout, specId) \
    Audio_QueueSeqCmd((0xF  << 28) | ((u8)(sfxChannelLayout) << 8) | (u8)(specId))

#endif
