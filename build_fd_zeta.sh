#!/bin/bash

# Andromeda888 FD
python3 ./Platforms/SurfaceDuo2Pkg/PlatformBuild.py TARGET=RELEASE
mv ./Build/SurfaceDuo2Pkg/RELEASE_CLANGPDB/FV/SM8350_EFI.fd ./ImageResources/Zeta/SM8350_EFI.fd
