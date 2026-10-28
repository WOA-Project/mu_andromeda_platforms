# Loaders

- [`Direct/`](./Direct/) contains the standalone loader used with fastboot boot.
  It runs the UEFI firmware directly.
- [`DualBoot/`](./DualBoot/) contains the prepatched Android kernel loader
  artifacts used to select between Android and UEFI.

The shared runtime handoff shim, configuration files, and payload packager
are kept separately in [`BootConfig/`](../BootConfig/). The payload order is
shim, UEFI FD, then config; it is padded to each loader's existing copy size so
loader code and its memory-map reservation remain unchanged.
