#!/bin/sh
if [ -d "build" ]; then
	rm -rf build
	mkdir build
fi

cd build
cmake ..
make
