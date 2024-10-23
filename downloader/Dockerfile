FROM ubuntu:22.04

ENV TZ=Europe/Rome DEBIAN_FRONTEND=noninteractive

RUN apt-get -y update && apt-get install -y cmake build-essential libcurl4-openssl-dev git libssl-dev libprotobuf-dev protobuf-compiler libprotoc-dev autoconf automake libtool pkg-config libhiredis-dev librdkafka-dev
RUN git clone https://github.com/protobuf-c/protobuf-c.git && cd protobuf-c && ./autogen.sh && ./configure && make && make install

WORKDIR /app
ADD . .

RUN mkdir -p build && cd build && cmake .. && make && cd ../ && ldconfig

CMD ["/app/build/output/downloader"]
#CMD ["tail", "-f", "/dev/null"]

