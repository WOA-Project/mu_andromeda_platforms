#!/bin/bash

sudo apt update
sudo apt install -y uuid-dev clang llvm gcc-aarch64-linux-gnu lld ca-certificates gnupg make

#cargo install --force cargo-make
#cargo add cargo-tarpaulin