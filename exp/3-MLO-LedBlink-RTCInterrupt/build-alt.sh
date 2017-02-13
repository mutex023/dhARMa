#!/bin/bash  
export TOOLCHAIN_PATH="/usr/arm-linux-gnueabi/bin"

${TOOLCHAIN_PATH}/as $1.s -o $1.o
${TOOLCHAIN_PATH}/objcopy $1.o $1.bin -O binary
#${TOOLCHAIN_PATH}/objdump -SD $1.o

../../bin/signgp ./$1.bin
mv $1.bin.ift MLO
