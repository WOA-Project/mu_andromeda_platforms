/** @file

  Copyright (c) 2022-2024 DuoWoA authors

  SPDX-License-Identifier: MIT

**/
#ifndef _SH_LIB_H_
#define _SH_LIB_H_

EFI_STATUS EFIAPI LoaderVersion(OUT UINT32* LoaderVersion);
EFI_STATUS EFIAPI InstallLib(IN CHAR8 *LibName, IN UINT32 LibVersion, IN VOID *LibIntf);
EFI_STATUS EFIAPI LoadLib(CHAR8 *LibName, UINT32 LibVersion, VOID **LibIntf);

#endif /* _SH_LIB_H_ */