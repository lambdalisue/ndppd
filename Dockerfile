FROM ubuntu:18.04

RUN apt-get update \
 && apt-get install -y --no-install-recommends g++-mipsel-linux-gnu make \
 && apt-get clean \
 && rm -rf /var/lib/apt/lists/*

CMD ["make", "PREFIX=/ndppd/local", "CXX=/usr/bin/mipsel-linux-gnu-g++", "LDFLAGS=-static", "CXXFLAGS=-O3 -static"]
