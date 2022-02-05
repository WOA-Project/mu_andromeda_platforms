#!/bin/bash

stuart_setup -c Platforms/SurfaceDuoPkg/PlatformBuild.py TOOL_CHAIN_TAG=GCC5
stuart_update -c Platforms/SurfaceDuoPkg/PlatformBuild.py TOOL_CHAIN_TAG=GCC5
stuart_build -c Platforms/SurfaceDuoPkg/PlatformBuild.py TOOL_CHAIN_TAG=GCC5
