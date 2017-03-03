#!/bin/bash  
export PATH="/home/svtuser/Projects/dhARMa/toolchain/arm-2014.05/bin":"/home/svtuser/Projects/dhARMa/toolchain/arm-2014.05/libexec/gcc/arm-none-eabi/4.8.3":"/home/svtuser/Projects/dhARMa/toolchain/arm-2014.05/arm-none-eabi/bin/"

/usr/bin/make

../../bin/signgp mlo.bin
/bin/mv mlo.bin.ift MLO
