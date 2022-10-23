# Patched Binaries

This file aims to provide further information about the different patches applied to Surface Duo 2 stock firmware UEFI DXEs.

## Reasoning behind each patch

- UFSDxe: An MMU Domain is already setup by the previous firmware and gets re-set again, causing a crash.

- UsbConfigDxe: Important to get USB to work after exit boot services for KdNet or DeveloperMenu or FFULoader.

- ButtonsDxe: to help navigating menus more easily.

## UFSDxe & DisplayDxe

MMU related setup routine was patched to not recreate already existing MMU domains.

## DisplayDxe

Panel Reset function was patched to not run again.

Exit BootServices routine was patched to not deinitialize the panels.

Patch is still Work in Progress, and isn't needed currently.

## UsbConfigDxe

Exit BootServices routine was patched to not deinit USB after exit boot services. Another patch disables recreating IOMMU domains for USB given detaching devices seems broken under Haven.

## ButtonsDxe

Key code was patched for the power button to be mapped as ENTER instead of SUSPEND.

## PmicDxe

PMIC AUX (LEICA) init sequence was patched to not fail due to the PMIC AUX chip already being powered on.

## ClockDxe

DCD Dependency enablement path was patched to not cause MDSS to reinitialize and thus lose framebuffer.

## GpiButtons

DXE Taken from Surface Duo 1, patched to sort of run on Surface Duo 2, vol up is broken. DXE is still unused currently

## FeatureEnablerDxe & MinidumpTADxe

Both DXEs were patched to not start again the TZ applet given it was already brought up

## QcomWDogDxe

Dependency check routine was patched to not fail due to ReturnStatusCodeHandler implementation being different.

## ManufacturingModeDxe & MsAbstractLayerDxe
The current PCD Token Store in our UEFI is as follows:

```
PcdTokenNumber: 
('PcdSKUEnableDfci', 'gDfciPkgTokenSpaceGuid') : 1
('PcdSetupUiReducedFunction', 'gDfciPkgTokenSpaceGuid') : 2
('PcdDeviceStateBitmask', 'gEfiMdeModulePkgTokenSpaceGuid') : 3
('PcdNvStoreDefaultValueBuffer', 'gEfiMdeModulePkgTokenSpaceGuid') : 9
('PcdSetNvStoreDefaultId', 'gEfiMdeModulePkgTokenSpaceGuid') : 10
('PcdTestKeyUsed', 'gEfiMdeModulePkgTokenSpaceGuid') : 4
('PcdVideoHorizontalResolution', 'gEfiMdeModulePkgTokenSpaceGuid') : 5
('PcdVideoVerticalResolution', 'gEfiMdeModulePkgTokenSpaceGuid') : 6
('PcdVpdBaseAddress64', 'gEfiMdeModulePkgTokenSpaceGuid') : 11
('PcdCurrentPointerState', 'gMsGraphicsPkgTokenSpaceGuid') : 12
('PcdPostBackgroundColoringSkipCount', 'gMsGraphicsPkgTokenSpaceGuid') : 13
('PcdStr3', 'gQcomPkgTokenSpaceGuid') : 7
('PcdStr4', 'gQcomPkgTokenSpaceGuid') : 8
```

MsAbstractLayerDxe depends on the following Dynamic Pcds:
```
('PcdStr3', 'gQcomPkgTokenSpaceGuid') : 3
('PcdStr4', 'gQcomPkgTokenSpaceGuid') : 4
```

ManufacturingModeDxe depends on the following Dynamic Pcds:
```
('PcdStr3', 'gQcomPkgTokenSpaceGuid') : 3
('PcdStr4', 'gQcomPkgTokenSpaceGuid') : 4
```

The DXEs were patched to check for 7 and 8 instead of 3 and 4 respectively.