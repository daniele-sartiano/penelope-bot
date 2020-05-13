# penelope-bot
A distributed web crawler based on nats.io


## compile downloader locally

### dependencies
```
libprotobuf-dev protobuf-compiler libprotoc-dev
https://github.com/protobuf-c/protobuf-c

```

### commands

```
cd downloader; mkdir -p build; cd build; cmake ..; make; cd ../..
```


### Build

```
docker-compose up --scale downloader=3
```
