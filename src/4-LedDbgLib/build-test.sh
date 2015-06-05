#!/bin/bash  
export PATH=$PATH:"/home/mutex/Projects/dhARMa/toolchain/gcc-arm-none-eabi-4_9-2015q1/bin" 

#Note - the order of specifying the .s is important since we are not using the linker
arm-none-eabi-as test-led.s ledDbgLib.s -o testLedDbgLib.o
arm-none-eabi-objdump -SD testLedDbgLib.o > testLedDbgLib.dmp
arm-none-eabi-objcopy testLedDbgLib.o testLedDbgLib.bin -O binary

../../bin/signgp ./testLedDbgLib.bin
mv testLedDbgLib.bin.ift MLO
