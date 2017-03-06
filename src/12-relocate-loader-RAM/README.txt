12-relocate-loader-RAM
======================
This is an example of a simple loader which will relocate the entire
program to the DDR3 RAM and begine execution from there. It is done in 3 steps:
1. Test the GPIO1 usr led 0 pin, if its not set then it indicates
   that relocation has not happened, and the code sets up the stack on L3 RAM and initializes the DDR3.
2. init will then copy the first 100kb of L3 RAM to the beginning of DDR3 RAM.
   and then jump to the start of DD3 at 0x80000000 to begin re-execution of the
   relocated program from '_start'
3. So the test for GPIO1 usr led 0 pin would happen again and this time it would have been set as part
   of the init code to initialize DDR3. So, the startup code would now shift the bss and stack to DDR3
   and branch to main to continue the execution from DDR3 RAM.

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
4) sudo dd if=/dev/zero of=/dev/mmcblk0 bs=512K count=1 conv=notrunc,fsync --- clear out first 512k bytes of the disk with zeroes; [usually in BBB you can replace 'sdX' by 'mmcblk0']
5) sudo dd if=/root/MLO of=/dev/mmcblk0 count=1 seek=1 conv=notrunc bs=128k --- write the MLO at an offset of 128k
6) While holding the boot (S2) button remove power cable/USB cable and re-insert power while continuing to hold down S2.

Output:
1. usr0 led = 1; after a delay of 5-7s, indicates DDR3 init and relocation is successful
2. usr1 led = 1; indicates execution has now started from the relocated code in DDR3 RAM
3. usr2 led = 1; BSS segment functional
3. usr3 led = toggles; relocated code is executing properly