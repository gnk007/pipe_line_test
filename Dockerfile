FROM ubuntu:latest

RUN apt update

RUN apt install -y make autoconf build-essential

COPY bb-kvstore-master /bb-kvstore-master

RUN mkdir -v /install /build

RUN cd /bb-kvstore-master && autoreconf -i

RUN cd /build

RUN /bb-kvstore-master/configure --prefix=/install

RUN make -j4 install

ENTRYPOINT ["/install/bin/bb-kvstore_server"]
