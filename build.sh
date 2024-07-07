#!/bin/sh

TEMPORARY_DIR=build

rm -rf $TEMPORARY_DIR
mkdir -p $TEMPORARY_DIR
cd $TEMPORARY_DIR

cmake ..
make
yes|cp bin/demo.bin ../

#sleep 1
#/toolchain/tools/toolchain/arm-sigmastar-linux-uclibcgnueabihf-9.1.0/bin/arm-sigmastar-linux-uclibcgnueabihf-9.1.0-strip build/bin/demo_cardv.bin
