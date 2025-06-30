/**
 * @file tpak.c
 * @brief Transfer Pak interface
 * @ingroup transferpak
 */

#include "../include/tpak.h"
#include "../../is64Printf.h"

OSTimer tpakTimer;
OSMesg tpakTimerMesg;
OSMesgQueue tpakTimerQueue __attribute__((aligned(8)));

int tpak_set_value(OSMesgQueue* serialEventQueue, int controller, u16 address, u8 value)
{
    u8 block[TPAK_BLOCK_SIZE];
    Lib_MemSet(block, TPAK_BLOCK_SIZE, value);
    return __osContRamWrite(serialEventQueue, controller, CONT_BLOCKS(address), block, 0);
}

int tpak_init(OSMesgQueue* serialEventQueue, int controller)
{
    int result = 0;
    
    osCreateMesgQueue(&tpakTimerQueue, &tpakTimerMesg, 1);
   
    if (gPadMgr.padStatus[0].errno == 0 &&
        gPadMgr.padStatus[0].type == CONT_TYPE_NORMAL &&
        gPadMgr.pakType[0] == CONT_PAK_OTHER)
    {
        result = tpak_set_power(serialEventQueue, controller, false);
        result = tpak_set_power(serialEventQueue, controller, true);
        if (result) return result;

        result = tpak_set_access(serialEventQueue, controller, true);
        if (result) return result;
        
        u8 status = tpak_get_status(serialEventQueue, controller);
    
        if (!(status & TPAK_STATUS_READY)) return TPAK_ERROR_UNKNOWN_BEHAVIOUR;
        
        return 0;
    }
    else
        return TPAK_ERROR_NO_TPAK;
}

int tpak_set_access(OSMesgQueue* serialEventQueue, int controller, bool access_state)
{
    u8 value = access_state ? 1 : 0;
    return tpak_set_value(serialEventQueue, controller, TPAK_ADDRESS_STATUS, value);
}

int tpak_set_power(OSMesgQueue* serialEventQueue, int controller, bool power_state)
{
    // Check if power is already on or off and do nothing if so.
    if ((tpak_get_status(serialEventQueue, controller) & TPAK_STATUS_POWERED) == power_state)
        return 0;

    u8 value = power_state ? TPAK_POWER_ON : TPAK_POWER_OFF;
    u8 ret = tpak_set_value(serialEventQueue, controller, TPAK_ADDRESS_POWER, value);
   
    if (power_state)
    {
        // Wait for power to stabilize.
        osSetTimer(&tpakTimer, OS_USEC_TO_CYCLES(120000), 0, &tpakTimerQueue, &tpakTimerMesg);
        osRecvMesg(&tpakTimerQueue, NULL, OS_MESG_BLOCK);  
    }
    
    return ret;
}

int tpak_open(OSMesgQueue* serialEventQueue, int controller)
{
    int ret = tpak_set_power(serialEventQueue, controller, true);
    
    if (!ret)
        ret = tpak_set_access(serialEventQueue, controller, true);
    
    return ret;
}

int tpak_set_bank(OSMesgQueue* serialEventQueue, int controller, int bank)
{
    return tpak_set_value(serialEventQueue, controller, TPAK_ADDRESS_BANK, bank);
}

u8 tpak_get_status(OSMesgQueue* serialEventQueue, int controller)
{
    u8 block[TPAK_BLOCK_SIZE];
    __osContRamRead(serialEventQueue, controller, CONT_BLOCKS(TPAK_ADDRESS_STATUS), block);
    return block[0];
}

int tpak_get_cartridge_header(OSMesgQueue* serialEventQueue, int controller, gameboy_cartridge_header* header)
{
    // We're interested in 0x0000 - 0x3FFF of gb space.
    tpak_set_bank(serialEventQueue, controller, 0);
    // Header starts at 0x0100
    const u16 address = 0x0100;

    return tpak_read(serialEventQueue, controller, address, (u8*) header, sizeof(gameboy_cartridge_header));
}

int tpak_write(OSMesgQueue* serialEventQueue, int controller, u16 address, u8* data, u16 size)
{
    if (controller < 0 || controller > 3 || size % TPAK_BLOCK_SIZE || address % TPAK_BLOCK_SIZE)
    {
        return TPAK_ERROR_INVALID_ARGUMENT;
    }

    u16 adjusted_address = (address % TPAK_BANK_SIZE) + TPAK_ADDRESS_DATA;
    u16 end_address = address + size;
    if (end_address < address)
    {
        return TPAK_ERROR_ADDRESS_OVERFLOW;
    }

    u8* cursor = data;

    int bank = address / TPAK_BANK_SIZE;
    tpak_set_bank(serialEventQueue, controller, bank);

    while(address < end_address)
    {
        // Check if we need to change the bank.
        if (address / TPAK_BANK_SIZE > bank) {
            bank = address / TPAK_BANK_SIZE;
            tpak_set_bank(serialEventQueue, controller, bank);
            adjusted_address = TPAK_ADDRESS_DATA;
        }
        
        __osContRamWrite(serialEventQueue, controller, CONT_BLOCKS(adjusted_address), cursor, 0);
        
        address += TPAK_BLOCK_SIZE;
        cursor += TPAK_BLOCK_SIZE;
        adjusted_address += TPAK_BLOCK_SIZE;
    }

    return 0;
}

int tpak_read(OSMesgQueue* serialEventQueue, int controller, u16 address, u8* buffer, u16 size)
{
    if (controller < 0 || controller > 3 || size % TPAK_BLOCK_SIZE || address % TPAK_BLOCK_SIZE)
    {
        return TPAK_ERROR_INVALID_ARGUMENT;
    }

    u16 adjusted_address = (address % TPAK_BANK_SIZE) + TPAK_ADDRESS_DATA;
    u16 end_address = address + size;
    if (end_address < address)
    {
        return TPAK_ERROR_ADDRESS_OVERFLOW;
    }

    u8* cursor = buffer;

    int bank = address / TPAK_BANK_SIZE;
    tpak_set_bank(serialEventQueue, controller, bank);

    while(address < end_address)
    {
        // Check if we need to change the bank.
        if (address / TPAK_BANK_SIZE > bank) {
            bank = address / TPAK_BANK_SIZE;
            tpak_set_bank(serialEventQueue, controller, bank);
            adjusted_address = TPAK_ADDRESS_DATA;
        }
        
        __osContRamRead(serialEventQueue, controller, CONT_BLOCKS(adjusted_address), cursor);

        address += TPAK_BLOCK_SIZE;
        cursor += TPAK_BLOCK_SIZE;
        adjusted_address += TPAK_BLOCK_SIZE;
    }

    return 0;
}

bool tpak_check_header(OSMesgQueue* serialEventQueue, int controller, struct gameboy_cartridge_header* header)
{
    u8 sum = 0;
    u8* data = (u8*) header;

    // sum values from 0x0134 (title) to 0x014C (version number)
    const u8 start = 0x34;
    const u8 end = 0x4C;
    for (u8 i = start; i <= end; i++) {
        sum = sum - data[i] - 1;
    }

    return sum == header->header_checksum;
}
