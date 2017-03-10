14-bootloader-durga
======================
We now have most of the pieces in place to write the first version of our bootloader 'durga'.
This is a stage-1 bootloader which will do the following:
1. Init UART for debugging
2. Init DDR3 RAM
3. Setup Interrupt handlers on RAM
   The setup here is slightly different from prog-11.
   Here instead of directly jumping to the interrupt handlers from the interrupt table
   we jump to a location that is specified below the interrupt table in the following way:
		
	0x9FFFFFC0	ldr pc, [pc, #24] ;reset_hdlr (remember that pc will be pointing 2 instr ahead due to pipelining)
	0x9FFFFFC4	ldr pc, [pc, #24] ;undef_hdlr
	0x9FFFFFC8	ldr pc, [pc, #24] ;swi_hdlr
	0x9FFFFFCC	ldr pc, [pc, #24] ;progabort_hdlr
	0x9FFFFFD0	ldr pc, [pc, #24] ;databort_hdlr
	0x9FFFFFD4	nop
	0x9FFFFFD8	ldr pc, [pc, #20] ;irq_hdlr
	0x9FFFFFDC	ldr pc, [pc, #20] ;fiq_hdlr
	0x9FFFFFE0	[addr of reset_hdlr]
	0x9FFFFFE4	[addr of undef_hdlr]
	0x9FFFFFE8	[addr of swi_hdlr]
	0x9FFFFFEC	[addr of progabort_hdlr]
	0x9FFFFFF0	[addr of databort_hdlr]
	0x9FFFFFF4	[addr of irq_hdlr]
	0x9FFFFFF8	[addr of fiq_hdlr]
	0x9FFFFFFC
	0xA0000000	--DDR3 RAM end
	
	This gives us the flexibility of allowing the user to register his own custom intr handlers.
	
4. Setup stack on RAM
6. Init RTC interrupts
7. Relocate itself to RAM and begin looking for an OS image
   This will involve reading a file from a file system and is a 'TODO' future work.
   For now durga will simply keep printing an ascii chart on the UART from the RTC interrupt handler

We shall keep adding code to Durga and fleshing it out as and when new modules are required.
For example - wall clock time, file system read, advanced intr handling,
hdmi interfacing (to show boot progress on screen), etc..

Another change is that the linker script has to be changed to force alignment of data and bss sections to
4-byte boundaries, otherwise if say a static u8 initialized variable is declared and the program
has some other u8 global uninitialized variables, then this will cause the bss to start at non-4-btye boundaries
leading to an exception when trying to zero out the bss in start.s
No need to align .rodata, as it starts soon after the data section and hence will be automatically aligned
since all instructions are 4 byte in ARM.

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
1. usr0 led = 1; after a delay of 5-7s - indicates DDR3 RAM has been initialized successfully
2. usr1 led = 1; UART init has finished successfully
3. usr2 led = 1; Stage1 loading sucessful
3. usr3 led = toggles; Stage2 loading complete and durga has booted fully
4. UART output = 'IRQ - RTC interrupt #75 - xx' -- keeps printing every sec
   implies, RTC and interrupt handlers are functional
