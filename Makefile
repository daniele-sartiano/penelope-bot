all: downloader parser data-manager

.PHONY: downloader parser data-manager clean

common/build/libcommon.so: common/src/*
	cd common && mkdir -p build && cd build && cmake .. && make -j 4 && cd ../..

downloader: common/build/libcommon.so
	rm -rf $@/common
	mkdir -p $@/common/include
	cp $< $@/common/libcommon.so
	cp common/src/*h $@/common/include

parser: common/build/libcommon.so
	rm -rf $@/common
	mkdir -p $@/common/include
	cp $< $@/common/libcommon.so
	cp common/src/*h $@/common/include

data-manager: common/build/libcommon.so
	rm -rf $@/common
	mkdir -p $@/common/include
	cp $< $@/common/libcommon.so
	cp common/src/*h $@/common/include

clean:
	rm -rf downloader/3rdparty downloader/cmake-build-debug downloader/cmake-build-release downloader/common
	rm -rf parser/3rdparty parser/cmake-build-debug parser/cmake-build-release parser/common
	rm -rf data-manager/3rdparty data-manager/cmake-build-debug data-manager/cmake-build-release data-manager/common
	rm -rf common/3rdparty common/cmake-build-debug common/cmake-build-release common/build
