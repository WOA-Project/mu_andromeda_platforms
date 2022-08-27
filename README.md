# Project Mu UEFI Implementation for Surface Duo Platforms

## Build

Quick notes for building:

- Use Ubuntu 20.04 x64
- Generate ACPI tables with IASL
- Follow this quick draft

NOTE: The build scripts were designed to run in a CI, take inspiration from them, but the behavior on installs with compilers already installed/env vars set is undefined.

```
# Setup environment
sudo ./setup_env.sh

# Activate Workspace
python3 -m venv SurfaceDuo
source SurfaceDuo/bin/activate

# Stamp
./build_releaseinfo.ps1

# Build UEFI
pip install --upgrade -r pip-requirements.txt
./build_uefi.sh

# Generate ELF image
./build.sh
```

## Acknowledgements

- [EFIDroid Project](http://efidroid.org)
- Andrei Warkentin and his [RaspberryPiPkg](https://github.com/andreiw/RaspberryPiPkg)
- Sarah Purohit
- [Googulator](https://github.com/Googulator/)
- [Ben (Bingxing) Wang](https://github.com/imbushuo/)
- [Renegade Project](https://github.com/edk2-porting/)

## License

All code except drivers in `GPLDriver` directory are licensed under BSD 2-Clause. 
GPL Drivers are licensed under GPLv2 license.
