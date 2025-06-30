#include "global.h"
#include "../../../../actor/_custom-1.0/common.h"

s32 DmaMgr_AudioDmaHandler(OSPiHandle* pihandle, OSIoMesg* mb, s32 direction);

extern uintptr_t sSysCfbEnd;

void AudioMgr_ThreadEntry(void* arg) {
    AudioMgr* audioMgr = (AudioMgr*)arg;
    IrqMgrClient irqClient;
    s16* msg = NULL;

    // Initialize audio driver
    AudioLoad_Init((u8*)sSysCfbEnd, AudioHeapSize);
    AudioLoad_SetDmaHandler(DmaMgr_AudioDmaHandler);
    Audio_InitSound();

    // Fill init queue to signal that the audio driver is initialized
    osSendMesg(&audioMgr->lockQueue, NULL, OS_MESG_BLOCK);

    IrqMgr_AddClient(audioMgr->irqMgr, &irqClient, &audioMgr->interruptQueue);

    // Spin waiting for events
    for (;;) {
        osRecvMesg(&audioMgr->interruptQueue, (OSMesg*)&msg, OS_MESG_BLOCK);

        switch (*msg) {
            case OS_SC_RETRACE_MSG:
                AudioMgr_HandleRetrace(audioMgr);

                // Empty the interrupt queue
                while (!MQ_IS_EMPTY(&audioMgr->interruptQueue)) {
                    osRecvMesg(&audioMgr->interruptQueue, (OSMesg*)&msg, OS_MESG_BLOCK);

                    switch (*msg) {
                        case OS_SC_RETRACE_MSG:
                            // Don't process a retrace more than once in quick succession
                            break;

                        case OS_SC_PRE_NMI_MSG:
                            // Always handle Pre-NMI
                            AudioMgr_HandlePreNMI(audioMgr);
                            break;
                    }
                }
                break;

            case OS_SC_PRE_NMI_MSG:
                AudioMgr_HandlePreNMI(audioMgr);
                break;
        }
    }
}