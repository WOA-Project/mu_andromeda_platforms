#ifndef _DEVICE_MEMORY_MAP_H_
#define _DEVICE_MEMORY_MAP_H_

#include <Library/ArmLib.h>

#define MAX_ARM_MEMORY_REGION_DESCRIPTOR_COUNT 128

/* Below flag is used for system memory */
#define SYSTEM_MEMORY_RESOURCE_ATTR_CAPABILITIES                               \
  EFI_RESOURCE_ATTRIBUTE_PRESENT | EFI_RESOURCE_ATTRIBUTE_INITIALIZED |        \
      EFI_RESOURCE_ATTRIBUTE_TESTED | EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE |     \
      EFI_RESOURCE_ATTRIBUTE_WRITE_COMBINEABLE |                               \
      EFI_RESOURCE_ATTRIBUTE_WRITE_THROUGH_CACHEABLE |                         \
      EFI_RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE |                            \
      EFI_RESOURCE_ATTRIBUTE_EXECUTION_PROTECTABLE

typedef enum { NoHob, AddMem, AddDev, MaxMem } DeviceMemoryAddHob;

#define MEMORY_REGION_NAME_MAX_LENGTH 32

typedef struct {
  CHAR8                        Name[MEMORY_REGION_NAME_MAX_LENGTH];
  EFI_PHYSICAL_ADDRESS         Address;
  UINT64                       Length;
  DeviceMemoryAddHob           HobOption;
  EFI_RESOURCE_TYPE            ResourceType;
  EFI_RESOURCE_ATTRIBUTE_TYPE  ResourceAttribute;
  EFI_MEMORY_TYPE              MemoryType;
  ARM_MEMORY_REGION_ATTRIBUTES ArmAttributes;
} ARM_MEMORY_REGION_DESCRIPTOR_EX, *PARM_MEMORY_REGION_DESCRIPTOR_EX;

#define MEM_RES EFI_RESOURCE_MEMORY_RESERVED
#define MMAP_IO EFI_RESOURCE_MEMORY_MAPPED_IO
#define SYS_MEM EFI_RESOURCE_SYSTEM_MEMORY

#define SYS_MEM_CAP SYSTEM_MEMORY_RESOURCE_ATTR_CAPABILITIES
#define INITIALIZED EFI_RESOURCE_ATTRIBUTE_INITIALIZED
#define WRITE_COMBINEABLE EFI_RESOURCE_ATTRIBUTE_WRITE_COMBINEABLE
#define UNCACHEABLE EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE

#define Reserv EfiReservedMemoryType
#define Conv EfiConventionalMemory
#define BsData EfiBootServicesData
#define RtData EfiRuntimeServicesData
#define MmIO EfiMemoryMappedIO
#define HobOnlyNoCacheSetting AddMem

#define NS_DEVICE ARM_MEMORY_REGION_ATTRIBUTE_NONSECURE_DEVICE
#define WRITE_THROUGH ARM_MEMORY_REGION_ATTRIBUTE_WRITE_THROUGH
#define WRITE_THROUGH_XN ARM_MEMORY_REGION_ATTRIBUTE_WRITE_THROUGH
#define WRITE_BACK ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK
#define WRITE_BACK_XN ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK
#define UNCACHED_UNBUFFERED ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED
#define UNCACHED_UNBUFFERED_XN ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED

static ARM_MEMORY_REGION_DESCRIPTOR_EX gDeviceMemoryDescriptorEx[] = {
    /* Name               Address     Length      HobOption        ResourceAttribute    ArmAttributes
                                                          ResourceType          MemoryType */

    /* DDR Regions */
    /* DDR Bank 0 Start */
    {"Axon DMA",          0x80600000, 0x00100000, AddMem, SYS_MEM, SYS_MEM_CAP, Reserv, WRITE_BACK_XN},
    {"BOOT",              0x80700000, 0x00100000, AddMem, MEM_RES, SYS_MEM_CAP, BsData, WRITE_BACK_XN},
    {"AOP CMD DB",        0x80860000, 0x00020000, AddMem, MEM_RES, UNCACHEABLE, Reserv, UNCACHED_UNBUFFERED_XN},
    {"XBL Log Buffer",    0x80884000, 0x00010000, AddMem, SYS_MEM, SYS_MEM_CAP, Reserv, WRITE_BACK_XN},
    {"SMEM",              0x80900000, 0x00200000, AddMem, MEM_RES, UNCACHEABLE, Reserv, UNCACHED_UNBUFFERED_XN},
    {"PIL Reserved",      0x85200000, 0x16600000, AddMem, MEM_RES, UNCACHEABLE, Reserv, UNCACHED_UNBUFFERED_XN},
    {"DXE Heap",          0x9B800000, 0x03C00000, AddMem, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},
    {"Sched Heap",        0x9F400000, 0x00400000, AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK_XN},
    {"FV Region",         0x9F800000, 0x00200000, AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK_XN},
    {"ABOOT FV",          0x9FA00000, 0x00200000, AddMem, SYS_MEM, SYS_MEM_CAP, Reserv, WRITE_BACK_XN},
    {"UEFI FD",           0x9FC00000, 0x00300000, AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK},
    {"SEC Heap",          0x9FF00000, 0x0008C000, AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK_XN},
    {"CPU Vectors",       0x9FF8C000, 0x00001000, AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK},
    {"MMU PageTables",    0x9FF8D000, 0x00003000, AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK_XN},
    {"UEFI Stack",        0x9FF90000, 0x00040000, AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK_XN},
    {"Log Buffer",        0x9FFF7000, 0x00008000, AddMem, SYS_MEM, SYS_MEM_CAP, RtData, WRITE_BACK_XN},
    {"Info Blk",          0x9FFFF000, 0x00001000, AddMem, SYS_MEM, SYS_MEM_CAP, RtData, WRITE_BACK_XN},
    {"Kernel",            0xA0000000, 0x10000000, AddMem, SYS_MEM, SYS_MEM_CAP, Reserv, WRITE_BACK_XN},
    /* DDR Bank 0 End (0xB9300000) */
    /* Carveout Region (0xB9300000 -> 0xB9900000, size 0x00600000) */
    /* DDR Bank 1 0xB9900000 -> 0xBB700000 */
    /* DDR Bank 2 Start */
    {"Guest VM",          0xD0000000, 0x08800000, AddMem, SYS_MEM, SYS_MEM_CAP, Reserv, WRITE_BACK_XN},
    {"TZApps Reserved",   0xDA000000, 0x03900000, HobOnlyNoCacheSetting, MEM_RES, UNCACHEABLE, Reserv, UNCACHED_UNBUFFERED_XN},
    {"DBI Dump",          0xE0000000, 0x00F00000, NoHob,  MMAP_IO, INITIALIZED, Conv,   UNCACHED_UNBUFFERED_XN},
    {"Display Reserved",  0xE1000000, 0x02400000, AddMem, MEM_RES, SYS_MEM_CAP, Reserv, WRITE_THROUGH_XN},
    {"Apps Hob",          0xE3400000, 0x00001000, AddMem, MEM_RES, UNCACHEABLE, Reserv, UNCACHED_UNBUFFERED_XN},

    /* RAM partition regions */
    {"RAM Partition",     0x140000000,0x40000000, AddMem, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},
    /* DDR Bank 2 End */
    /* DDR Bank 3 Start */
#if USE_MEMORY_FOR_SERIAL_OUTPUT == 1
    {"RAM Partition",     0x180000000,0x3FE00000, AddMem, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},
    {"PStore",            0x1BFE00000,0x00200000, AddMem, MEM_RES, SYS_MEM_CAP, Reserv, WRITE_THROUGH_XN},
    {"RAM Partition",     0x1C0000000,0xC0000000, AddMem, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},
#else
    {"RAM Partition",    0x180000000,0x100000000, AddMem, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},
#endif
    /* DDR Bank 3 End */

    /* Other memory regions */
    {"IMEM Base",         0x14680000, 0x00040000, NoHob,  MMAP_IO, INITIALIZED, Conv,   NS_DEVICE},
    {"IMEM Cookie Base",  0x146BF000, 0x00001000, AddDev, MMAP_IO, INITIALIZED, Conv,   NS_DEVICE},

    /* Register regions */
    {"IPC_ROUTER_TOP",    0x00400000, 0x00100000, AddDev, MMAP_IO, UNCACHEABLE, MmIO,   NS_DEVICE},
    {"SECURITY CONTROL",  0x00780000, 0x00007000, AddDev, MMAP_IO, UNCACHEABLE, MmIO,   NS_DEVICE},
    {"QUP",               0x00800000, 0x00300000, AddDev, MMAP_IO, UNCACHEABLE, MmIO,   NS_DEVICE},
    {"PRNG_CFG_PRNG",     0x010D0000, 0x00010000, AddDev, MMAP_IO, UNCACHEABLE, MmIO,   NS_DEVICE},
    {"TCSR_TCSR_REGS",    0x01FC0000, 0x00030000, AddDev, MMAP_IO, UNCACHEABLE, MmIO,   NS_DEVICE},
    {"PERIPH_SS",         0x08800000, 0x00200000, AddDev, MMAP_IO, UNCACHEABLE, MmIO,   NS_DEVICE},
    {"USB",               0x0A600000, 0x04000000, AddDev, MMAP_IO, UNCACHEABLE, MmIO,   NS_DEVICE},
    {"AOSS",              0x0B000000, 0x04000000, AddDev, MMAP_IO, UNCACHEABLE, MmIO,   NS_DEVICE},
    {"TLMM_WEST",         0x0F100000, 0x00300000, AddDev, MMAP_IO, UNCACHEABLE, MmIO,   NS_DEVICE},
    {"TLMM_SOUTH",        0x0F500000, 0x00300000, AddDev, MMAP_IO, UNCACHEABLE, MmIO,   NS_DEVICE},
    {"TLMM_NORTH",        0x0F900000, 0x00300000, AddDev, MMAP_IO, UNCACHEABLE, MmIO,   NS_DEVICE},
    {"SMMU",              0x15000000, 0x000D0000, AddDev, MMAP_IO, UNCACHEABLE, MmIO,   NS_DEVICE},
    {"APSS_HM",           0x17800000, 0x00D99000, AddDev, MMAP_IO, UNCACHEABLE, MmIO,   NS_DEVICE},

    /* Terminator for MMU */
    {"Terminator", 0, 0, 0, 0, 0, 0, 0}};

#endif