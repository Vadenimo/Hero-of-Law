#include "global.h"
#include "message_data_static.h"
#include "sfx.h"

#include "../../../../actor/_custom-1.0/common.h"
#include "../../../../actor/_custom-1.0/draw2D.h"
#include "../../../../actor/_custom-1.0/holText.h"

extern void Message_SetTextColor(MessageContext* msgCtx, u16 colorParameter);
        asm("Message_SetTextColor = 0x800D6BA8");

extern u16 Message_DrawItemIcon(PlayState* play, u16 itemId, Gfx** p, u16 i);
        asm("Message_DrawItemIcon = 0x800D75A4");
        
extern void Font_LoadMessageBoxIcon(Font* font, u16 icon);
        asm("Font_LoadMessageBoxIcon = 0x8005BD34");    


#define NA_SE_NONE 0 
#define gPreventTxtboxAdvanceAndSounds msgCtx->textboxBackgroundForeColorIdx
#define gTextSpeed msgCtx->textboxBackgroundBackColorIdx
#define gTestimonySpeakerSoundIdx msgCtx->textboxBackgroundYOffsetIdx
#define gTextboxPaused msgCtx->unk_E3F4

#define NOT_PAUSED 0
#define CURRENTLY_PAUSED 1
#define UNPAUSED_OR_ENDED 2     

#define TEXT_SCALE 75

#define SOUND_HOL_EXPLOSION 0x6E3A

// =============== UNUSED STUFF
s16** sTextboxBackgroundForePrimColors = (s16**)0x80112E40;  // used one byte for useSquareEnd
s16** sTextboxBackgroundBackPrimColors = (s16**)0x80112E70;
s16* sTextboxBackgroundYOffsets = (s16*)0x80112E88;
s16* sTextIsCredits = (s16*)0x8010A934;
u8* sTextboxSkipped = (u8*)0x8010A930;
s16* sMessageHasSetSfx = (s16*)0x8010A944;
// =============== UNUSED STUFF


f32* sFontWidths = (f32*)0x80112F40;
u8 lastSndEffectPlayedAtI = 0;
u8 lastQuickDrawEnableAtI = 0;
u8 lastQuickDrawDisableAt = 0;
u8 lastCommaAt = 0;
u8 lastChar;
u8* useSquareEnd = (u8*)0x80112E40;
bool msgUnskippable = false;

static Vec3f pos = {0, 0, 1}; 

typedef enum
{
    SPEAKER_COMMON_FIRST = 250,
    SPEAKER_TYPEWRITER = 251,
    SPEAKER_CROSS_EXAM = 252,
    SPEAKER_REBUTTAL = 253,
    SPEAKER_UNKNOWN = 254,
    SPEAKER_NONE = 255
} SpeakerType;


typedef struct SpeakerEntry
{
	u8 id;
    bool disableIndicator;
    u16 sndID;
	f32 sndPitch;
	Color_RGB8 textboxColor;
	Color_RGB8 textColor;
	Color_RGB8 textShadowColor;
	char* text;    
} SpeakerEntry;

typedef struct UIStruct
{
    Actor actor;
    u8 guiSpeaker;
    u8 guiToEnable;
    u8 guiArrows;
    u8 guiSubtitle;
    u8 guiShowHearts;
    s8 guiDrawIcon;
    u8 guiShowHeartsDamageFrame;
    u8 guiDrawCheckmark;
    u8 guiCEStatementCount;
    u8 guiCECurStatement;
    u8 guiPickerEnabled;
    u8 guiExplodeTextboxStatus;
    u8 guiMsgLoggingDisabled;
    u8 guiMsgLogStatus;
    u8 guiMsgLogCurrentScene;
    u8 guiCourtRecordStatus;
    s8 guiCourtRecordPlayerPresented;
    u8 guiCourtRecordMode;
    s8 guiCourtRecordListOnly;
    s8 guiSpeakerForce;
    s8 guiLogSubtitle;
    u8 pad1;
    u8 pad2;
    u8 pad3;
    u16 guiPickerPosX;
    u16 guiPickerPosY;
    SpeakerEntry* curSpeakerData;
    SpeakerEntry* curTestimonySpeakerData;
} UIStruct;


// REMEMBER TO EDIT MESSAGE_SHOULDADVANCESILENT!!!!!!!!!!!!!!!!!!
//800D8EF4
void Message_DrawText(PlayState* play, Gfx** gfxP) 
{
    MessageContext* msgCtx = &play->msgCtx;
    Font* font = &play->msgCtx.font;
    Input* input = &play->state.input[0];
    Gfx* gfx = *gfxP;
    
    // Find the GUI actor
    UIStruct* act = (UIStruct*)play->actorCtx.actorLists[ACTORCAT_PROP].head;

    while (act)
    {
        if (act->actor.id == 1)
            break;

        act = (UIStruct*)act->actor.next;
    }

    u8 character;
    u8 textShift = 0;
    u16 j;
    u16 i;
    u16 sfxHi;
    u16 charTexIdx = 0;
    
    bool disableSnd = false;
    bool disableSndManual = false;
    bool dontDoSoundForThisChar = false;

    msgCtx->textPosX = R_TEXT_INIT_XPOS;
    msgCtx->textPosY = R_TEXT_INIT_YPOS;

    if (msgCtx->textBoxType == TEXTBOX_TYPE_NONE_NO_SHADOW) 
        msgCtx->textColorR = msgCtx->textColorG = msgCtx->textColorB = 0;
    else
        msgCtx->textColorR = msgCtx->textColorG = msgCtx->textColorB = 255;
    
    // Starting a new message!
    if (msgCtx->textDrawPos == 1)
    {
        lastSndEffectPlayedAtI = 0;
        lastQuickDrawEnableAtI = 0;
        lastQuickDrawDisableAt = 0;
        lastCommaAt = 0;
        msgCtx->textDrawPos = gTextSpeed;
        
        msgUnskippable = true;
        gTextboxPaused = NOT_PAUSED;
        msgCtx->textUnskippable = true;
    }

    character = msgCtx->msgBufDecoded[msgCtx->textDrawPos - 1];
    
    // Mark message as unpaused if it's currently paused but the last character wasn't a quicktext disable/speed character
    if (msgCtx->textDelayTimer == 0 && gTextboxPaused == CURRENTLY_PAUSED && character != MESSAGE_QUICKTEXT_DISABLE && character != MESSAGE_TEXT_SPEED)
        gTextboxPaused = UNPAUSED_OR_ENDED; 
    
    // Skip typing out the current message.
    if (!gPreventTxtboxAdvanceAndSounds && CHECK_BTN_ALL(input->cur.button, BTN_CRIGHT) && !msgUnskippable)
    {
        lastCommaAt = 200;
        msgCtx->textDrawPos = 200;
    }
    
    // gPreventTxtboxAdvanceAndSounds is used when unpausing the court record, so we set the whole message to be typed out instantly.
    if (gPreventTxtboxAdvanceAndSounds)
    {
        lastSndEffectPlayedAtI = 200;
        lastQuickDrawEnableAtI = 200;
        lastQuickDrawDisableAt = 200;
        lastCommaAt = 200;
        msgCtx->textDrawPos = 200;
        msgCtx->textDelayTimer = 0;
    }
    
    lastChar = 0;
    *useSquareEnd = false;
    msgUnskippable = false;

    for (i = 0; i < msgCtx->textDrawPos; i++) 
    {
        character = msgCtx->msgBufDecoded[i];
        dontDoSoundForThisChar = false;

        switch (character) 
        {
            // Removed, unnecessary.
            case MESSAGE_OCARINA:
            case MESSAGE_BACKGROUND:
            case MESSAGE_TEXTID:
            case MESSAGE_EVENT:
            case MESSAGE_AWAIT_BUTTON_PRESS:
            case 0xFF:
                break;
            case 0xFE:
                *useSquareEnd  = !*useSquareEnd;
                break;
            case 0xFD:
            {
                int width = GetTextPxWidth(&msgCtx->msgBufDecoded[i + 1], TEXT_SCALE);
                msgCtx->textPosX = R_TEXTBOX_X + (R_TEXTBOX_WIDTH / 2) - (width / 2);
            }
            break;
            case MESSAGE_FADE2:
                i+=2;
                break;
            case MESSAGE_NEWLINE:
                msgCtx->textPosX = R_TEXT_INIT_XPOS + textShift;
                msgCtx->textPosY += R_TEXT_LINE_SPACING;
                
                break;
            case MESSAGE_COLOR:
                // If text color is Green or Cyan, disable the text typer sounds.
                if (msgCtx->msgBufDecoded[i + 1] == 0x43 || msgCtx->msgBufDecoded[i + 1] == 0x44)
                    disableSnd = true;
                else
                    disableSnd = false;
                            
                Message_SetTextColor(msgCtx, msgCtx->msgBufDecoded[++i] & 0xF);
                break;
            case MESSAGE_BOX_BREAK:
                if (msgCtx->msgMode == MSGMODE_TEXT_DISPLAYING) 
                {
                    gTextboxPaused = UNPAUSED_OR_ENDED;
                    msgCtx->msgMode = MSGMODE_TEXT_AWAIT_NEXT;
                    Font_LoadMessageBoxIcon(font, TEXTBOX_ICON_TRIANGLE);
                    lastChar = 0;
                }
                *gfxP = gfx;
                return;
            case MESSAGE_SHIFT:
                msgCtx->textPosX += msgCtx->msgBufDecoded[++i];
                break;
            case MESSAGE_QUICKTEXT_ENABLE:
                // This is way more complex than in the original code because it needs to account for multiple
                // characters being printed per frame
                if (i >= lastQuickDrawEnableAtI && (msgCtx->msgMode == MSGMODE_TEXT_DISPLAYING ||
                                                     (msgCtx->msgMode >= MSGMODE_OCARINA_STARTING &&
                                                      msgCtx->msgMode < MSGMODE_SCARECROW_LONG_RECORDING_START))) 
                {
                                                
                    j = i;
                    lastQuickDrawEnableAtI = i + 1;
                    while (true) {
                        character = msgCtx->msgBufDecoded[j];
                        if (character == MESSAGE_SHIFT) {
                            j += 2;
                        } else if ((character != MESSAGE_QUICKTEXT_DISABLE) && (character != MESSAGE_PERSISTENT) &&
                                   (character != MESSAGE_EVENT) && (character != MESSAGE_BOX_BREAK_DELAYED) &&
                                   (character != MESSAGE_AWAIT_BUTTON_PRESS) && (character != MESSAGE_BOX_BREAK) &&
                                   (character != MESSAGE_END)) {
                            j++;
                        } else {
                            break;
                        }
                    }
                    i = j - 1;
                    lastCommaAt = i + 1;
                    msgCtx->textDrawPos = i + 1;
                    dontDoSoundForThisChar = true;
                }
                break;
            case MESSAGE_QUICKTEXT_DISABLE:
                dontDoSoundForThisChar = true;
                
                // If the previous character was also a QUICKTEXT_DISABLE, then we're pausing the textbox. 
                if (msgCtx->textDrawPos >= lastQuickDrawDisableAt)
                {
                    if (msgCtx->textDrawPos != 0)
                    {
                        character = msgCtx->msgBufDecoded[msgCtx->textDrawPos - 1];
                        
                        if (character == MESSAGE_QUICKTEXT_DISABLE)
                            gTextboxPaused = CURRENTLY_PAUSED;
                    }
                }
                
                lastQuickDrawDisableAt = msgCtx->textDrawPos;
                break;
            case MESSAGE_BOX_BREAK_DELAYED:
                if (msgCtx->msgMode == MSGMODE_TEXT_DISPLAYING) 
                {
                    msgCtx->stateTimer = msgCtx->msgBufDecoded[++i];
                    msgCtx->msgMode = MSGMODE_TEXT_DELAYED_BREAK;
                    gTextboxPaused = UNPAUSED_OR_ENDED;
                }
                *gfxP = gfx;
                return;
            case MESSAGE_SFX:
                sfxHi = msgCtx->msgBufDecoded[i + 1];
                sfxHi <<= 8;
                
                int sndID = sfxHi | msgCtx->msgBufDecoded[i + 2];
                
                if (sndID == NA_SE_NONE)
                    disableSndManual = !disableSndManual;
                else
                {
                    if (!gPreventTxtboxAdvanceAndSounds)
                    {
                        // Or it has already been played.
                        if (i >= lastSndEffectPlayedAtI)
                        {
                            Audio_PlaySfxGeneral(sndID, &pos, 4, &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);     
                            lastSndEffectPlayedAtI = i + 2;
                            
                            // If it's an EXPLOSION sfx, shake the screen too.
                            if (sndID == SOUND_HOL_EXPLOSION)
                            {
                                s32 temp = Quake_Add(GET_ACTIVE_CAM(play), 1);
                                Quake_SetSpeed(temp, 0x3A98);
                                Quake_SetQuakeValues(temp, 0, 1, 0xFA, 1);
                                Quake_SetCountdown(temp, 0xA);
                                Rumble_Request(0, 255, 0xA, 150);
                            }                       
                        }                               
                    }
                    else
                        lastSndEffectPlayedAtI = i + 2;
                }

                i += 2;
                break;
            case MESSAGE_ITEM_ICON:
			{
				u8 iconIdx = msgCtx->msgBufDecoded[i + 1];
				void* offs = (void*)(iconIdx * 64 * 64 * 4);
				
				Draw2DScaled(RGBA32, 7, play, &gfx, msgCtx->textPosX + R_TEXTBOX_ICON_XPOS + 16, R_TEXTBOX_ICON_YPOS + 16, offs, NULL, 64, 64, 32, 32, 255);
                Gfx_SetupDL_39Ptr(&gfx);
			
				//gDPSetAlphaCompare(gfx++, G_AC_NONE);
				gDPSetCombineLERP(gfx++, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0);				
						
                textShift = 32;
				msgCtx->textPosX += 32;
                lastChar = 0;
				i++;
                break;
			}
            case MESSAGE_TEXT_SPEED:
                {
                    u8 speed = msgCtx->msgBufDecoded[++i];
                    
                    if (speed >= 101)
                        gTextSpeed = speed - 100;
                    else
                        msgCtx->textDelay = speed;
                }
                break;
            case MESSAGE_UNSKIPPABLE:
                {
                    msgUnskippable = true;
                }
                break;
            case MESSAGE_TWO_CHOICE:
            case MESSAGE_THREE_CHOICE:
                msgCtx->textboxEndType = character == MESSAGE_THREE_CHOICE ? TEXTBOX_ENDTYPE_3_CHOICE : TEXTBOX_ENDTYPE_2_CHOICE;
                textShift = 16;
                msgCtx->textPosX += 16;
                if (msgCtx->msgMode == MSGMODE_TEXT_DISPLAYING) 
                {
                    msgCtx->choiceTextId = msgCtx->textId;
                    msgCtx->stateTimer = 4;
                    msgCtx->choiceIndex = 0;
                    Font_LoadMessageBoxIcon(font, TEXTBOX_ICON_ARROW);
                    lastChar = 0;
                }
                break;
            case MESSAGE_END:
                if (msgCtx->msgMode == MSGMODE_TEXT_DISPLAYING) 
                {
                    msgCtx->msgMode = MSGMODE_TEXT_DONE;
                    if (msgCtx->textboxEndType == TEXTBOX_ENDTYPE_DEFAULT) 
                    {
                        //if (!gPreventTxtboxAdvanceAndSounds)
                        //    Audio_PlaySfxGeneral(NA_SE_SY_MESSAGE_END, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale,&gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
                        
                        Font_LoadMessageBoxIcon(font, *useSquareEnd  ? TEXTBOX_ICON_SQUARE : TEXTBOX_ICON_TRIANGLE);
                        lastChar = 0;
                    }
                    gTextboxPaused = UNPAUSED_OR_ENDED;
                }
                *gfxP = gfx;
                return;
            case MESSAGE_FADE:
                if (msgCtx->msgMode == MSGMODE_TEXT_DISPLAYING) 
                {
                    msgCtx->msgMode = MSGMODE_TEXT_DONE;
                    msgCtx->textboxEndType = TEXTBOX_ENDTYPE_FADING;
                    msgCtx->stateTimer = msgCtx->msgBufDecoded[++i];
                    
                    Font_LoadMessageBoxIcon(font, *useSquareEnd  ? TEXTBOX_ICON_SQUARE : TEXTBOX_ICON_TRIANGLE);
                        
                    lastChar = 0;

                    gTextboxPaused = UNPAUSED_OR_ENDED;
                }
                *gfxP = gfx;
                return;
            case MESSAGE_PERSISTENT:
                if (msgCtx->msgMode == MSGMODE_TEXT_DISPLAYING) 
                {
                    msgCtx->msgMode = MSGMODE_TEXT_DONE;
                    msgCtx->textboxEndType = TEXTBOX_ENDTYPE_PERSISTENT;
                    gTextboxPaused = UNPAUSED_OR_ENDED;
                }
                *gfxP = gfx;
                return;
            case ' ':
                msgCtx->textPosX += (s32)(sFontWidths[0] * (R_TEXT_CHAR_SCALE / 100.0f));
                break;
            default:
            {
                // Walk back a pixel to fix the kerning of select characters following a ( bracket.
                // This is WMC's fault. Blame HIM. 
                if (i != 0 && msgCtx->msgBufDecoded[i - 1] == '(' && strchr("CGOQScdeotuvw~(@$^?<", character)) 
                    msgCtx->textPosX -= 1;            
            
                if (msgCtx->msgMode == MSGMODE_TEXT_DISPLAYING && i + 1 == msgCtx->textDrawPos &&msgCtx->textDelayTimer == msgCtx->textDelay) 
                {
                    // Play a textbox typer sound if the UI actor exists, sound is not disabled and the textbox is not a choice one.
                    if (act != NULL && 
                        !disableSnd && 
                        !disableSndManual && 
                        !dontDoSoundForThisChar && 
                        msgCtx->textboxEndType != TEXTBOX_ENDTYPE_2_CHOICE && 
                        msgCtx->textboxEndType != TEXTBOX_ENDTYPE_3_CHOICE &&
                        !gPreventTxtboxAdvanceAndSounds)
                    {
                        // For testimony; sets the speaker sound based on a field in msgCtx.
                        if (gTestimonySpeakerSoundIdx > 0 && act->curTestimonySpeakerData != NULL && (act->guiSpeaker == SPEAKER_CROSS_EXAM || act->guiSpeaker == SPEAKER_REBUTTAL))
                            Audio_PlaySfxGeneral(act->curTestimonySpeakerData->sndID, &gSfxDefaultPos, 4, &act->curTestimonySpeakerData->sndPitch, &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);  
                        // Else, if the data is available, play speaker sound.
                        else if (act->curSpeakerData != NULL)
                            Audio_PlaySfxGeneral(act->curSpeakerData->sndID, &gSfxDefaultPos, 4, &act->curSpeakerData->sndPitch, &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);                 
                    }
                }
                
                // Slight pause on every comma!
                if (i >= lastCommaAt && character == ',')
                {
                    lastCommaAt = i + 1;
                    msgCtx->textDrawPos = i + 1;
                    msgCtx->textDelayTimer += 2;
                }

                DrawCharTexture(play, &gfx, &msgCtx->font.fontBuf[(character - ' ') * FONT_CHAR_TEX_SIZE], msgCtx->textPosX, msgCtx->textPosY, TEXT_SCALE, lastChar != character,
                                msgCtx->textColorAlpha, (Color_RGB8){msgCtx->textColorR, msgCtx->textColorG, msgCtx->textColorB}, (Color_RGB8){0, 0, 0}, true, msgCtx->textColorAlpha, 1, 1);
                                
  
                lastChar = character;
                
                float width = sFontWidths[character - ' '];

                if (character > 0xAB)
                    width = 13;
                
                msgCtx->textPosX += (s32)(width * (R_TEXT_CHAR_SCALE / 100.0f));
                break;
            }
        }
    }
    
    if (msgCtx->textDelayTimer == 0) 
    {
        u8 sp = gTextSpeed + (R_UPDATE_RATE - 3);
        sp = sp < 1 ? 1 : sp; 
        msgCtx->textDrawPos = i + sp;
        msgCtx->textDelayTimer = msgCtx->textDelay;
    } 
    else 
    {
        msgCtx->textDelayTimer--;
    }

    *gfxP = gfx;
}