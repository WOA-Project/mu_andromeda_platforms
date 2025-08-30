#!/bin/bash

sudo apt update
sudo apt install -y uuid-dev clang llvm gcc-aarch64-linux-gnu lld

#cargo install --force cargo-make
#cargo add cargo-tarpaulin

# Workaround an issue with bundled nuget and new enough Ubuntu versions (like the one used in the CI)
sudo sh -c 'echo "deb http://archive.ubuntu.com/ubuntu jammy main universe" > /etc/apt/sources.list.d/jammy.list'
sudo apt-get update
sudo apt-get install -y mono-complete nuget make

# Install and Upgrade all needed python packages
pip install --upgrade -r ./pip-requirements.txt

python3 ./Platforms/SurfaceDuo1Pkg/PlatformBuild.py --setup -t RELEASE
python3 ./Platforms/SurfaceDuo1Pkg/PlatformBuild.py --update -t RELEASE
