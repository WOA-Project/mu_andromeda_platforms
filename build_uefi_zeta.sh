#!/bin/bash

python3 ./Platforms/SurfaceDuo2Pkg/PlatformBuild.py --setup TOOL_CHAIN_TAG=CLANG38
python3 ./Platforms/SurfaceDuo2Pkg/PlatformBuild.py --update TOOL_CHAIN_TAG=CLANG38
python3 ./Platforms/SurfaceDuo2Pkg/PlatformBuild.py --build TOOL_CHAIN_TAG=CLANG38