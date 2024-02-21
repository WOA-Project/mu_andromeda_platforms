#!/bin/bash

# Andromeda855 FD
python3 ./Platforms/SurfaceDuo1Pkg/PlatformBuild.py TARGET=DEBUG
mv ./Build/SurfaceDuo1Pkg/DEBUG_CLANGDWARF/FV/SM8150_EFI.fd ./ImageResources/Epsilon/SM8150_EFI.fd

# Andromeda855 FD (SecureBoot Disabled)
python3 ./Platforms/SurfaceDuo1Pkg/PlatformBuildNoSb.py TARGET=DEBUG
mv ./Build/SurfaceDuo1Pkg/DEBUG_CLANGDWARF/FV/SM8150_EFI.fd ./ImageResources/Epsilon/SM8150_EFI_NOSB.fd