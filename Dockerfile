# syntax = docker/dockerfile:experimental
FROM python:3.7.5-stretch

WORKDIR /app

# APT packages
RUN apt update && apt install -y curl xxd zip build-essential wget cmake

RUN cd .. && \
    wget https://armkeil.blob.core.windows.net/developer/Files/downloads/gnu-rm/9-2019q4/RC2.1/gcc-arm-none-eabi-9-2019-q4-major-x86_64-linux.tar.bz2 && \
    tar xjf gcc-arm-none-eabi-9-2019-q4-major-x86_64-linux.tar.bz2 && \
    echo "PATH=$PATH:/gcc-arm-none-eabi-9-2019-q4-major/bin" >> ~/.bashrc && \
    cd /app

ENV PATH="/gcc-arm-none-eabi-9-2019-q4-major/bin:${PATH}"

COPY . ./

RUN python build.py

ENTRYPOINT [ "/bin/bash", "build.sh" ]
