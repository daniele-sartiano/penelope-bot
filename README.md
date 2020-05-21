# penelope-bot
A distributed C++ web crawler based on docker, nats.io and Scylla db.

## Modules

- Downloder
- Parser
- Data Manager
- Common

### Build and Run

```
make
docker-compose up --scale downloader=3
```


### Downloader

#### compile downloader locally

#### dependencies
```
libprotobuf-dev protobuf-compiler libprotoc-dev
https://github.com/protobuf-c/protobuf-c

```

#### commands

```
cd downloader; mkdir -p build; cd build; cmake ..; make; cd ../..
```
