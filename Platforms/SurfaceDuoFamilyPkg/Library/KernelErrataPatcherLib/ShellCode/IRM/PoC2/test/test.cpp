#include <iostream>
#include <Windows.h>

#define SGIR_TGT_MASK     ((UINT64)0xffff)
#define SGIR_AFF1_SHIFT   16
#define SGIR_INTID_SHIFT  24
#define SGIR_INTID_MASK   ((UINT64)0xf)
#define SGIR_AFF2_SHIFT   32
#define SGIR_IRM_SHIFT    40
#define SGIR_IRM_MASK     ((UINT64)0x1)
#define SGIR_AFF3_SHIFT   48
#define SGIR_AFF_MASK     ((UINT64)0xff)

#define SGIR_IRM_TO_AFF     0
#define SGIR_IRM_TO_OTHERS  1

#define GICV3_SGIR_VALUE(_aff3, _aff2, _aff1, _intid, _irm, _tgt) \
  ((((UINT64) (_aff3) & SGIR_AFF_MASK) << SGIR_AFF3_SHIFT) | \
   (((UINT64) (_irm) & SGIR_IRM_MASK) << SGIR_IRM_SHIFT) | \
   (((UINT64) (_aff2) & SGIR_AFF_MASK) << SGIR_AFF2_SHIFT) | \
   (((_intid) & SGIR_INTID_MASK) << SGIR_INTID_SHIFT) | \
   (((_aff1) & SGIR_AFF_MASK) << SGIR_AFF1_SHIFT) | \
   ((_tgt) & SGIR_TGT_MASK))

void ArmGicV3SendNsG1Sgi(UINT64 SgiVal)
{
    printf_s("SGIValue=0x%p\n", SgiVal);
}

void Send(UINT8  SgiId, UINT64 CurrentMpidr)
{
    /*for (UINT32 i = 0; i < 8; i++)
    {
        if (i == CurrentMpidr >> 8) {
            continue;
        }

        // 0x000000000F000001
        // 0x000000000F010001
        // 0x000000000F020001
        ArmGicV3SendNsG1Sgi(GICV3_SGIR_VALUE(0, 0, i, SgiId, 0, 1));
    }*/

    /*UINT64 SGI = (SgiId & 0xF) << 24 | 1;
    for (UINT64 i = SGI; i < SGI + 0x80000; i += 0x10000)
    {
      ArmGicV3SendNsG1Sgi(i);
    }*/

    /*
    
    UINT64 SGI = (SgiId & 0xF) << 24 | 1;
    UINT8 CurrentI = CurrentMpidr >> 8;

    UINT8 i = 0;

    do {
      if (i == CurrentI) {
        i++;
        continue;
      }

      ArmGicV3SendNsG1Sgi(SGI | (i << 16));
      i++;
    } while (i != 8);
    
    */

    /*UINT64 SGI = (SgiId & 0xF) << 24 | 1;
    UINT8 CurrentI = CurrentMpidr >> 8;

    UINT8 i = 0;

    do {
      if (i == CurrentI) {
        i++;
        SGI += 0x10000;
        continue;
      }

      ArmGicV3SendNsG1Sgi(SGI);
      i++;
      SGI += 0x10000;
    } while (i != 8);*/

    UINT64 SGI      = (SgiId & 0xF) << 24 | 1;
    UINT64 CurrentSGI = SGI | (CurrentMpidr & 0xF00) << 8;
    for (UINT64 i = SGI; i < SGI + 0x80000; i += 0x10000) {
      if (i != CurrentSGI)
        ArmGicV3SendNsG1Sgi(i);
    }
}

int main()
{
    UINT8 SgiId = 15;
    Send(SgiId, 0x81000500);
}