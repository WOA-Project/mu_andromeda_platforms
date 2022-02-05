#!/bin/bash

apt-get update

# Mono
apt-get install -y gnupg ca-certificates
apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv-keys 3FA7E0328081BFF6A14DA29AA6A19B38D3D831EF
echo "deb https://download.mono-project.com/repo/ubuntu stable-focal main" | sudo tee /etc/apt/sources.list.d/mono-official-stable.list

apt-get update

apt-get install -y mono-devel

# Host utilities
apt-get install -y python3 python3-venv python3-pip python git-core git build-essential

# PowerShell
# Import the public repository GPG keys
curl https://packages.microsoft.com/keys/microsoft.asc | apt-key add -

# Register the Microsoft Ubuntu repository
curl https://packages.microsoft.com/config/ubuntu/20.04/prod.list | tee /etc/apt/sources.list.d/microsoft.list

# Update the list of products
apt-get update

# Install PowerShell
apt-get install -y powershell

# Linaro Toolchains
cd /opt
wget http://releases.linaro.org/components/toolchain/binaries/7.5-2019.12/aarch64-elf/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-elf.tar.xz
wget http://releases.linaro.org/components/toolchain/binaries/7.4-2019.02/aarch64-linux-gnu/gcc-linaro-7.4.1-2019.02-x86_64_aarch64-linux-gnu.tar.xz
tar xf gcc-linaro-7.5.0-2019.12-x86_64_aarch64-elf.tar.xz
tar xf gcc-linaro-7.4.1-2019.02-x86_64_aarch64-linux-gnu.tar.xz
sudo update-alternatives --install /usr/bin/gcc gcc /opt/gcc-linaro-7.4.1-2019.02-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-gcc 70
sudo update-alternatives --install /usr/bin/gcc-ar gcc-ar /opt/gcc-linaro-7.4.1-2019.02-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-gcc-ar 70
sudo update-alternatives --install /usr/bin/objcopy objcopy /opt/gcc-linaro-7.4.1-2019.02-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-objcopy 70
export PATH="$PATH:/opt/gcc-linaro-7.4.1-2019.02-x86_64_aarch64-linux-gnu/bin"
