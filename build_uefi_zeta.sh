#!/bin/bash

stuart_setup -c Platforms/SurfaceDuo2Pkg/PlatformBuild.py TOOL_CHAIN_TAG=CLANG38
stuart_update -c Platforms/SurfaceDuo2Pkg/PlatformBuild.py TOOL_CHAIN_TAG=CLANG38
stuart_build -c Platforms/SurfaceDuo2Pkg/PlatformBuild.py TOOL_CHAIN_TAG=CLANG38