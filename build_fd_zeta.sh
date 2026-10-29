#!/bin/bash

# Andromeda888 FD
export BOARD=SurfaceDuo2
python3 ./Platforms/AndromedaPkg/PlatformBuild.py TARGET=RELEASE
mv ./Build/SurfaceDuo2Pkg/RELEASE_CLANGPDB/FV/SM8350_EFI.fd ./ImageResources/Zeta/SM8350_EFI.fd
