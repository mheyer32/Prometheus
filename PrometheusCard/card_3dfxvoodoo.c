#include <proto/exec.h>
#include <proto/picasso96_chip.h>
#include <proto/prometheus.h>
#include <exec/types.h>
#include <libraries/prometheus.h>

#include "card.h"

#ifdef DBG
#include <clib/debug_protos.h>
#endif

struct CardInfo
{
    ULONG Device;
    UBYTE *Memory0;
    UBYTE *Memory1;
    UBYTE *Memory2;
    ULONG Memory1Size;
    UBYTE *ROM;
};

static ULONG count = 0;
#define PCI_VENDOR 0x121A
#define CHIP_NAME "picasso96/3dfxVoodoo.chip"

#define ROMBase ChipData[14]
#define DeviceID ChipData[15]

BOOL Init3dfxVoodoo(struct CardBase *cb, struct BoardInfo *bi, ULONG dmaSize)
{
    struct Library *PrometheusBase = cb->cb_PrometheusBase;
    struct Library *SysBase = cb->cb_SysBase;

    APTR board = NULL;
    ULONG current = 0;

#ifdef DBG
    KPrintF("prometheus.card: Init3dfxVoodoo()\n");
#endif

    while ((board = (APTR)Prm_FindBoardTags(board, PRM_Vendor, PCI_VENDOR, TAG_END)) != NULL) {
        struct CardInfo ci;
        BOOL found = FALSE;

#ifdef DBG
        KPrintF("  Voodoo board found on PCI [$%08lx]\n", (ULONG)board);
#endif

        Prm_GetBoardAttrsTags(board, PRM_Device, (ULONG)&ci.Device, PRM_MemoryAddr0, (ULONG)&ci.Memory0,
                              PRM_MemoryAddr1, (ULONG)&ci.Memory1, PRM_MemoryAddr2, (ULONG)&ci.Memory2, PRM_MemorySize1,
                              (ULONG)&ci.Memory1Size, PRM_ROM_Address, (ULONG)&ci.ROM, TAG_END);

        switch (ci.Device) {
        case 3:  // Banshee
        case 5:  // Avenger
        case 9:  // Rampage
            found = TRUE;
            break;
        default:
            found = FALSE;
        }

        if (found) {
            struct ChipBase *ChipBase;

#ifdef DBG
            KPrintF("  card attrs read (device %ld)\n", (ULONG)ci.Device);
#endif

            // check for multiple hits and skip the ones already used
            if (current++ < count)
                continue;

            // we have found one - so mark it as used and
            // don't care about possible errors from here on
            count++;

            if ((ChipBase = (struct ChipBase *)OpenLibrary(CHIP_NAME, 7)) != NULL) {
#ifdef DBG
                KPrintF("  chip driver opened [$%08lx]\n", (ULONG)ChipBase);
                KPrintF("  MemIOBase [$%08lx], Memory0Base [$%08lx], Memory1Base [$%08lx], Memory1Size [%ldmb] \n",
                        ci.Memory2, ci.Memory0, ci.Memory1, ci.Memory1Size / (1024 * 1024));
#endif

                bi->ChipBase = ChipBase;

                bi->DeviceID = ci.Device;
                bi->MemoryIOBase = ci.Memory0;
                bi->MemoryBase = ci.Memory1;
                bi->RegisterBase = ci.Memory2;
                bi->MemorySize = ci.Memory1Size;
                bi->ROMBase = (ULONG)ci.ROM;

                // register interrupt server
                RegisterIntServer(cb, board, &bi->HardInterrupt);

                InitChip(bi);

                // enable vertical blanking interrupt
                bi->Flags |= BIF_VBLANKINTERRUPT;

                Prm_SetBoardAttrsTags(board, PRM_BoardOwner, (ULONG)ChipBase, TAG_END);

                if ((dmaSize > 0) && (dmaSize <= bi->MemorySize)) {
#ifdef DBG
                    KPrintF("  BoardInfo::MemorySize %ldmb\n", bi->MemorySize / (1024 * 1024));
#endif
                    // Place DMA window at end of memory window and page-align it
                    ULONG dmaOffset = (bi->MemorySize - dmaSize) & ~(4096 - 1);
                    InitDMAMemory(cb, bi->MemoryBase + dmaOffset, dmaSize);
                    bi->MemorySize = dmaOffset;
                    cb->cb_DMAMemGranted = TRUE;
                }

                return TRUE;
            }
        }
    }  // while
    return FALSE;
}
