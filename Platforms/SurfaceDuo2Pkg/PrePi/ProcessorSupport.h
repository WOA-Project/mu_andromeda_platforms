#pragma once
#ifndef _ARM_PROC_SUPPORT_H_
#define _ARM_PROC_SUPPORT_H_

extern void ArmDeInitialize(void);
extern void ArmRelocateFirmware(void);
extern UINTN QcHavenWatchdogCall(UINT32 SmcId, UINT32 Argument1, UINT32 Argument2, VOID* Response); 

#endif