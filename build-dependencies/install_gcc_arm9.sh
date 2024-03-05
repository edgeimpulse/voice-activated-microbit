#!/bin/bash
set -e

UNAME=`uname -m`

cd /
if [ "$UNAME" == "aarch64" ]; then
    wget https://cdn.edgeimpulse.com/build-system/gcc-arm-none-eabi-9-2019-q4-major-aarch64-linux.tar.bz2
    tar xjf gcc-arm-none-eabi-9-2019-q4-major-aarch64-linux.tar.bz2
else
    wget https://cdn.edgeimpulse.com/build-system/gcc-arm-none-eabi-9-2019-q4-major-x86_64-linux.tar.bz2
    tar xjf gcc-arm-none-eabi-9-2019-q4-major-x86_64-linux.tar.bz2
fi
echo "PATH=$PATH:/gcc-arm-none-eabi-9-2019-q4-major/bin" >> ~/.bashrc
