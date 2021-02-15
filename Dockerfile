FROM ubuntu:xenial-20210114

ENV DEBIAN_FRONTEND noninteractive
RUN apt-get update && apt-get -yq dist-upgrade \
    && apt-get install -yq --no-install-recommends \
    wget \
    cmake protobuf-compiler \
    build-essential \
    ca-certificates \
    gcc \
    git \
    libpq-dev \
    make \
    python-pip \
    python2.7 \
    python2.7-dev \
    && apt-get autoremove \
    && apt-get clean

RUN mkdir -p /app-gcc

RUN wget -q https://developer.arm.com/-/media/Files/downloads/gnu-rm/9-2020q2/gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz2 \
    && tar -xf gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz2 -C /app-gcc --strip-components=1 \
    && rm gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz2

ENV PATH="/app-gcc/bin:${PATH}"

RUN apt install -y rsync

WORKDIR /app

COPY . ./

RUN python build.py

CMD ["/bin/bash", "/data/build.sh"]
