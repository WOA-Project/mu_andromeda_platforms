#include <PiDxe.h>
#include <Uefi.h>

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>

STATIC EFI_IMAGE_LOAD mEfiImageLoad = NULL;

VOID *SetServicePointer(
    IN OUT EFI_TABLE_HEADER *ServiceTableHeader,
    IN OUT VOID **ServiceTableFunction, IN VOID *NewFunction)
{
  EFI_TPL oldTPL           = 0;
  VOID   *OriginalFunction = NULL;

  if (ServiceTableFunction == NULL || NewFunction == NULL)
    return NULL;

  oldTPL = gBS->RaiseTPL(TPL_HIGH_LEVEL);

  OriginalFunction      = *ServiceTableFunction;
  *ServiceTableFunction = NewFunction;

  ServiceTableHeader->CRC32 = 0;
  gBS->CalculateCrc32(
      (UINT8 *)ServiceTableHeader, ServiceTableHeader->HeaderSize,
      &ServiceTableHeader->CRC32);

  gBS->RestoreTPL(oldTPL);

  return OriginalFunction;
}

EFI_STATUS
EFIAPI
KernelErrataPatcherLoadImage(
    IN BOOLEAN BootPolicy, IN EFI_HANDLE ParentImageHandle,
    IN EFI_DEVICE_PATH_PROTOCOL *DevicePath, IN VOID *SourceBuffer OPTIONAL,
    IN UINTN SourceSize, OUT EFI_HANDLE *ImageHandle)
{
  CHAR16 *ImagePath = NULL;
  if (DevicePath != NULL) {
    ImagePath = ConvertDevicePathToText(DevicePath, TRUE, TRUE);

    DEBUG(
        (EFI_D_ERROR, "KernelErrataPatcherLoadImage: Loading %S...\n",
         ImagePath));

    // Bootmgr
    if (StrStr(ImagePath, L"bootaa64.efi") != 0) {
      DEBUG(
          (EFI_D_ERROR,
           "KernelErrataPatcherLoadImage: Detected Windows Boot Manager\n"));
    }

    // Winload
    if (StrStr(ImagePath, L"winload.efi") != 0) {
      DEBUG(
          (EFI_D_ERROR,
           "KernelErrataPatcherLoadImage: Detected Windows OS Loader\n"));
    }

    // Ntoskrnl
    if (StrStr(ImagePath, L"ntoskrnl.exe") != 0) {
      DEBUG(
          (EFI_D_ERROR,
           "KernelErrataPatcherLoadImage: Detected Windows OS Kernel\n"));
    }
  }

  return mEfiImageLoad(
      BootPolicy, ParentImageHandle, DevicePath, SourceBuffer, SourceSize,
      ImageHandle);
}

EFI_STATUS
EFIAPI
KernelErrataPatcherEntryPoint(
    IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  mEfiImageLoad = (EFI_IMAGE_LOAD)SetServicePointer(
      &gBS->Hdr, (VOID **)&gBS->LoadImage,
      (VOID *)&KernelErrataPatcherLoadImage);

  return EFI_SUCCESS;
}