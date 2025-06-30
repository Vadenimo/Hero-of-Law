/**
 * File: voicesetword.c
 *
 * Registers words to the Voice Recognition System dictionary
 */
#include "../../include/voice.h"

#include "ultra64.h"
#include "../../include/PR/controller_voice.h"
#include "../../include/PR/os_voice.h"
#include "ultra64/controller.h"
#include "macros.h"
#include "../../../is64Printf.h"

s32 osVoiceSetWord(OSVoiceHandle* hd, u8* word, u8 region, u16 USAframes) 
{
    s32 i;
    s32 k;
    s32 errorCode;
    u8 status;
    u8 data[40];

    errorCode = __osVoiceGetStatus(hd->__mq, hd->__channel, &status);
    if (errorCode != 0) {
        return errorCode;
    }

    if (status & 2) {
        return CONT_ERR_VOICE_NO_RESPONSE;
    }

    bzero(data, ARRAY_COUNT(data));
    k = 0;
    
    if (region == VRS_REGION_JPN)
    {
        while (word[k] != 0)
            k+=2;
        
        for (i = 0; i < k; i += 2) 
        {
            data[ARRAY_COUNT(data) - k + i - 1] = word[i];
            data[ARRAY_COUNT(data) - k + i - 2] = word[i + 1];
        }       
        
        data[ARRAY_COUNT(data) - i - 6] = 3;
        
        if (k >= 15) 
        {
            errorCode = __osVoiceContWrite20(hd->__mq, hd->__channel, 0, &data[0]);
            if (errorCode != 0) {
                return errorCode;
            }
        }        
    }
    else
    {
        while (word[k] != 0 || word[k+1] != 0) 
            k+=2;        
        
        __osVoiceContWrite20(hd->__mq, hd->__channel, 0, &data[0]);
        __osVoiceContWrite20(hd->__mq, hd->__channel, 0, &data[0]);    
        
        for (i = 0; i < k; i += 2) 
        {
            data[ARRAY_COUNT(data) - k + i - 3] = word[i];
            data[ARRAY_COUNT(data) - k + i - 4] = word[i + 1];
        }          
        
        data[ARRAY_COUNT(data) - i - 6] = k / 2;
        u8* USAframes8 = (u8*)&USAframes;
        
        data[ARRAY_COUNT(data) - i - 7] = USAframes8[0];
        data[ARRAY_COUNT(data) - i - 8] = USAframes8[1];        
        
        u16 checkSum = 0;
        
        for (int c = 0; c < 40; c += 2)
        {
            u16 val = (data[c + 1] << 8) | data[c];
            checkSum -= val;
        }
        
        u8* checkSum8 = (u8*)&checkSum;
        data[37] = checkSum8[0];
        data[36] = checkSum8[1];          

        data[ARRAY_COUNT(data) - i - 12] = 3;

        if (k >= 11) 
        {
            errorCode = __osVoiceContWrite20(hd->__mq, hd->__channel, 0, &data[0]);
            if (errorCode != 0) {
                return errorCode;
            }
        } 
    }
    errorCode = __osVoiceContWrite20(hd->__mq, hd->__channel, 0, &data[20]);
    if (errorCode != 0) {
        return errorCode;
    }
   
    errorCode = __osVoiceCheckResult(hd, &status);
    
    if (errorCode != 0) {
        if (errorCode & 0x100) {
            errorCode = CONT_ERR_VOICE_MEMORY;
        } else if (errorCode & 0x200) {
            errorCode = CONT_ERR_VOICE_WORD;
        } else if (errorCode & 0xFF00) {
            errorCode = CONT_ERR_INVALID;
        }
    }

    return errorCode;
}
