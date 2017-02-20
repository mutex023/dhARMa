#!/bin/bash  
export PATH="/home/kishore/Projects/dhARMa-git/toolchain/arm-2014.05/bin":"/home/kishore/Projects/dhARMa-git/toolchain/arm-2014.05/libexec/gcc/arm-none-eabi/4.8.3":"/home/kishore/Projects/dhARMa-git/toolchain/arm-2014.05/arm-none-eabi/bin/"

/usr/bin/make

../../bin/signgp mlo.bin
/bin/mv mlo.bin.ift MLO
