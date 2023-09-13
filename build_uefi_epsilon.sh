#!/bin/bash

python3 ./Platforms/SurfaceDuo1Pkg/PlatformBuild.py --setup TOOL_CHAIN_TAG=CLANG38
python3 ./Platforms/SurfaceDuo1Pkg/PlatformBuild.py --update TOOL_CHAIN_TAG=CLANG38
python3 ./Platforms/SurfaceDuo1Pkg/PlatformBuild.py --build TOOL_CHAIN_TAG=CLANG38