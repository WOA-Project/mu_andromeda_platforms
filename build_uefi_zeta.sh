#!/bin/bash

python3 ./Platforms/SurfaceDuo2Pkg/PlatformBuild.py --setup -t RELEASE
python3 ./Platforms/SurfaceDuo2Pkg/PlatformBuild.py --update -t RELEASE
python3 ./Platforms/SurfaceDuo2Pkg/PlatformBuild.py TARGET=RELEASE