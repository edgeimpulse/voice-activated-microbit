FROM ubuntu:xenial-20210114

ENV DEBIAN_FRONTEND noninteractive
RUN apt update && apt install -yq --no-install-recommends \
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
        rsync \
    && apt autoremove \
    && apt clean

# Install GCC ARM
COPY build-dependencies/install_gcc_arm9.sh install_gcc_arm9.sh
RUN /bin/bash install_gcc_arm9.sh && \
    rm install_gcc_arm9.sh
ENV PATH="/gcc-arm-none-eabi-9-2019-q4-major/bin:${PATH}"

WORKDIR /app

COPY . ./

RUN python build.py

CMD ["/bin/bash", "/data/build.sh"]
