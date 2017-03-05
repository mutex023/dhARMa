10-RAM-stack
===============================
Extension of prog 9.
We now place the stack on the RAM and execute function calls on the RAM.
For this we first place the stack on internal L3 RAM, branch to the C code which initializes DDR3 RAM,
then return from the C code back to asm, where we switch the stack to the RAM address and then branch
back to C code.

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

Output:
1. usr0 led = 1; indicates branch to C-code init function from startup asm successful
2. usr1 led = 1; after a delay of around 5 sec, indicates RAM init and test successful
3. usr2 led = 1; jump back to asm startup code from C successful
4. usr3 led = 1; relocation of stack to RAM and branch back to C main function successful
5. usr1 led = 0; BSS segment functional

