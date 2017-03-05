8-C-Runtime-Led
===============================
We now switch to bare metal C programming. (whew ! finally)
For this the initial setup of the stack and BSS segment is done by the assembly code in start.s
and then jumps to C code in main.c
The BSS segment has to be zeroed out.
Note that in bare metal mode when compiling we have to mention some options to the gcc compiler:
-Wall = turn on all warnings, optional but very useful !
-O2 = optimization level 2, this is only optional
-nostdlib = do not use standard C libraries while compiling, because in bare metal there is no C runtime available
-ffreestanding = specifies that there is no underlying OS and c-runtime when the binary is run
-fstack-usage -Wstack-usage=256 : for ensuring a stack size of 256 is used.

The 'memmap' file is a linker script, required to specify the load address and BSS segments.

To build: 
1) Make sure you have built the signgp utility first.
2) Edit build.sh script to reflect your toolchain path
2) ./build.sh

To run:
This file has to be put onto an sd-card in raw format as described in the TI AM335x TRM - Sec 26.1.7.5.5
1) Get hold of a u-sdcard and card reader.
2) Delete all partitions on it and ensure there is only free space.
3) Let BBB boot into its default OS and insert the usd.
4) Copy the MLO to '/root' on BBB - 'scp MLO root@192.168.7.2:/root' (default password is blank)
4) login to BBB - 'ssh root@192.168.7.2' (default password is blank)
4) sudo dd if=/dev/zero of=/dev/sdX bs=512K count=1 --- clear out first 512k bytes of the disk with zeroes; [usually in BBB you can replace 'sdX' by 'mmcblk0']
5) sudo dd if=/root/MLO of=/dev/sdX count=1 seek=1 conv=notrunc bs=128k --- write the MLO at an offset of 128k
6) While holding the boot (S2) button remove power cable/USB cable and re-insert power while continuing to hold down S2.
7) You should see the usr3 led turn on

