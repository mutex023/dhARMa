1-MLO-usdBoot:
==============

This is a bare metal ARM assembly program to just turn on the USR0 blue led on the BBB.
The actual binary generated will be prefixed with a TOC and GP header using the 'signgp' utility.
This final binary is called MLO - Minimal loader.
The actual purpose of an MLO is to chain load the second stage bootloader which will futher boot the kernel.
But here we're just trying to boot the BBB using our MLO from the external u-sdcard.
The MLO here just acts as a container for bare metal assembly code.

To build: 
1) Make sure you have built the signgp utility first.
2) ./build.sh mlo

To run:
This file has to be put onto an sd-card in raw format as described in the TI AM335x TRM - Sec 26.1.7.5.5
1) Get hold of a u-sdcard and card reader.
2) Delete all partitions on it and ensure there is only free space.
3) sudo dd if=/dev/zero of=/dev/sdX bs=512K count=1 --- clear out first 512k bytes of the disk with zeroes
4) sudo dd if=MLO of=/dev/sdX count=1 seek=1 conv=notrunc bs=128k --- write the MLO at an offset of 128k
5) Let BBB boot into its default OS and insert the usd.
6) While holding the boot (S2) button remove power cable/USB cable and re-insert power while continuing to hold down S2.
7) You should see the USR0 led glow
