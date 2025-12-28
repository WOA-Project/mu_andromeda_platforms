# Patched Binaries

This file aims to provide further information about the different patches applied to Surface Duo 2 stock firmware UEFI DXEs.

## Reasoning behind each patch

- UsbConfigDxe: Important to get USB to work after exit boot services for KdNet or DeveloperMenu or FFULoader.

## DisplayDxe

MMU related setup routine was patched to not recreate already existing MMU domains.

## DisplayDxe

Panel Reset function was patched to not run again.

Exit BootServices routine was patched to not deinitialize the panels.

Patch is still Work in Progress, and isn't needed currently.

## UsbConfigDxe

Exit BootServices routine was patched to not deinit USB after exit boot services. Another patch disables recreating IOMMU domains for USB given detaching devices seems broken under Haven.

## PmicDxe

PMIC AUX (LEICA) init sequence was patched to not fail due to the PMIC AUX chip already being powered on.

## ClockDxe

DCD Dependency enablement path was patched to not cause MDSS to reinitialize and thus lose framebuffer.

## GpioButtons

DXE Taken from Surface Duo (1st Gen), patched to sort of run on Surface Duo 2, vol up is broken. DXE is still unused currently

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
('PcdConOutColumn', 'gEfiMdeModulePkgTokenSpaceGuid') : 3
('PcdConOutRow', 'gEfiMdeModulePkgTokenSpaceGuid') : 4
('PcdDeviceStateBitmask', 'gEfiMdeModulePkgTokenSpaceGuid') : 5
('PcdTestKeyUsed', 'gEfiMdeModulePkgTokenSpaceGuid') : 6
('PcdVideoHorizontalResolution', 'gEfiMdeModulePkgTokenSpaceGuid') : 7
('PcdVideoVerticalResolution', 'gEfiMdeModulePkgTokenSpaceGuid') : 8
('PcdTestMfgModeRequiredEKU', 'gQcomPkgTokenSpaceGuid') : 9
('PcdTkRequiredEKU', 'gQcomPkgTokenSpaceGuid') : 10
('PcdNvStoreDefaultValueBuffer', 'gEfiMdeModulePkgTokenSpaceGuid') : 11
('PcdSetNvStoreDefaultId', 'gEfiMdeModulePkgTokenSpaceGuid') : 12
('PcdVpdBaseAddress64', 'gEfiMdeModulePkgTokenSpaceGuid') : 13
('PcdCurrentPointerState', 'gMsGraphicsPkgTokenSpaceGuid') : 14
('PcdPostBackgroundColoringSkipCount', 'gMsGraphicsPkgTokenSpaceGuid') : 15
```

MsAbstractLayerDxe depends on the following Dynamic Pcds:
```
('PcdTestMfgModeRequiredEKU', 'gQcomPkgTokenSpaceGuid') : 3
('PcdTkRequiredEKU', 'gQcomPkgTokenSpaceGuid') : 4
```

ManufacturingModeDxe depends on the following Dynamic Pcds:
```
('PcdTestMfgModeRequiredEKU', 'gQcomPkgTokenSpaceGuid') : 3
('PcdTkRequiredEKU', 'gQcomPkgTokenSpaceGuid') : 4
```

The DXEs were patched to check for 9 and 10 instead of 3 and 4 respectively.