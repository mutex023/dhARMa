13-UART-serial
===============================
This prog requires that you have the Standard FTDI USB2Serial cable mentioned in BBB SRM Sec 7.5
(http://elinux.org/Beagleboard:BeagleBone_Black_Serial).
Make sure to connect pin-1 of the cable(black wire) to pin-1 on the board.
We initialize the UART and output a string onto the serial console.
Use the linux 'minicom' utility to see the output on a linux host - 'sudo minicom -D /dev/ttyUSB0 -b 115200'

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
1. All 4 usr leds should turn on indicating successful init of uart.
2. You should see the string - 'dhARMa !' being printed on minicom terminal.

