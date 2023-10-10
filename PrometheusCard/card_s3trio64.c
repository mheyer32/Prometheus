#include <clib/debug_protos.h>
#include <libraries/prometheus.h>
#include <proto/exec.h>
#include <proto/picasso96_chip.h>
#include <proto/prometheus.h>

#include "card.h"
#include "chip_s3trio64.h"

struct CardInfo
{
  ULONG Device;
  ULONG Revision;
  UBYTE *Memory0;
  ULONG Memory0Size;
};

static ULONG count = 0;
#define PCI_VENDOR 0x5333

ULONG CompatibleFormats[] = {1,   232, 0, 0, 232, 232, 40, 0, 0, 232, 400,
                             400, 0,   0, 0, 0,   0,   0,  0, 0, 0};

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

    KPrintF("prometheus.card: device %lx revision %lx\n", ci.Device,
            ci.Revision);

    switch (ci.Device) {
    case 0x8811:  // 86c764/765 [Trio32/64/64V+]
    case 0x8813:  // 86c764_3 [Trio 32/64 vers 3]
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
      found = TRUE;
      break;
    default:
      found = FALSE;
    }

    if (found) {
      BOOL isTrio64Plus = ((ci.Revision & 0x40) != 0);
#ifdef DBG
      KPrintF("prometheus.card: Trio%s found\n",
              (isTrio64Plus ? "64+" : "32/64"));
#endif

      getChipData(bi)->Revision = ci.Revision;

      // check for multiple hits and skip the ones already used
      //            if (current++ < count)
      //                continue;

      // we have found one - so mark it as used and
      // don't care about possible errors from here on
      count++;

#ifdef DBG
      KPrintF("prometheus.card: cb_LegacyIOBase 0x%lx , MemoryBase 0x%lx, MemorySize %ld\n",
              cb->cb_LegacyIOBase, ci.Memory0, ci.Memory0Size);
      APTR physicalAddress = Prm_GetPhysicalAddress(bi->MemoryBase);
      KPrintF("prometheus.card: physicalAdress 0x%08lx\n", physicalAddress);
#endif

      bi->CardPrometheusBase = PrometheusBase;

      if (isTrio64Plus) {
        // The Trio64
        // S3Trio64.chip expects register base adress to be offset by 0x8000 to
        // be able to address all registers with just regular signed 16bit
        // offsets
        bi->RegisterBase = ((UBYTE *)cb->cb_LegacyIOBase) + REGISTER_OFFSET;
        // Trio64+ places the MMIO range at BaseAddress + 0x1000000
        bi->MemoryIOBase = ci.Memory0 + 0x1000000 + MMIOREGISTER_OFFSET;
        // No need to fudge with the base address here
        bi->MemoryBase = ci.Memory0;
      } else {
        if ((Prm_GetPhysicalAddress(ci.Memory0) != 0x0 ||
             ci.Memory0Size < 0x800000)) {
          KPrintF(
              "ERROR: Trio64/32 needs to be at physical address 0x0, so we can "
              "use the address range 0xA8000-0xAFFFF for MMIO access");
          return FALSE;
        }
        bi->RegisterBase = ((UBYTE *)cb->cb_LegacyIOBase) + REGISTER_OFFSET;
        // This is how I understand Trio64/32 MMIO approach: 0xA0000 is
        // hardcoded as the base of the enhanced registers I need to make sure,
        // the first 1 MB of address space don't overlap with anything.
        bi->MemoryIOBase =
            Prm_GetVirtualAddress((UBYTE *)0xA0000 + MMIOREGISTER_OFFSET);

        // I have to push out the card's Linear Address Window memory base
        // address to not overlap with its own MMIO address range at
        // 0xA8000-0xAFFFF On Trio64+ this is way easier with the "new MMIO"
        // approach. Here we move the Linear Address Window up by 4MB. This
        // gives us 4MB alignment and moves the LAW while not moving the PCI
        // BAR of teh card. The assumption is that the gfx card is the first
        // one to be initialized and thus sit at 0x00000000 in PCI address
        // space. This way 0xA8000 is in the card's BAR and the LAW should be
        // at 0x400000
        bi->MemoryBase = ci.Memory0 + 0x400000;
      }

      REGBASE();
      MMIOBASE();

#ifdef DBG
      KPrintF("prometheus.card: Trio64 init chip\n");
#endif
      InitChipL(bi);

      CacheClearU();

#ifdef DBG
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

//      RegisterOwner(cb, board, (struct Node *)ChipBase);
       RegisterOwner(cb, board, 0xCAFEBABE);

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
    }
  }  // while

#ifdef DBG
  KPrintF("prometheus.card: no Trio64 found.\n");
#endif

  return FALSE;
}
