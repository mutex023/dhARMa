#!/bin/bash  
#export PATH=$PATH:"/home/svtuser/Projects/dhARMa/toolchain/gcc-arm-none-eabi-4_9-2015q1/bin"
export PATH=$PATH:"/home/svtuser/Projects/dhARMa/toolchain/arm-2014.05/bin/"

arm-none-eabi-as $1.s -o $1.o
arm-none-eabi-objcopy $1.o $1.bin -O binary
arm-none-eabi-objdump -SD $1.o > $1.obj

../../bin/signgp ./$1.bin
mv $1.bin.ift MLO
