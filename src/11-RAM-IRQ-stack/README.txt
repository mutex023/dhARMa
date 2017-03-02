11-RAM-STACK-IRQ
===============================
Now shift the interrupt table and handlers also to the RAM along with the stack as per the following
memory map:

	0xA0000000  ---> ldr pc, reset_hdlr ;intr_table() - interrupt table start
	0x9FFFFFFF ---> ldr pc, undef_hdlr
	.
	.
	.
	0x9FFFFFD2 ---> ldr pc, irq_hdlr
	0x9FFFFFCE ---> ldr pc, fiq_hdlr ;interrupt table end
	0x9FFFFFCA ---> reset_hdlr() ; individual handlers (assuming a 0x256 bytes size for each function
	.								for illustrative purposes only !)
	.
	.
	0x9FFFFD74 ---> undef_hdlr()
	.
	.
	0x9FFFFB1E ---> swi_hdlr()
	.
	.
	.
	.
	.
	0x9FFFF1C6 ---> irq_hdlr()
	.
	.
	0x9FFFEF70 ---> fiq_hdlr() - end of fiq_hdlr()
	.
	.
	.
	.
	.			---> start of fiq_hdlr()
	0x9FFFED1A ---> stack starts from here and grows downwards
	.			|
	.			|
	.			|
	.			|
	.
	.
	.
	.
	.			|
	.			|
	.			|
	.			V
	0x80000000 ---> DDR3 RAM start
	.
	.
	.
	.
	0x4030FFFF ---> L3 RAM + SRAM end
	.
	.			^
	.			|
	.			|	intr_table()	-----
	.			|	.					|
	.			|	.					|
	.			|	irq_hdlr()			|-----> These will be relocated (copied)
	.			|						|		to DDR3 RAM as above
	.			|	fiq_hdlr()		-----
	.			|
	.			|   main()
	.			|
	.			|   init()
	.			|
	0x402F0400 ---> L3 RAM start - _start: 
					code section of our program (MLO) starts from here and grows upwards

The above relocation is quite tricky since it relies on the order in which the functions
are written in C and is based on the assumption that the assembler will generate code in
the same order.
The interrupt table uses 'ldr' instructions which will actually generate a pc relative addressing
instruction in the table to jump to irq/fiq/reset handlers. But this pc relative addressing will work
only if we maintain the same ordering of the code section when copying from L3 RAM to DDR RAM.

Like in prog10, the stack is first on the L3 RAM, and later shifted to DDR3 RAM.
In the main() function we will keep toggling the usr1 led in a delay loop.
Meanwhile the 1-sec RTC interrupt will cause the usr-0 led to also toggle.
This will ensure that both the relocated intr handlers and the stack on the RAM is continuously
tested.


To build: 
1) Make sure you have built the signgp utility first.
2) Edit build.sh script to reflect your toolchain path
2) ./build.sh mlo

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
1. usr0 led toggles every second - RTC 1 second interrupt and interrupt table/handler relocation to RAM successful
1. usr1 led toggles every second - stack relocation to RAM successful.

