#include <clib/debug_protos.h>
#include <libraries/prometheus.h>
#include <proto/exec.h>
#include <proto/picasso96_chip.h>
#include <proto/prometheus.h>

#include "card.h"

struct CardInfo
{
  ULONG Device;
  ULONG Revision;
  UBYTE *Memory0;
  ULONG Memory0Size;
};

typedef enum ChipFamily
{
  UNKNOWN,
  VISION864 = 1,  // pre-Trio64, separate RAMDAC, oldstyle MMIO
  TRIO64,         // integrated RAMDAC, oldstyle MMIO
  TRIO64PLUS      // integrated RAMDAC, newstyle MMIO
} ChipFamily_t;

static ULONG count = 0;
#define PCI_VENDOR 0x5333
#define CHIP_NAME_TRIO64PLUS "picasso96/S3Trio64Plus.chip"
#define CHIP_NAME_TRIO3264 "picasso96/S3Trio3264.chip"
#define CHIP_NAME_VISION864 "picasso96/S3Vision864.chip"

#define CardPrometheusBase CardData[0]
#define CardPrometheusDevice CardData[1]

// The offsets allow for using signed 16bit indexed addressing be used
// FIXME: card driver and chip driver should share these somehow
#define REGISTER_OFFSET 0x8000
#define MMIOREGISTER_OFFSET 0x8000

BOOL InitS3Trio64(struct CardBase *cb, struct BoardInfo *bi, ULONG dmaSize)
{
  struct Library *SysBase = cb->cb_SysBase;
  struct Library *PrometheusBase = cb->cb_PrometheusBase;

  APTR board = NULL;
  ULONG current = 0;

#ifdef DBG
  KPrintF("prometheus.card: InitS3Trio64()\n");
#endif

  while ((board = (APTR)Prm_FindBoardTags(board, PRM_Vendor, PCI_VENDOR,
                                          TAG_END)) != NULL) {
    struct CardInfo ci;
    BOOL found = FALSE;

    Prm_GetBoardAttrsTags(board, PRM_Device, (ULONG)&ci.Device, PRM_Revision,
                          (ULONG)&ci.Revision, PRM_MemoryAddr0,
                          (ULONG)&ci.Memory0, PRM_MemorySize0,
                          (ULONG)&ci.Memory0Size, TAG_END);

#ifdef DBG
    KPrintF("prometheus.card: device %lx revision %lx\n", ci.Device,
            ci.Revision);
#endif

    ChipFamily_t chipFamily = UNKNOWN;

    switch (ci.Device) {
    case 0x88C0:  // 86c864 Vision 864
    case 0x88C1:  // 86c864 Vision 864
      chipFamily = VISION864;
      break;
    case 0x8813:            // 86c764_3 [Trio 32/64 vers 3]
      chipFamily = TRIO64;  // correct?
      break;
    case 0x8811:  // 86c764/765 [Trio32/64/64V+]
      chipFamily = ci.Revision & 0x40 ? TRIO64PLUS : TRIO64;
      break;
    case 0x8812:  // 86CM65 Aurora64V+
    case 0x8814:  // 86c767 [Trio 64UV+]
    case 0x8900:  // 86c755 [Trio 64V2/DX]
    case 0x8901:  // 86c775/86c785 [Trio 64V2/DX or /GX]
    case 0x8905:  // Trio 64V+ family
    case 0x8906:  // Trio 64V+ family
    case 0x8907:  // Trio 64V+ family
    case 0x8908:  // Trio 64V+ family
    case 0x8909:  // Trio 64V+ family
    case 0x890a:  // Trio 64V+ family
    case 0x890b:  // Trio 64V+ family
    case 0x890c:  // Trio 64V+ family
    case 0x890d:  // Trio 64V+ family
    case 0x890e:  // Trio 64V+ family
    case 0x890f:  // Trio 64V+ family
      chipFamily = TRIO64PLUS;
      break;
    default:
      chipFamily = UNKNOWN;
    }

    if (chipFamily == UNKNOWN) {
      continue;
    }

#ifdef DBG
    KPrintF("prometheus.card: %s found\n",
            (chipFamily >= TRIO64PLUS ? "Trio64+/V2"
             : chipFamily >= TRIO64   ? "Trio32/64"
                                      : "Vision864"));
#endif

    // check for multiple hits and skip the ones already used
    if (current++ < count)
      continue;

    // we have found one - so mark it as used and
    // don't care about possible errors from here on
    count++;

#ifdef DBG
    KPrintF(
        "prometheus.card: cb_LegacyIOBase 0x%lx , MemoryBase 0x%lx, "
        "MemorySize %ld\n",
        cb->cb_LegacyIOBase, ci.Memory0, ci.Memory0Size);
    APTR physicalAddress = Prm_GetPhysicalAddress(ci.Memory0);
    KPrintF("prometheus.card: physicalAddress 0x%08lx\n", physicalAddress);
#endif

    struct ChipBase *ChipBase = NULL;

    static const char *libNames[] = {CHIP_NAME_VISION864, CHIP_NAME_TRIO3264,
                                     CHIP_NAME_TRIO64PLUS};

    if (!(ChipBase = (struct ChipBase *)OpenLibrary(libNames[chipFamily - 1],
                                                    0)) != NULL) {
#ifdef DBG
      KPrintF("prometheus.card: ERROR could not open %d\n",
              libNames[chipFamily - 1]);
      return FALSE;
#endif
    }
    bi->CardPrometheusBase = (ULONG)PrometheusBase;
    bi->CardPrometheusDevice = (ULONG)board;
    bi->ChipBase = ChipBase;

    if (chipFamily >= TRIO64PLUS) {
      // The Trio64
      // S3Trio64.chip expects register base adress to be offset by 0x8000
      // to be able to address all registers with just regular signed 16bit
      // offsets
      bi->RegisterBase = ((UBYTE *)cb->cb_LegacyIOBase) + REGISTER_OFFSET;
      // Use the Trio64+ MMIO range in the BE Address Window at BaseAddress +
      // 0x3000000
      bi->MemoryIOBase = ci.Memory0 + 0x3000000 + MMIOREGISTER_OFFSET;
      // No need to fudge with the base address here
      bi->MemoryBase = ci.Memory0;
    } else {
      bi->RegisterBase = ((UBYTE *)cb->cb_LegacyIOBase) + REGISTER_OFFSET;
      // This is how I understand Trio64/32 MMIO approach: 0xA0000 is
      // hardcoded as the base of the enhanced registers I need to make
      // sure, the first 1 MB of address space don't overlap with anything.
      bi->MemoryIOBase = Prm_GetVirtualAddress((UBYTE *)0xA0000);

      if (bi->MemoryIOBase == NULL) {
#ifdef DBG
        KPrintF(
            "prometheus.card: VGA memory window at 0xA0000-BFFFF isnot "
            "available. Aborting.\n");
#endif
        return FALSE;
      }

#ifdef DBG
      KPrintF("MMIO Base at physical address 0xA0000 virtual: 0x%lx.\n",
              bi->MemoryIOBase);
#endif
      bi->MemoryIOBase += MMIOREGISTER_OFFSET;

      // I have to push out the card's Linear Address Window memory base
      // address to not overlap with its own MMIO address range at
      // 0xA8000-0xAFFFF On Trio64+ this is way easier with the "new MMIO"
      // approach. Here we move the Linear Address Window up by 4MB. This
      // gives us 4MB alignment and moves the LAW while not moving the PCI
      // BAR of teh card. The assumption is that the gfx card is the first
      // one to be initialized and thus sit at 0x00000000 in PCI address
      // space. This way 0xA8000 is in the card's BAR and the LAW should be
      // at 0x400000
      bi->MemoryBase = ci.Memory0;
      if (Prm_GetPhysicalAddress(bi->MemoryBase) <= (APTR)0xB0000) {
        // This shifts the memory base address by 4MB, which should be ok
        // since the S3Trio asks for 8MB PCI address space, typically only
        // utilizing the first 4MB
        bi->MemoryBase += 0x400000;
#ifdef DBG
        KPrintF(
            "WARNING: Trio64/32 memory base overlaps with MMIO address at "
            "0xA8000-0xAFFFF.\n"
            "Moving FB adress window out by 4mb to 0x%lx\n",
            bi->MemoryBase);
#endif
      }
    }

#ifdef DBG
    KPrintF("prometheus.card: Trio64 init chip\n");
#endif
    InitChip(bi);
#ifdef DBG
    KPrintF("prometheus.card: Trio64 has %ldkb usable memory\n",
            bi->MemorySize / 1024);

    KPrintF("prometheus.card: register server\n");
#endif

    // register interrupt server
    RegisterIntServer(cb, board, &bi->HardInterrupt);

    // enable vertical blanking interrupt
    //                bi->Flags |= BIF_VBLANKINTERRUPT;

    bi->MemorySpaceBase = ci.Memory0;
    bi->MemorySpaceSize = ci.Memory0Size;

    // enable special cache mode settings
    bi->Flags |= BIF_CACHEMODECHANGE;

#ifdef DBG
    KPrintF("prometheus.card: register owner\n");
#endif

    RegisterOwner(cb, board, (struct Node *)ChipBase);

    // FIXME: DMA memory needs to be setup differently on Trio64 and Trio64+
    // Trio64 has only one memory aperture and it is LE (BE from CPU view)
    // Trio64+ can expose two windows, LE and BE (thus should be able to
    // support more formats)

    if ((dmaSize > 0) && (dmaSize <= bi->MemorySize)) {
      // Place DMA window at end of memory window 0 and page-align it
      ULONG dmaOffset = (bi->MemorySize - dmaSize) & ~(4096 - 1);
      InitDMAMemory(cb, bi->MemoryBase + dmaOffset, dmaSize);
      bi->MemorySize = dmaOffset;
      cb->cb_DMAMemGranted = TRUE;
    }
    // no need to continue - we have found a match
    return TRUE;

  }  // while

#ifdef DBG
  KPrintF("prometheus.card: no Vision864/Trio32/64 found.\n");
#endif

  return FALSE;
}
