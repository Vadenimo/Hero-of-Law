#ifndef COMMON_H
#define COMMON_H

#define AudioHeapSize 0x3C000

u32* emuIdentifier = (u32*)0x80198948;
u32* hzChoice = (u32*)0x80198950;
u32* tpakBeingUsed = (u32*)0x80198954;

#define GCN_CHECK_MAGIC 0x47434E45
#define WII_CHECK_MAGIC 0x5245564F
#define WIIU_CHECK_MAGIC 0x43414645

OSPiHandle** sISVHandle = (OSPiHandle**)0x8019894C;

#endif