#!/bin/bash

# Andromeda888 FD
python3 ./Platforms/SurfaceDuo2Pkg/PlatformBuild.py TARGET=RELEASE
mv ./Build/SurfaceDuo2Pkg/RELEASE_CLANGDWARF/FV/SM8350_EFI.fd ./ImageResources/Zeta/SM8350_EFI.fd

# Andromeda888 FD (SecureBoot Disabled)
python3 ./Platforms/SurfaceDuo2Pkg/PlatformBuildNoSb.py TARGET=RELEASE
mv ./Build/SurfaceDuo2Pkg/RELEASE_CLANGDWARF/FV/SM8350_EFI.fd ./ImageResources/Zeta/SM8350_EFI_NOSB.fd