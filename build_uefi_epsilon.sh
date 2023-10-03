#!/bin/bash

python3 ./Platforms/SurfaceDuo1Pkg/PlatformBuild.py --setup -t RELEASE
python3 ./Platforms/SurfaceDuo1Pkg/PlatformBuild.py --update -t RELEASE
python3 ./Platforms/SurfaceDuo1Pkg/PlatformBuild.py TARGET=RELEASE