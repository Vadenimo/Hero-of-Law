#include "global.h"
#include "../../../../actor/_custom-1.0/common.h"

#define gISVDbgPrnAdrs ((ISVDbg*)0xB3FF0000)
#define ASCII_TO_U32(a, b, c, d) ((u32)((a << 24) | (b << 16) | (c << 8) | (d << 0)))

extern s32 osEPiWriteIo(OSPiHandle* handle, u32 devAddr, u32 data);
    asm("osEPiWriteIo = 0x80005800");

extern OSPiHandle* osCartRomInit(void);
    asm("osCartRomInit = 0x80005680");

//80078290 + 0x1C4
void _isPrintfInit() 
{
    *sISVHandle = osCartRomInit();
    osEPiWriteIo(*sISVHandle, (u32)&gISVDbgPrnAdrs->put, 0);
    osEPiWriteIo(*sISVHandle, (u32)&gISVDbgPrnAdrs->get, 0);
    osEPiWriteIo(*sISVHandle, (u32)&gISVDbgPrnAdrs->magic, ASCII_TO_U32('I', 'S', '6', '4'));
}