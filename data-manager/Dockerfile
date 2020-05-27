FROM ubuntu:18.04

RUN apt-get -y update && apt-get install -y cmake build-essential libuv1-dev libcurl4-openssl-dev git libssl-dev libprotobuf-dev protobuf-compiler libprotoc-dev autoconf automake libtool pkg-config libhiredis-dev librdkafka-dev
RUN git clone https://github.com/protobuf-c/protobuf-c.git && cd protobuf-c && ./autogen.sh && ./configure && make && make install

WORKDIR /app
ADD . .

RUN mkdir -p build && cd build && cmake .. && make -j4 && cd ../ && ldconfig

CMD ["/app/build/output/data_manager"]