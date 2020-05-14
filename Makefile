all: downloader/common/libcommon.so parser/common/libcommon.so


common/build/libcommon.so: common/src/*
	cd common && mkdir -p build && cd build && cmake .. && make -j 4 && cd ../..

downloader/common/libcommon.so: common/build/libcommon.so
	rm -rf downloader/common
	mkdir -p downloader/common/include
	cp $< $@
	cp common/src/*h downloader/common/include

parser/common/libcommon.so: common/build/libcommon.so
	rm -rf parser/common
	mkdir -p parser/common/include
	cp $< $@
	cp common/src/*h parser/common/include
