#include <libraries/prometheus.h>
#include <proto/exec.h>
#include <proto/picasso96_chip.h>
#include <proto/prometheus.h>

#include "card.h"

#include "clib/debug_protos.h"


struct CardInfo
{
    ULONG Device;
    ULONG Revision;
    UBYTE *Memory0;
    ULONG Memory0Size;
};

static ULONG count = 0;
#define PCI_VENDOR 0x5333
#define CHIP_NAME "picasso96/S3Trio64.chip"

#define SystemSourceAperture ChipData[0]
#define SpecialRegisterBase ChipData[1]
#define BusType ChipData[3]  // 0: ZorroII - 1: ZorroIII - 2: PCI


#define SEQX 0x3C4          // Access SRxx registers
#define SEQ_DATA 0x3C5

#define CRTC_IDX 0x3D4    // Access CRxx registers
#define CRTC_DATA 0x3D5

static inline UBYTE readCRx(const struct CardBase *cb, UBYTE regIndex)
{
  volatile UBYTE *regbase = (volatile UBYTE*)cb->cb_LegacyIOBase;
  regbase[CRTC_IDX] = regIndex;
  UBYTE value =  regbase[CRTC_DATA];
#ifdef DBG
  KPrintF("prometheus.card: reading CR%2lx -> 0x%lx\n", (LONG)regIndex, (LONG)value);
#endif
  return value;
}

static inline void writeCRx(const struct CardBase *cb, UBYTE regIndex, UBYTE value)
{
  volatile UBYTE *regbase = (volatile UBYTE*)cb->cb_LegacyIOBase;
  regbase[CRTC_IDX] = regIndex;
  regbase[CRTC_DATA] = value;
#ifdef DBG
  KPrintF("prometheus.card: writing CR%2lx <- 0x%lx\n", (LONG)regIndex, (LONG)value);
#endif

}


static inline UBYTE readSRx(const struct CardBase *cb, UBYTE regIndex)
{
  volatile UBYTE *regbase = (volatile UBYTE*)cb->cb_LegacyIOBase;
  regbase[SEQX] = regIndex;
  UBYTE value =  regbase[SEQ_DATA];
#ifdef DBG
  KPrintF("prometheus.card: reading SR%2lx -> 0x%lx\n", (LONG)regIndex, (LONG)value);
#endif
  return value;
}

static inline void writeSRx(const struct CardBase *cb, UBYTE regIndex, UBYTE value)
{
  volatile UBYTE *regbase = (volatile UBYTE*)cb->cb_LegacyIOBase;
  regbase[SEQX] = regIndex;
  regbase[SEQ_DATA] = value;
#ifdef DBG
  KPrintF("prometheus.card: wrting SR%2lx <- 0x%lx\n", (LONG)regIndex, (LONG)value);
#endif
}

BOOL InitS3Trio64(struct CardBase *cb, struct BoardInfo *bi, ULONG dmaSize)
{
    struct Library *SysBase = cb->cb_SysBase;
    struct Library *PrometheusBase = cb->cb_PrometheusBase;
    APTR board = NULL;
    ULONG current = 0;


#ifdef DBG
    KPrintF("prometheus.card: InitS3Trio64()\n");
#endif

    while ((board = (APTR)Prm_FindBoardTags(board, PRM_Vendor, PCI_VENDOR, TAG_END)) != NULL) {
        struct CardInfo ci;
        BOOL found = FALSE;

        Prm_GetBoardAttrsTags(board, PRM_Device, (ULONG)&ci.Device, PRM_Revision, (ULONG)&ci.Revision, PRM_MemoryAddr0, (ULONG)&ci.Memory0,
                              PRM_MemorySize0, (ULONG)&ci.Memory0Size, TAG_END);

#ifdef DBG
        KPrintF("prometheus.card: device %lx revision %lx\n", ci.Device, ci.Revision);
#endif

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

          BOOL isTrio64Plus = ((ci.Revision & 0x40) != 0) ;
#ifdef DBG
            KPrintF("prometheus.card: Trio%s found\n", (isTrio64Plus ? "64+" : "32/64"));
#endif

            struct ChipBase *ChipBase;

            // check for multiple hits and skip the ones already used
//            if (current++ < count)
//                continue;

            // we have found one - so mark it as used and
            // don't care about possible errors from here on
            count++;

            ULONG memorySize = 2 * 1024*1024; // FIXME: query for actual size, could also be just 2MB

            if ((ChipBase = (struct ChipBase *)OpenLibrary(CHIP_NAME, 7)) != NULL) {

#ifdef DBG
              KPrintF("prometheus.card: cb_LegacyIOBase 0x%lx , MemoryBase 0x%lx\n", cb->cb_LegacyIOBase, ci.Memory0);
#endif

                bi->ChipBase = ChipBase;
                // The Trio64
                // S3Trio64.chip expects register base adress to be offset by 0x8000 to be able to address
                // all registers with just regular signed 16bit offsets
                bi->RegisterBase = ((UBYTE *)cb->cb_LegacyIOBase) + 0x8000;
                ///XXXX
                bi->MemoryBase =   (UBYTE *)((ULONG)ci.Memory0 + 0x00400000);

                bi->MemoryIOBase = 0;
                bi->MemorySize = memorySize;

#ifdef DBG
                KPrintF("prometheus.card: init chip\n");
#endif

                CacheClearU();

                volatile UBYTE *registersB = ((UBYTE *)cb->cb_LegacyIOBase);

//#define TRIO64



                if (!isTrio64Plus) {
                  // Chip wakeup via IO
                  registersB[0x3c3] = 0x10;// bit 3 = 0, disable I/O and memory decoders
                                           // bit 4 = 1; place video subsystem in setup mode
                  registersB[0x102] = 0x01;  // Take out of sleep mode
                  registersB[0x3c3] = 0x08;  // Bit 3 = 1, enable I/O and memory decoders
                                           // Bit 4 = 0, return to operational mode
                } else {
                  // Chip wakeup via IO
                  registersB[0x3c3] = 0x01;
                }

                // so the readCRx/writeCRx functions read the read/write the
                // right registers
                registersB[0x3c6] = 0xff;  // Initialize DAC mask and release BLANK signal FIXME: Bit 4 of CR33 controls writes to these registers.
                registersB[0x3c2] = 0x23;  // Enable color emulation, D-based addresses


                // Write code to CR38 to provide access to the S3 VGA registers (CR30-CR3F)
                writeCRx(cb, 0x38, 0x48);
                // Write code to CR39 to provide access to the System Control
                // and System Extension registers (CR40-CRFF)
                writeCRx(cb, 0x39, 0xa5);

#ifdef DBG
                UBYTE cr65 = readCRx(cb, 0x65);
                WORD setupReg = (cr65 & 0x4) ? 0x3C3 : 0x46E8;
                UBYTE chipIDLow = readCRx(cb, 0x2e);
                UBYTE revision = readCRx(cb, 0x2f);
                UBYTE chipIdRev = readCRx(cb, 0x30);
                KPrintF("prometheus.card: trio  IdRev %lx device ID 0x%lx, revision %ld, setupReg %lx \n", (LONG) chipIdRev, (LONG)chipIDLow, (LONG)revision, (LONG)setupReg);
#endif

                // Enable 0x4AE8 as system control register
                writeCRx(cb, 0x65, readCRx(cb, 0x65) & ~0x4);

                 CacheClearU();
                InitChip(bi);

                APTR physicalAddress = Prm_GetPhysicalAddress(bi->MemoryBase);
                KPrintF("prometheus.card: physicalAdress 0x%08lx\n", physicalAddress);

//                writeCRx(cb, 0x5a,  (ULONG)physicalAddress >> 16); // CR5A contains lower byte of LAW address
//                writeCRx(cb, 0x59,  (ULONG)physicalAddress >> 24); // CR59 contains upper byte

                // Unlock SR9-SR18 registers
                writeSRx(cb, 0x8, 0x06);

                //                registersB[0x4AE8] = 0;

                //                registersB[0x3c6] = 0xff;
                //                registersB[0x3c2] = 0x23;  // Enable color
                //                emulation, D-based addresses

                //                // Unlock S3 registers
                //                // Write code to CR38 to provide access to the
                //                S3 VGA registers (CR30-CR3F) writeCRx(cb,
                //                0x38,0x48);
                //                // Write code to CR39 to provide access to the
                //                System Control and System Extension registers
                //                (CR40-CRFF) writeCRx(cb, 0x39,0xa5);
                //                // Set bit 0 in CR40 to enable access to the
                //                Enhanced Commands registers. writeCRx(cb,
                //                0x40, readCRx(cb, 0x40) | 0x1);

                //                // Enable Enhanced mode
                registersB[0x4AE9] = registersB[0x4AE9] | 0x31;
                writeCRx(cb, 0x31, 0x4);


                //                // enable MMIO and IO based register access
                //                when MMIO is enabled writeSRx(cb, 0x9, 0x00);

                // enable MMIO Access
                //                writeCRx(cb, 0x53, readCRx(cb, 0x53) | 0x10);

                // FIXME: the specs say "Define display memory size", does that mean this register can be used to change the
                // memory size, not query the physically present memory? But it also says that CR36 is being
                // initialized with via straps at reset time

                //FIXME: hardcode to 2MB for now
//                writeCRx(cb, 0x36, (readCRx(cb, 0x36) & ~0xE0) | (0b100 << 5));
                // 1MB
                //                writeCRx(cb, 0x36, (readCRx(cb, 0x36) & ~0xE0) | (0b110 << 5));
                // OE vs RAS
//                writeCRx(cb, 0x68, (readCRx(cb, 0x68) & ~0x80) | 0x80);
//                writeCRx(cb, 0x68, (readCRx(cb, 0x68) & ~0x80));

 //               writeSRx(cb, 0xA, (readSRx(cb, 0xA) & ~0x40) | 0x40);

                CacheClearU();

                bi->MemorySize = 0x400000;
                volatile ULONG *framebuffer = (volatile ULONG *)bi->MemoryBase;
                framebuffer[0] = 0;
                while (bi->MemorySize) {
                  // Enable Linear Addressing Window LAW
                  {
                    UBYTE LAWSize = 0;
                    UBYTE MemSize = 0;
                    if (bi->MemorySize >= 0x400000) {
                      LAWSize = 0b11;
                      MemSize = 0b000;
                    } else if (bi->MemorySize >= 0x200000) {
                      LAWSize = 0b10;  // 2MB
                      MemSize = 0b100;
                    } else {
                      LAWSize = 0b01;  // 1MB
                      MemSize = 0b110;
                    }
                    writeCRx(cb, 0x36,
                             (readCRx(cb, 0x36) & ~0xE) | (MemSize << 5));
                    writeCRx(cb, 0x58,
                             (readCRx(cb, 0x58) & ~0x13) | LAWSize | 0x10);
                  }

                  CacheClearU();

                  volatile ULONG *highOffset = framebuffer + (bi->MemorySize >> 2) - 1;
                  volatile ULONG *lowOffset = framebuffer + (bi->MemorySize >> 3);
                  // Probe  memory
                  *framebuffer = 0;
                  *highOffset = (ULONG)highOffset;
                  *lowOffset = (ULONG)lowOffset;

                  CacheClearU();

                  ULONG readbackHigh = *highOffset;
                  ULONG readbackLow = *lowOffset;
                  ULONG readbackZero = *framebuffer;
#ifdef DBG
                  KPrintF("prometheus.card: probing memory at 0x%lx == 0x%lx\n",
                          highOffset, readbackHigh);
#endif
                  if (readbackHigh == (ULONG)highOffset &&
                      readbackLow == (ULONG)lowOffset && readbackZero == 0) {
                    break;
                  }
                  // reduce available memory size
                  bi->MemorySize >>= 1;
                }

#ifdef DBG
                KPrintF("prometheus.card: memorySize %ldmb ", bi->MemorySize / (1024 * 1024));
#endif

#ifdef DBG
                UBYTE memType = (readCRx(cb, 0x36) >> 2) & 3;
                switch (memType) {
                case 0b10:
                  KPrintF("EDO\n");
                  break;
                case 0b11:
                  KPrintF("FPM\n");
                  break;
                default:
                  KPrintF("unknown memory type\n");
                }
#endif

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
                RegisterOwner(cb, board, (struct Node *)ChipBase);


                if ((dmaSize > 0) && (dmaSize <= bi->MemorySize)) {
                    // Place DMA window at end of memory window 0 and page-align it
                    ULONG dmaOffset = (bi->MemorySize - dmaSize) & ~(4096 - 1);
                    InitDMAMemory(cb, ci.Memory0 + dmaOffset, dmaSize);
                    bi->MemorySize = dmaOffset;
                    cb->cb_DMAMemGranted = TRUE;
                }

//                assert(bi->MemorySize > 0x3800);
                bi->MemorySize -= 0x3800; // reserve top part of memory for sprite cursor

                // no need to continue - we have found a match
                return TRUE;
            }
        }
    }  // while

#ifdef DBG
    KPrintF("prometheus.card: no Trio64 found.\n");
#endif

    return FALSE;
}
