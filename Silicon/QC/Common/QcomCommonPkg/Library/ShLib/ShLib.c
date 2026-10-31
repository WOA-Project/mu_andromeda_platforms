/** @file

  Copyright (c) 2022-2024 DuoWoA authors

  SPDX-License-Identifier: MIT

**/
#include <PiPei.h>
#include <Library/HobLib.h>
#include <Library/PlatformHobs.h>
#include <Library/ShLib.h>

STATIC ShLibLoaderType *ShLibPtr = NULL;

EFI_STATUS ShLibInitialize(VOID)
{ 
  EFI_HOB_GUID_TYPE *GuidHob;
  UINT32            **DataPtr;
  
  GUID gEfiShLibHobGuid = EFI_SHIM_LIBRARY_GUID;
  GuidHob               = GetFirstGuidHob(&gEfiShLibHobGuid);

  if (GuidHob == NULL) {
    return EFI_NOT_READY;
  }
  
  DataPtr  = GET_GUID_HOB_DATA(GuidHob);
  ShLibPtr = (ShLibLoaderType *) DataPtr;
  
  return EFI_SUCCESS;
}

EFI_STATUS EFIAPI LoaderVersion(OUT UINT32* LoaderVersion)
{
  EFI_STATUS Status = EFI_SUCCESS;

  if (ShLibPtr == NULL) {
    Status = ShLibInitialize();
    if (EFI_ERROR(Status)) {
      return Status;
    }
  }
  
  if (ShLibPtr == NULL) {
      return EFI_NOT_READY;
  }

  if (LoaderVersion == NULL) {
    return EFI_COMPROMISED_DATA;
  }

  *LoaderVersion = ShLibPtr->LoaderVersion;

  return Status;
}

EFI_STATUS EFIAPI InstallLib(IN CHAR8 *LibName, IN UINT32 LibVersion, IN VOID *LibIntf)
{
  EFI_STATUS Status = EFI_SUCCESS;

  if (ShLibPtr == NULL) {
    Status = ShLibInitialize();
    if (EFI_ERROR(Status)) {
      return Status;
    }
  }
  
  if (ShLibPtr == NULL) {
      return EFI_NOT_READY;
  }

  return ShLibPtr->InstallLib(LibName, LibVersion, LibIntf);
}

EFI_STATUS EFIAPI LoadLib(CHAR8 *LibName, UINT32 LibVersion, VOID **LibIntf)
{
  EFI_STATUS Status = EFI_SUCCESS;

  if (ShLibPtr == NULL) {
    Status = ShLibInitialize();
    if (EFI_ERROR(Status)) {
      return Status;
    }
  }
  
  if (ShLibPtr == NULL) {
      return EFI_NOT_READY;
  }

  return ShLibPtr->LoadLib(LibName, LibVersion, LibIntf);
}
