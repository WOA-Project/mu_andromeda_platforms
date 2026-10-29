#!/bin/bash

# Andromeda855 FD
export BOARD=SurfaceDuo1
python3 ./Platforms/AndromedaPkg/PlatformBuild.py TARGET=RELEASE
mv ./Build/SurfaceDuo1Pkg/RELEASE_CLANGPDB/FV/SM8150_EFI.fd ./ImageResources/Epsilon/SM8150_EFI.fd
