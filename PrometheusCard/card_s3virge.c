#include <libraries/prometheus.h>
#include <proto/exec.h>
#include <proto/picasso96_chip.h>
#include <proto/prometheus.h>

#include "card.h"

struct CardInfo
{
    ULONG Device;
    UBYTE *Memory0;
    ULONG Memory0Size;
};

static ULONG count = 0;
#define PCI_VENDOR 0x5333
#define CHIP_NAME "picasso96/S3ViRGE.chip"

#define SystemSourceAperture ChipData[0]
#define SpecialRegisterBase ChipData[1]
#define BusType ChipData[3]  // 0: ZorroII - 1: ZorroIII - 2: PCI

BOOL InitS3ViRGE(struct CardBase *cb, struct BoardInfo *bi, ULONG dmaSize)
{
    struct Library *SysBase = cb->cb_SysBase;
    struct Library *PrometheusBase = cb->cb_PrometheusBase;
    APTR board = NULL;
    ULONG current = 0;


#ifdef DBG
    KPrintF("prometheus.card: InitS3ViRGE()\n");
#endif

    while ((board = (APTR)Prm_FindBoardTags(board, PRM_Vendor, PCI_VENDOR, TAG_END)) != NULL) {
        struct CardInfo ci;
        BOOL found = FALSE;

        Prm_GetBoardAttrsTags(board, PRM_Device, (ULONG)&ci.Device, PRM_MemoryAddr0, (ULONG)&ci.Memory0,
                              PRM_MemorySize0, (ULONG)&ci.Memory0Size, TAG_END);

        switch (ci.Device) {
        case 0x8A01:  // VirgeDX/GX
        case 0x8A02:  // Virge GX2
        case 0x5631:  // 86c325 [ViRGE]
            found = TRUE;
            break;
        default:
            found = FALSE;
        }

        if (found) {

#ifdef DBG
            KPrintF("prometheus.card: virge found\n");
#endif

            struct ChipBase *ChipBase;

            // check for multiple hits and skip the ones already used
            if (current++ < count)
                continue;

            // we have found one - so mark it as used and
            // don't care about possible errors from here on
            count++;

            ULONG memorySize = 4 * 1024*1024; // FIXME: query for actual size, could also be just 2MB

            if ((ChipBase = (struct ChipBase *)OpenLibrary(CHIP_NAME, 7)) != NULL) {
                // Take DMA memory off the lower end of memory, non-byteswapped
                if ((dmaSize > 0) && (dmaSize <= memorySize)) {
                    cb->cb_DMAMemGranted = TRUE;
                    InitDMAMemory(cb, ci.Memory0 + memorySize - dmaSize, dmaSize);
                } else {
                    dmaSize = 0;
                }

                bi->ChipBase = ChipBase;
                // The Virge is using a 64Mbyte  addressing window. This space is divided
                // into two 32mb windows, the lower (starting at BAR offset 0x00000000)
                // providing LE access, the upper (starting at 0x2000000) BE access.
                // Thus here we setup the chip access for BE access.
                bi->MemoryBase =   (UBYTE *)((ULONG)ci.Memory0 + 0x2000000);
                bi->RegisterBase = (UBYTE *)((ULONG)ci.Memory0 + 0x3008000);
                bi->SpecialRegisterBase =    (ULONG)ci.Memory0 + 0x3008000;
                bi->MemoryIOBase = (UBYTE *)((ULONG)ci.Memory0 + 0x3007FFC);
                bi->SystemSourceAperture =   (ULONG)ci.Memory0 + 0x3007FFC;
                bi->MemorySize = 0x400000 - dmaSize;
                bi->BusType = 2;

                ((UBYTE *)cb->cb_LegacyIOBase)[0x3C3] = 1;  // wakeup will only work with legacy IO

#ifdef DBG
                KPrintF("prometheus.card: register server\n");
#endif

                // register interrupt server
                RegisterIntServer(cb, board, &bi->HardInterrupt);

#ifdef DBG
                KPrintF("prometheus.card: init chip\n");
#endif

                InitChip(bi);

                // enable vertical blanking interrupt
                bi->Flags |= BIF_VBLANKINTERRUPT;

                bi->MemorySpaceBase = bi->MemoryBase;
                bi->MemorySpaceSize = bi->MemorySize;

                // enable special cache mode settings
                bi->Flags |= BIF_CACHEMODECHANGE;

                RegisterOwner(cb, board, (struct Node *)ChipBase);
                // no need to continue - we have found a match
                return TRUE;
            }
        }
    }  // while

#ifdef DBG
    KPrintF("prometheus.card: no virge found.\n");
#endif

    return FALSE;
}
