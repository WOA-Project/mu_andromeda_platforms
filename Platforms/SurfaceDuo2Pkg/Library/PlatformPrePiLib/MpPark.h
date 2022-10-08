#ifndef _MPPARK_H_
#define _MPPARK_H_

extern void _SecondaryModuleEntryPoint();
VOID WaitForSecondaryCPUs(VOID);

#define REDIR_MAILBOX_READY 0xfffffff1
#define REDIR_MAILBOX_SIGNAL 0xfffffff2
#define REDIR_MAILBOX_ACKNOWLEDGE 0xfffffff3

#pragma pack(1)
typedef struct {
  /* First 2KB is reserved for OS */
  UINT32 ProcessorId;
  UINT32 Reserved;
  UINT64 JumpAddress;
  UINT8  OsReserved[2032];
  /* Next 2KB is reserved for firmware */
  UINT64 JumpFlag;
} EFI_PROCESSOR_MAILBOX, *PEFI_PROCESSOR_MAILBOX;
#pragma pack()

#endif /* _MPPARK_H_ */