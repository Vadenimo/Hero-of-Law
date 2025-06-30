#include "global.h"
#include "../../../../actor/_custom-1.0/common.h"

#define gISVDbgPrnAdrs ((ISVDbg*)0xB3FF0000)
#define ASCII_TO_U32(a, b, c, d) ((u32)((a << 24) | (b << 16) | (c << 8) | (d << 0)))

extern s32 osEPiWriteIo(OSPiHandle* handle, u32 devAddr, u32 data);
    asm("osEPiWriteIo = 0x80005800");

extern s32 osEPiReadIo(OSPiHandle* handle, u32 devAddr, u32* data);
    asm("osEPiReadIo = 0x80005630");

extern int _Printf(PrintCallback pfn, void* arg, const char* fmt, va_list ap);
    asm("_Printf = 0x800D1D00");
    
void* _is_proutSyncPrintf(void* arg, const char* str, u32 count);

//800dcf50
void is64Printf(const char* fmt, ...) 
{
    va_list args;
    va_start(args, fmt);

    _Printf(_is_proutSyncPrintf, NULL, fmt, args);

    va_end(args);
}


void* _is_proutSyncPrintf(void* arg, const char* str, u32 count) 
{
    u32 data;
    s32 pos;
    s32 start;
    s32 end;

    osEPiReadIo(*sISVHandle, (u32)&gISVDbgPrnAdrs->magic, &data);

    if (data != ASCII_TO_U32('I', 'S', '6', '4')) 
        return (void*)1;

    osEPiReadIo(*sISVHandle, (u32)&gISVDbgPrnAdrs->get, &data);
    pos = data;
    osEPiReadIo(*sISVHandle, (u32)&gISVDbgPrnAdrs->put, &data);
    start = data;
    end = start + count;

    if (end >= 0xFFE0) 
    {
        end -= 0xFFE0;
        if (pos < end || start < pos) 
            return (void*)1;
    } 
    else 
    {
        if (start < pos && pos < end)
            return (void*)1;
    }

    while (count) 
    {
        u32 addr = (u32)&gISVDbgPrnAdrs->data + (start & 0xFFFFFFC);
        s32 shift = ((3 - (start & 3)) * 8);

        if (*str) 
        {
            osEPiReadIo(*sISVHandle, addr, &data);
            osEPiWriteIo(*sISVHandle, addr, (*str << shift) | (data & ~(0xFF << shift)));

            start++;
            if (start >= 0xFFE0) 
                start -= 0xFFE0;
        }
        count--;
        str++;
    }

    osEPiWriteIo(*sISVHandle, (u32)&gISVDbgPrnAdrs->put, start);

    return (void*)1;
}