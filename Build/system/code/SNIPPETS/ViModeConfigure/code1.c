#include "global.h"

#define VI_CTRL_PIXEL_ADV_DEFAULT VI_CTRL_PIXEL_ADV(3)

#define VI_CTRL_TYPE_16             0x00002 /* [1:0] pixel size: 16 bit */
#define VI_CTRL_TYPE_32             0x00003 /* [1:0] pixel size: 32 bit */
#define VI_CTRL_GAMMA_DITHER_ON     0x00004 /* 2: default = on */
#define VI_CTRL_GAMMA_ON            0x00008 /* 3: default = on */
#define VI_CTRL_DIVOT_ON            0x00010 /* 4: default = on */
#define VI_CTRL_SERRATE_ON          0x00040 /* 6: on if interlaced */
#define VI_CTRL_ANTIALIAS_MASK      0x00300 /* [9:8] anti-alias mode */
#define VI_CTRL_ANTIALIAS_MODE_0    0x00000 /* Bit [9:8] anti-alias mode: AA enabled, resampling enabled, always fetch extra lines */
#define VI_CTRL_ANTIALIAS_MODE_1    0x00100 /* Bit [9:8] anti-alias mode: AA enabled, resampling enabled, fetch extra lines as-needed */
#define VI_CTRL_ANTIALIAS_MODE_2    0x00200 /* Bit [9:8] anti-alias mode: AA disabled, resampling enabled, operate as if everything is covered */
#define VI_CTRL_ANTIALIAS_MODE_3    0x00300 /* Bit [9:8] anti-alias mode: AA disabled, resampling disabled, replicate pixels */
#define VI_CTRL_PIXEL_ADV_MASK      0x0F000 /* [15:12] pixel advance mode */
#define VI_CTRL_PIXEL_ADV(n)        (((n) << 12) & VI_CTRL_PIXEL_ADV_MASK) /* Bit [15:12] pixel advance mode: Always 3 on N64 */
#define VI_CTRL_DITHER_FILTER_ON    0x10000 /* 16: dither-filter mode */

#define VI_STATE_MODE_SET           (1 << 0)
#define VI_STATE_XSCALE_SET         (1 << 1)
#define VI_STATE_YSCALE_FACTOR_SET  (1 << 2)
#define VI_STATE_CTRL_SET           (1 << 3)
#define VI_STATE_BUFFER_SET         (1 << 4)
#define VI_STATE_BLACK              (1 << 5)
#define VI_STATE_REPEATLINE         (1 << 6)
#define VI_STATE_FADE               (1 << 7)

#define VI_SCALE_MASK       0xFFF
#define VI_2_10_FPART_MASK  0x3FF
#define VI_SUBPIXEL_SH      0x10

#define BURST(hsync_width, color_width, vsync_width, color_start) \
   ((((u8)(hsync_width) & 0xFF) << 0) | \
    (((u8)(color_width) & 0xFF) << 8) | \
    (((u8)(vsync_width) & 0xF) << 16) | \
    (((u16)(color_start) & 0xFFF) << 20))
#define WIDTH(v) (v)
#define VSYNC(v) (v)
#define HSYNC(duration, leap) (((u16)(leap) << 16) | (u16)(duration))
#define LEAP(upper, lower) (((u16)(upper) << 16) | (u16)(lower))
#define START(start, end) (((u16)(start) << 16) | (u16)(end))

#define FTOFIX(val, i, f) ((u32)((val) * (f32)(1 << (f))) & ((1 << ((i) + (f))) - 1))
#define F210(val) FTOFIX(val, 2, 10)
#define SCALE(scaleup, off) (F210(1.0f / (f32)(scaleup)) | (F210((f32)(off)) << 16))

#define VCURRENT(v) (v)
#define ORIGIN(v) (v)
#define VINTR(v) (v)
#define HSTART(start, end) START(start, end)
#define VSTART(start, end) START(start, end)

void ViMode_Configure(ViMode* viMode, s32 type, s32 tvType, s32 loRes, s32 antialiasOff, s32 modeN, s32 fb16Bit, s32 width, s32 height, s32 leftAdjust, s32 rightAdjust, s32 upperAdjust, s32 lowerAdjust) 
{
    s32 hiRes;
    s32 antialiasOn;
    s32 modeF;
    s32 fb32Bit;
    s32 hiResDeflicker; // deflickered interlacing
    s32 hiResInterlaced;
    s32 loResDeinterlaced;
    s32 loResInterlaced;
    s32 modeLAN1; // L=(lo res) A=(antialias)     N=(deinterlace)        1=(16-bit)
    s32 modeLPN2; // L=(lo res) P=(point-sampled) N=(deinterlace)        2=(32-bit)
    s32 modeHPN2; // H=(hi res) P=(point-sampled) N=(normal interlacing) 2=(32-bit)
    s32 yScaleLo;
    s32 yScaleHiEvenField;
    s32 yScaleHiOddField;

    hiRes = !loRes;
    antialiasOn = !antialiasOff;
    modeF = !modeN;
    fb32Bit = !fb16Bit;

    hiResDeflicker = hiRes && modeF;
    hiResInterlaced = hiRes && modeN;
    loResDeinterlaced = loRes && modeN;
    loResInterlaced = loRes && modeF;

    modeLAN1 = loRes && antialiasOn && modeN && fb16Bit;
    modeLPN2 = loRes && antialiasOff && modeN && fb32Bit;
    modeHPN2 = hiRes && antialiasOff && modeN && fb32Bit;

    upperAdjust &= ~1;
    lowerAdjust &= ~1;

    yScaleLo =
        (hiResDeflicker ? 2 : 1) * ((height << 11) / (SCREEN_HEIGHT * 2 + lowerAdjust - upperAdjust) / (loRes ? 1 : 2));

    yScaleHiEvenField = modeF ? (loResInterlaced ? (F210(0.25) << 16) : (F210(0.5) << 16)) : 0;
    yScaleHiOddField = modeF ? (loResInterlaced ? (F210(0.75) << 16) : (F210(0.5) << 16)) : 0;

    viMode->customViMode.type = type;
    viMode->customViMode.comRegs.ctrl = VI_CTRL_PIXEL_ADV(3) | VI_CTRL_GAMMA_ON | VI_CTRL_GAMMA_DITHER_ON |
                                        (!loResDeinterlaced ? VI_CTRL_SERRATE_ON : 0) |
                                        (antialiasOn ? VI_CTRL_DIVOT_ON : 0) |
                                        (fb32Bit ? VI_CTRL_TYPE_32 : VI_CTRL_TYPE_16);

    if (modeLAN1) 
    {
        // Anti-aliased, fetch extra lines as-needed
        viMode->customViMode.comRegs.ctrl |= VI_CTRL_ANTIALIAS_MODE_1;
    } 
    else if (modeLPN2 | modeHPN2) 
    {
        // Point-sampled, resampling disabled
        viMode->customViMode.comRegs.ctrl |= VI_CTRL_ANTIALIAS_MODE_3;
    } else 
    {
        if (antialiasOff) 
        {
            // Point-sampled, resampling enabled
            viMode->customViMode.comRegs.ctrl |= VI_CTRL_ANTIALIAS_MODE_2;
        }
        else 
        {
            // Anti-aliased, always fetch extra lines
            viMode->customViMode.comRegs.ctrl |= VI_CTRL_ANTIALIAS_MODE_0;
        }
    }

    viMode->customViMode.comRegs.width = width * (hiResInterlaced ? 2 : 1);

    if (tvType == OS_TV_NTSC) 
    {
        viMode->customViMode.comRegs.burst = BURST(57, 34, 5, 62);
        viMode->customViMode.comRegs.vSync = VSYNC(524);
        viMode->customViMode.comRegs.hSync = HSYNC(3093, 0);
        viMode->customViMode.comRegs.leap = LEAP(3093, 3093);
        viMode->customViMode.comRegs.hStart = HSTART(108, 748);
        viMode->customViMode.fldRegs[0].vStart = START(37, 511);
        viMode->customViMode.fldRegs[0].vBurst = BURST(4, 2, 14, 0);
    }
    // Changed this to act like FPAL 
    else if (tvType == OS_TV_PAL) 
    {
        viMode->customViMode.comRegs.burst = BURST(58, 35, 4, 64);
        viMode->customViMode.comRegs.vSync = VSYNC(624);
        viMode->customViMode.comRegs.hSync = HSYNC(3177, 21);
        viMode->customViMode.comRegs.leap = LEAP(3183, 3182);
        viMode->customViMode.comRegs.hStart = HSTART(128, 768);
        viMode->customViMode.fldRegs[0].vStart = VSTART(47, 617);
        viMode->customViMode.fldRegs[0].vBurst = BURST(107, 2, 9, 0);
    } 
    else if (tvType == OS_TV_MPAL) 
    {
        viMode->customViMode.comRegs.burst = BURST(57, 30, 5, 70);
        viMode->customViMode.comRegs.vSync = VSYNC(524);
        viMode->customViMode.comRegs.hSync = HSYNC(3088, 0);
        viMode->customViMode.comRegs.leap = LEAP(3100, 3100);
        viMode->customViMode.comRegs.hStart = HSTART(108, 748);
        viMode->customViMode.fldRegs[0].vStart = START(37, 511);
        viMode->customViMode.fldRegs[0].vBurst = BURST(4, 2, 14, 0);
    }

    viMode->customViMode.fldRegs[1].vStart = viMode->customViMode.fldRegs[0].vStart;

    viMode->customViMode.comRegs.hStart += (leftAdjust << 16) + (s16)rightAdjust;
    viMode->customViMode.fldRegs[0].vStart += (upperAdjust << 16) + (s16)lowerAdjust;
    viMode->customViMode.fldRegs[1].vStart += (upperAdjust << 16) + (s16)lowerAdjust;

    viMode->customViMode.fldRegs[1].vBurst = viMode->customViMode.fldRegs[0].vBurst;

    if (loResDeinterlaced) 
    {
        viMode->customViMode.comRegs.vSync++;
        if (tvType == OS_TV_MPAL)
            viMode->customViMode.comRegs.hSync += HSYNC(1, 4);

        if (tvType == OS_TV_MPAL)
            viMode->customViMode.comRegs.leap += LEAP(-4, -2);
    } 
    else 
    {
        viMode->customViMode.fldRegs[0].vStart += START(-3, -2);
        if (tvType == OS_TV_MPAL)
            viMode->customViMode.fldRegs[0].vBurst += BURST(-2, -1, 12, -1);

        if (tvType == OS_TV_PAL)
            viMode->customViMode.fldRegs[1].vBurst += BURST(-2, -1, 2, 0);
    }

    viMode->customViMode.comRegs.xScale = (width << 10) / (SCREEN_WIDTH * 2 + rightAdjust - leftAdjust);
    viMode->customViMode.comRegs.vCurrent = VCURRENT(0);

    viMode->customViMode.fldRegs[0].origin = ORIGIN(width * 2 * (fb16Bit ? 1 : 2));
    viMode->customViMode.fldRegs[1].origin = ORIGIN(width * 2 * (fb16Bit ? 1 : 2) * (loRes ? 1 : 2));

    viMode->customViMode.fldRegs[0].yScale = yScaleLo | yScaleHiEvenField;
    viMode->customViMode.fldRegs[1].yScale = yScaleLo | yScaleHiOddField;

    viMode->customViMode.fldRegs[0].vIntr = VINTR(2);
    viMode->customViMode.fldRegs[1].vIntr = VINTR(2);
}