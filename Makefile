all: downloader parser data-manager

.PHONY: downloader parser data-manager

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
