#include "global.h"
#include "../../../../actor/_custom-1.0/common.h"

void PadMgr_UpdateRumble(PadMgr* padMgr)
{
    static u32 sRumbleErrorCount = 0;
    static u32 sRumbleUpdateCounter;
    s32 i;
    s32 ret;
    OSMesgQueue* serialEventQueue = PadMgr_AcquireSerialEventQueue(padMgr);
    s32 triedRumbleComm = false;

    for (i = 0; i < MAXCONTROLLERS; i++) 
    {
        if (padMgr->ctrlrIsConnected[i]) 
        {
            if (padMgr->padStatus[i].status & CONT_CARD_ON) 
            {
                if (padMgr->pakType[i] == CONT_PAK_RUMBLE) 
                {
                    if (padMgr->rumbleEnable[i]) 
                    {
                        if (padMgr->rumbleTimer[i] < 3) 
                        {
                            if (osMotorStart(&padMgr->rumblePfs[i]) != 0)
                                padMgr->pakType[i] = CONT_PAK_NONE;
                            else 
                                padMgr->rumbleTimer[i] = 3;
                            
                            triedRumbleComm = true;
                        }
                    } 
                    else 
                    {
                        if (padMgr->rumbleTimer[i] != 0) 
                        {
                            if (osMotorStop(&padMgr->rumblePfs[i]) != 0)
                                padMgr->pakType[i] = CONT_PAK_NONE;
                            else
                                padMgr->rumbleTimer[i]--;

                            triedRumbleComm = true;
                        }
                    }
                }
            } 
            else 
            {
                if (padMgr->pakType[i] != CONT_PAK_NONE) 
                    padMgr->pakType[i] = CONT_PAK_NONE;
            }
        }
    }

    if (!triedRumbleComm) 
    {
        i = sRumbleUpdateCounter % MAXCONTROLLERS;
        
        if (*tpakBeingUsed && i == 0)
            padMgr->pakType[i] = CONT_PAK_OTHER;
        else
        {
            if (padMgr->ctrlrIsConnected[i] && (padMgr->padStatus[i].status & CONT_CARD_ON) && padMgr->pakType[i] != CONT_PAK_RUMBLE) 
            {
                ret = osMotorInit(serialEventQueue, &padMgr->rumblePfs[i], i);

                if (ret == 0) 
                {
                    padMgr->pakType[i] = CONT_PAK_RUMBLE;
                    osMotorStart(&padMgr->rumblePfs[i]);
                    osMotorStop(&padMgr->rumblePfs[i]);
                } 
                else if (ret == PFS_ERR_DEVICE)
                    padMgr->pakType[i] = CONT_PAK_OTHER;
                else if (ret == PFS_ERR_CONTRFAIL)
                {
                }
            }
        }
    }
    
    sRumbleUpdateCounter++;
    PadMgr_ReleaseSerialEventQueue(padMgr, serialEventQueue);
}