all: downloader/common/libcommon.so parser/common/libcommon.so


common/build/libcommon.so: common/src/*
	cd common && mkdir -p build && cd build && cmake .. && make -j 4 && cd ../..

downloader/common/libcommon.so: common/build/libcommon.so
	mkdir -p downloader/common/include
	cp $< $@
	cp common/src/*h downloader/common/include
	cp -R common/build/NatsCClient-prefix/include/* downloader/common/include

parser/common/libcommon.so: common/build/libcommon.so
	mkdir -p parser/common/include
	cp $< $@
	cp common/src/*h parser/common/include
	cp -R common/build/NatsCClient-prefix/include/* parser/common/include
