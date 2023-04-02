/*  Picasso96.h -- include File
 *  (C) Copyright 1996-98 Alexander Kneer & Tobias Abt
 *  All Rights Reserved.
 */
/************************************************************************/
#ifndef LIBRARIES_PICASSO96_H
#define LIBRARIES_PICASSO96_H
/************************************************************************/
/* includes
 */
#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef EXEC_NODES_H
#include <exec/nodes.h>
#endif

#ifndef UTILITY_TAGITEM_H
#include <utility/tagitem.h>
#endif

/************************************************************************/
/* This is the name of the library
 */
#define P96NAME "Picasso96API.library"

/************************************************************************/
/* Types for RGBFormat used
 */
typedef enum
{
    RGBFB_NONE,     /* no valid RGB format (should not happen) */
    RGBFB_CLUT,     /* palette mode, set colors when opening screen using
                            tags or use SetRGB32/LoadRGB32(...) */
    RGBFB_R8G8B8,   /* TrueColor RGB (8 bit each) */
    RGBFB_B8G8R8,   /* TrueColor BGR (8 bit each) */
    RGBFB_R5G6B5PC, /* HiColor16 (5 bit R, 6 bit G, 5 bit B),
                                format: gggbbbbbrrrrrggg */
    RGBFB_R5G5B5PC, /* HiColor15 (5 bit each), format: gggbbbbb0rrrrrgg */
    RGBFB_A8R8G8B8, /* 4 Byte TrueColor ARGB (A unused alpha channel) */
    RGBFB_A8B8G8R8, /* 4 Byte TrueColor ABGR (A unused alpha channel) */
    RGBFB_R8G8B8A8, /* 4 Byte TrueColor RGBA (A unused alpha channel) */
    RGBFB_B8G8R8A8, /* 4 Byte TrueColor BGRA (A unused alpha channel) */
    RGBFB_R5G6B5,   /* HiColor16 (5 bit R, 6 bit G, 5 bit B),
                            format: rrrrrggggggbbbbb */
    RGBFB_R5G5B5,   /* HiColor15 (5 bit each), format: 0rrrrrgggggbbbbb */
    RGBFB_B5G6R5PC, /* HiColor16 (5 bit R, 6 bit G, 5 bit B),
                                format: gggrrrrrbbbbbggg */
    RGBFB_B5G5R5PC, /* HiColor15 (5 bit each), format: gggrrrrr0bbbbbbgg */

    /* By now, the following formats are for use with a hardware window only
        (bitmap operations may be implemented incompletely) */

    RGBFB_YUV422CGX,  /* 2 Byte TrueColor YUV (CCIR recommendation CCIR601).
                              Each two-pixel unit is stored as one longword
                              containing luminance (Y) for each of the two pixels,
                              and chrominance (U,V) for alternate pixels.
                              The missing chrominance values are generated by
                              interpolation. (Y0-V0-Y1-U0) */
    RGBFB_YUV411,     /* 1 Byte TrueColor ACCUPAK. Four adjacent pixels form
                              a packet of 5 bits Y (luminance) each pixel and 6 bits
                              U and V (chrominance) shared by the four pixels */
    RGBFB_YUV411PC,   /* 1 Byte TrueColor ACCUPAK byte swapped (1234 -> 4321) */
    RGBFB_YUV422,     /* 2 Byte TrueColor YUV (CCIR recommendation CCIR601).
                              Each two-pixel unit is stored as one longword
                              containing luminance (Y) for each of the two pixels,
                              and chrominance (U,V) for alternate pixels.
                              The missing chrominance values are generated by
                              interpolation. (Y1-U0-Y0-V0) */
    RGBFB_YUV422PC,   /* 2 Byte TrueColor CCIR601 byte swapped (V0-Y0-U0-Y1) */
    RGBFB_YUV422PA,   /* 2 Byte TrueColor CCIR601 for use with YUV12 planar
                                  assist mode on Cirrus Logic base graphics chips.
                                  (Y0-Y1-V0-U0) */
    RGBFB_YUV422PAPC, /* 2 Byte TrueColor YUV12 byte swapped (U0-V0-Y1-Y0) */

    RGBFB_MaxFormats
} RGBFTYPE;

// legacy
#define RGBFB_Y4U2V2 RGBFB_YUV422CGX
#define RGBFB_Y4U1V1 RGBFB_YUV411

#define RGBFF_NONE (1 << RGBFB_NONE)
#define RGBFF_CLUT (1 << RGBFB_CLUT)
#define RGBFF_R8G8B8 (1 << RGBFB_R8G8B8)
#define RGBFF_B8G8R8 (1 << RGBFB_B8G8R8)
#define RGBFF_R5G6B5PC (1 << RGBFB_R5G6B5PC)
#define RGBFF_R5G5B5PC (1 << RGBFB_R5G5B5PC)
#define RGBFF_A8R8G8B8 (1 << RGBFB_A8R8G8B8)
#define RGBFF_A8B8G8R8 (1 << RGBFB_A8B8G8R8)
#define RGBFF_R8G8B8A8 (1 << RGBFB_R8G8B8A8)
#define RGBFF_B8G8R8A8 (1 << RGBFB_B8G8R8A8)
#define RGBFF_R5G6B5 (1 << RGBFB_R5G6B5)
#define RGBFF_R5G5B5 (1 << RGBFB_R5G5B5)
#define RGBFF_B5G6R5PC (1 << RGBFB_B5G6R5PC)
#define RGBFF_B5G5R5PC (1 << RGBFB_B5G5R5PC)
#define RGBFF_YUV422CGX (1 << RGBFB_YUV422CGX)
#define RGBFF_YUV411 (1 << RGBFB_YUV411)
#define RGBFF_YUV411PC (1 << RGBFB_YUV411PC)
#define RGBFF_YUV422 (1 << RGBFB_YUV422)
#define RGBFF_YUV422PC (1 << RGBFB_YUV422PC)
#define RGBFF_YUV422PA (1 << RGBFB_YUV422PA)
#define RGBFF_YUV422PAPC (1 << RGBFB_YUV422PAPC)

// legacy
#define RGBFF_Y4U2V2 RGBFF_YUV422CGX
#define RGBFF_Y4U1V1 RGBFF_YUV411

#define RGBFF_HICOLOR (RGBFF_R5G6B5PC | RGBFF_R5G5B5PC | RGBFF_R5G6B5 | RGBFF_R5G5B5 | RGBFF_B5G6R5PC | RGBFF_B5G5R5PC)
#define RGBFF_TRUECOLOR (RGBFF_R8G8B8 | RGBFF_B8G8R8)
#define RGBFF_TRUEALPHA (RGBFF_A8R8G8B8 | RGBFF_A8B8G8R8 | RGBFF_R8G8B8A8 | RGBFF_B8G8R8A8)

/************************************************************************/
/* Flags for p96AllocBitMap
 */
#define BMF_USERPRIVATE                                                      \
    (0x8000) /* private user bitmap that will never                          \
be put to a board, but may be used as a temporary render buffer and accessed \
with OS blit functions, too. Bitmaps allocated with this flag do not need to \
be locked. */

/************************************************************************/
/* Attributes for p96GetBitMapAttr
 */
enum
{
    P96BMA_WIDTH,
    P96BMA_HEIGHT,
    P96BMA_DEPTH,
    P96BMA_MEMORY,
    P96BMA_BYTESPERROW,
    P96BMA_BYTESPERPIXEL,
    P96BMA_BITSPERPIXEL,
    P96BMA_RGBFORMAT,
    P96BMA_ISP96,
    P96BMA_ISONBOARD,
    P96BMA_BOARDMEMBASE,
    P96BMA_BOARDIOBASE,
    P96BMA_BOARDMEMIOBASE
};

/************************************************************************/
/* Attributes for p96GetModeIDAttr
 */
enum
{
    P96IDA_WIDTH,
    P96IDA_HEIGHT,
    P96IDA_DEPTH,
    P96IDA_BYTESPERPIXEL,
    P96IDA_BITSPERPIXEL,
    P96IDA_RGBFORMAT,
    P96IDA_ISP96,
    P96IDA_BOARDNUMBER,
    P96IDA_STDBYTESPERROW,
    P96IDA_BOARDNAME,
    P96IDA_COMPATIBLEFORMATS,
    P96IDA_VIDEOCOMPATIBLE,
    P96IDA_PABLOIVCOMPATIBLE,
    P96IDA_PALOMAIVCOMPATIBLE
};

/************************************************************************/
/* Tags for p96BestModeIDTagList
 */
#define P96BIDTAG_Dummy (TAG_USER + 96)

#define P96BIDTAG_FormatsAllowed (P96BIDTAG_Dummy + 0x0001)
#define P96BIDTAG_FormatsForbidden (P96BIDTAG_Dummy + 0x0002)
#define P96BIDTAG_NominalWidth (P96BIDTAG_Dummy + 0x0003)
#define P96BIDTAG_NominalHeight (P96BIDTAG_Dummy + 0x0004)
#define P96BIDTAG_Depth (P96BIDTAG_Dummy + 0x0005)
#define P96BIDTAG_VideoCompatible (P96BIDTAG_Dummy + 0x0006)
#define P96BIDTAG_PabloIVCompatible (P96BIDTAG_Dummy + 0x0007)
#define P96BIDTAG_PalomaIVCompatible (P96BIDTAG_Dummy + 0x0008)

/************************************************************************/
/* Tags for p96RequestModeIDTagList
 */

#define P96MA_Dummy (TAG_USER + 0x10000 + 96)

#define P96MA_MinWidth (P96MA_Dummy + 0x0001)
#define P96MA_MinHeight (P96MA_Dummy + 0x0002)
#define P96MA_MinDepth (P96MA_Dummy + 0x0003)
#define P96MA_MaxWidth (P96MA_Dummy + 0x0004)
#define P96MA_MaxHeight (P96MA_Dummy + 0x0005)
#define P96MA_MaxDepth (P96MA_Dummy + 0x0006)
#define P96MA_DisplayID (P96MA_Dummy + 0x0007)
#define P96MA_FormatsAllowed (P96MA_Dummy + 0x0008)
#define P96MA_FormatsForbidden (P96MA_Dummy + 0x0009)
#define P96MA_WindowTitle (P96MA_Dummy + 0x000a)
#define P96MA_OKText (P96MA_Dummy + 0x000b)
#define P96MA_CancelText (P96MA_Dummy + 0x000c)
#define P96MA_Window (P96MA_Dummy + 0x000d)
#define P96MA_PubScreenName (P96MA_Dummy + 0x000e)
#define P96MA_Screen (P96MA_Dummy + 0x000f)
#define P96MA_VideoCompatible (P96MA_Dummy + 0x0010)
#define P96MA_PabloIVCompatible (P96MA_Dummy + 0x0011)
#define P96MA_PalomaIVCompatible (P96MA_Dummy + 0x0012)

/************************************************************************/
/* Tags for p96OpenScreenTagList
 */

#define P96SA_Dummy (TAG_USER + 0x20000 + 96)
#define P96SA_Left (P96SA_Dummy + 0x0001)
#define P96SA_Top (P96SA_Dummy + 0x0002)
#define P96SA_Width (P96SA_Dummy + 0x0003)
#define P96SA_Height (P96SA_Dummy + 0x0004)
#define P96SA_Depth (P96SA_Dummy + 0x0005)
#define P96SA_DetailPen (P96SA_Dummy + 0x0006)
#define P96SA_BlockPen (P96SA_Dummy + 0x0007)
#define P96SA_Title (P96SA_Dummy + 0x0008)
#define P96SA_Colors (P96SA_Dummy + 0x0009)
#define P96SA_ErrorCode (P96SA_Dummy + 0x000a)
#define P96SA_Font (P96SA_Dummy + 0x000b)
#define P96SA_SysFont (P96SA_Dummy + 0x000c)
#define P96SA_Type (P96SA_Dummy + 0x000d)
#define P96SA_BitMap (P96SA_Dummy + 0x000e)
#define P96SA_PubName (P96SA_Dummy + 0x000f)
#define P96SA_PubSig (P96SA_Dummy + 0x0010)
#define P96SA_PubTask (P96SA_Dummy + 0x0011)
#define P96SA_DisplayID (P96SA_Dummy + 0x0012)
#define P96SA_DClip (P96SA_Dummy + 0x0013)
#define P96SA_ShowTitle (P96SA_Dummy + 0x0014)
#define P96SA_Behind (P96SA_Dummy + 0x0015)
#define P96SA_Quiet (P96SA_Dummy + 0x0016)
#define P96SA_AutoScroll (P96SA_Dummy + 0x0017)
#define P96SA_Pens (P96SA_Dummy + 0x0018)
#define P96SA_SharePens (P96SA_Dummy + 0x0019)
#define P96SA_BackFill (P96SA_Dummy + 0x001a)
#define P96SA_Colors32 (P96SA_Dummy + 0x001b)
#define P96SA_VideoControl (P96SA_Dummy + 0x001c)
#define P96SA_RGBFormat (P96SA_Dummy + 0x001d)
#define P96SA_NoSprite (P96SA_Dummy + 0x001e)
#define P96SA_NoMemory (P96SA_Dummy + 0x001f)
#define P96SA_RenderFunc (P96SA_Dummy + 0x0020)
#define P96SA_SaveFunc (P96SA_Dummy + 0x0021)
#define P96SA_UserData (P96SA_Dummy + 0x0022)
#define P96SA_Alignment (P96SA_Dummy + 0x0023)
#define P96SA_FixedScreen (P96SA_Dummy + 0x0024)
#define P96SA_Exclusive (P96SA_Dummy + 0x0025)
#define P96SA_ConstantBytesPerRow (P96SA_Dummy + 0x0026)
#define P96SA_ConstantByteSwapping (P96SA_Dummy + 0x0027)

/************************************************************************/
/*
 */

#define MODENAMELENGTH 48

struct P96Mode
{
    struct Node Node;
    char Description[MODENAMELENGTH];
    UWORD Width;
    UWORD Height;
    UWORD Depth;
    ULONG DisplayID;
};

/************************************************************************/
/* Structure to describe graphics data
 *
 * short description of the entries:
 * Memory:        pointer to graphics data
 * BytesPerRow:   distance in bytes between one pixel and its neighbour up
 *                or down.
 * pad:           private, not used.
 * RGBFormat:     RGBFormat of the data.
 */

struct RenderInfo
{
    APTR Memory;
    WORD BytesPerRow;
    WORD pad;
    RGBFTYPE RGBFormat;
};

/************************************************************************/
/* Structure for p96WriteTrueColorData() and p96ReadTrueColorData()
 *
 * short description of the entries:
 * PixelDistance: distance in bytes between the red (must be the same as
 *                for the green or blue) component of one pixel and its
 *                next neighbour to the left or right.
 * BytesPerRow:   distance in bytes between the red (must be the same as
 *                for the green or blue) component of one pixel and its
 *                next neighbour up or down.
 * RedData:       pointer to the red component of the upper left pixel.
 * GreenData, BlueData: the same as above.
 *
 * examples (for an array width of 640 pixels):
 * a) separate arrays for each color:
 *    { 1, 640, red, green, blue };
 * b) plain 24 bit RGB data:
 *    { 3, 640*3, array, array+1, array+2 };
 * c) 24 bit data, arranged as ARGB:
 *    { 4, 640*4, array+1, array+2, array+3 };
 */

struct TrueColorInfo
{
    ULONG PixelDistance, BytesPerRow;
    UBYTE *RedData, *GreenData, *BlueData;
};

/************************************************************************/
/* Tags for PIPs
 */

#define P96PIP_Dummy (TAG_USER + 0x30000 + 96)
#define P96PIP_SourceFormat (P96PIP_Dummy + 1) /* RGBFTYPE (I) */
#define P96PIP_SourceBitMap (P96PIP_Dummy + 2) /* struct BitMap * (G) */
#define P96PIP_SourceRPort (P96PIP_Dummy + 3)  /* struct RastPort * (G) */
#define P96PIP_SourceWidth (P96PIP_Dummy + 4)  /* ULONG (I) */
#define P96PIP_SourceHeight (P96PIP_Dummy + 5) /* ULONG (I) */
#define P96PIP_Type (P96PIP_Dummy + 6)         /* ULONG (I) default: PIPT_MemoryWindow */
#define P96PIP_ErrorCode (P96PIP_Dummy + 7)    /* LONG* (I) */
#define P96PIP_Brightness (P96PIP_Dummy + 8)   /* ULONG (IGS) default: 0 */
#define P96PIP_Left (P96PIP_Dummy + 9)         /* ULONG (I) default: 0 */
#define P96PIP_Top (P96PIP_Dummy + 10)         /* ULONG (I) default: 0 */
#define P96PIP_Width (P96PIP_Dummy + 11)       /* ULONG (I) default: inner width of window */
#define P96PIP_Height (P96PIP_Dummy + 12)      /* ULONG (I) default: inner height of window */
#define P96PIP_Relativity (P96PIP_Dummy + 13)  /* ULONG (I) default: PIPRel_Width|PIPRel_Height */
#define P96PIP_Colors                                                 \
    (P96PIP_Dummy + 14) /* struct ColorSpec * (IS)                    \
                         * ti_Data is an array of struct ColorSpec,   \
                         * terminated by ColorIndex = -1.  Specifies  \
                         * initial screen palette colors.             \
                         * Also see P96PIP_Colors32.                  \
                         * This only works with CLUT PIPs on non-CLUT \
                         * screens. For CLUT PIPs on CLUT screens the \
                         * PIP colors share the screen palette.       \
                         */
#define P96PIP_Colors32                                                      \
    (P96PIP_Dummy + 15) /* ULONG* (IS)                                       \
                         * Tag to set the palette colors at 32 bits-per-gun. \
                         * ti_Data is a pointer * to a table to be passed to \
                         * the graphics.library/LoadRGB32() function.        \
                         * This format supports both runs of color           \
                         * registers and sparse registers.  See the          \
                         * autodoc for that function for full details.       \
                         * Any color set here has precedence over            \
                         * the same register set by P96PIP_Colors.           \
                         * This only works with CLUT PIPs on non-CLUT        \
                         * screens. For CLUT PIPs on CLUT screens the        \
                         * PIP colors share the screen palette.              \
                         */
#define P96PIP_NoMemory (P96PIP_Dummy + 16)
#define P96PIP_RenderFunc (P96PIP_Dummy + 17)
#define P96PIP_SaveFunc (P96PIP_Dummy + 18)
#define P96PIP_UserData (P96PIP_Dummy + 19)
#define P96PIP_Alignment (P96PIP_Dummy + 20)
#define P96PIP_ConstantBytesPerRow (P96PIP_Dummy + 21)
#define P96PIP_AllowCropping (P96PIP_Dummy + 22)
#define P96PIP_InitialIntScaling (P96PIP_Dummy + 23)
#define P96PIP_ClipLeft (P96PIP_Dummy + 24)   /* ULONG (IS) */
#define P96PIP_ClipTop (P96PIP_Dummy + 25)    /* ULONG (IS) */
#define P96PIP_ClipWidth (P96PIP_Dummy + 26)  /* ULONG (IS) */
#define P96PIP_ClipHeight (P96PIP_Dummy + 27) /* ULONG (IS) */
#define P96PIP_ConstantByteSwapping (P96PIP_Dummy + 28)

enum
{
    PIPT_MemoryWindow, /* default */
    PIPT_VideoWindow,
    PIPT_NUMTYPES
};

#define P96PIPT_MemoryWindow PIPT_MemoryWindow
#define P96PIPT_VideoWindow PIPT_VideoWindow

#define PIPRel_Right 1  /* P96PIP_Left is relative to the right side (negative value) */
#define PIPRel_Bottom 2 /* P96PIP_Top is relative to the bottom (negative value) */
#define PIPRel_Width                                             \
    4 /* P96PIP_Width is amount of pixels not used by PIP at the \
                 right side of the window (negative value) */
#define PIPRel_Height                                             \
    8 /* P96PIP_Height is amount of pixels not used by PIP at the \
                 window bottom (negative value) */

#define PIPERR_NOMEMORY (1)      /* couldn't get normal memory */
#define PIPERR_ATTACHFAIL (2)    /* Failed to attach to a screen */
#define PIPERR_NOTAVAILABLE (3)  /* PIP not available for other reason	*/
#define PIPERR_OUTOFPENS (4)     /* couldn't get a free pen for occlusion */
#define PIPERR_BADDIMENSIONS (5) /* type, width, height or format invalid */
#define PIPERR_NOWINDOW (6)      /* couldn't open window */
#define PIPERR_BADALIGNMENT (7)  /* specified alignment is not ok */
#define PIPERR_CROPPED (8)       /* pip would be cropped, but isn't allowed to */
/************************************************************************/
/* Tags for P96GetRTGDataTagList
 */

#define P96RD_Dummy (TAG_USER + 0x40000 + 96)
#define P96RD_NumberOfBoards (P96RD_Dummy + 1)

/************************************************************************/
/* Tags for P96GetBoardDataTagList
 */

#define P96BD_Dummy (TAG_USER + 0x50000 + 96)
#define P96BD_BoardName (P96BD_Dummy + 1)
#define P96BD_ChipName (P96BD_Dummy + 2)
#define P96BD_TotalMemory (P96BD_Dummy + 4)
#define P96BD_FreeMemory (P96BD_Dummy + 5)
#define P96BD_LargestFreeMemory (P96BD_Dummy + 6)
#define P96BD_MonitorSwitch (P96BD_Dummy + 7)
#define P96BD_RGBFormats (P96BD_Dummy + 8)
#define P96BD_MemoryClock (P96BD_Dummy + 9)

/************************************************************************/
#endif
/************************************************************************/
