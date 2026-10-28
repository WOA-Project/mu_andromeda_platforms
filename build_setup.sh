#!/bin/bash

sudo apt update
sudo apt install -y uuid-dev clang llvm gcc-aarch64-linux-gnu lld

#cargo install --force cargo-make
#cargo add cargo-tarpaulin

# Mono repository setup
sudo apt install ca-certificates gnupg
sudo gpg --homedir /tmp --no-default-keyring --keyring gnupg-ring:/usr/share/keyrings/mono-official-archive-keyring.gpg --keyserver hkp://keyserver.ubuntu.com:80 --recv-keys 3FA7E0328081BFF6A14DA29AA6A19B38D3D831EF
sudo chmod +r /usr/share/keyrings/mono-official-archive-keyring.gpg
echo "deb [signed-by=/usr/share/keyrings/mono-official-archive-keyring.gpg] https://download.mono-project.com/repo/ubuntu stable-focal main" | sudo tee /etc/apt/sources.list.d/mono-official-stable.list
sudo apt update

# Install Mono (for nuget), Nuget, and make
sudo apt-get install -y mono-complete nuget make

# Install and Upgrade all needed python packages
pip install --upgrade -r ./pip-requirements.txt

python3 ./Platforms/SurfaceDuo1Pkg/PlatformBuild.py --setup -t RELEASE
python3 ./Platforms/SurfaceDuo1Pkg/PlatformBuild.py --update -t RELEASE
