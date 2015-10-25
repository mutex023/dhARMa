6-MLO-LedBlink-RTCInterrupt-RelocatedIntVec-Stack
=================================================
Slight addition to "5".
We now setup irq, exception-mode and supervisor-mode stacks
and use a stack in our irq handler

To build: 
1) Make sure you have built the signgp utility first.
2) Edit build.sh script to reflect your toolchain path
2) ./build.sh mlo

To run:
This file has to be put onto an sd-card in raw format as described in the TI AM335x TRM - Sec 26.1.7.5.5
1) Get hold of a u-sdcard and card reader.
2) Delete all partitions on it and ensure there is only free space.
3) Let BBB boot into its default OS and insert the usd.
4) Copy the MLO to '/root' on BBB - 'scp MLO root@192.168.7.2:/root' (default password is blank, changed to 'rootflash')
4) login to BBB - 'ssh root@192.168.7.2' (default password is blank)
4) sudo dd if=/dev/zero of=/dev/mmcblk0 bs=512K count=1 --- clear out first 512k bytes of the disk with zeroes; [usually in BBB the ext sdcard is 'mmcblk0', however better check in '/dev' once to see]
5) sudo dd if=/root/MLO of=/dev/mmcblk0 count=1 seek=1 conv=notrunc bs=128k --- write the MLO at an offset of 128k
6) While holding the boot (S2) button remove power cable/USB cable and re-insert power while continuing to hold down S2.
7) You should see the usr3 led toggle every second.

Some problems:
-------------

1. The irq handler executes inconsistently if the rtc 1 sec interrupt is not masked when the irq happens.
   This implies that the irq handler is taking more than 1 sec to execute ??? Very unlikely.
   Is it because of the stack operation addition or because of the branch to a sub routine ?

2. Besides, the ARM cortex TI manual says that ARM disables IRQ before the IRQ handler begins execution.
   In that case why should another irq be executed when the current one has not yet finished ??
   
3. Also, the data synchronisation barrier primitive is causing some more problems. If I enable that
   instruction, again I see the inconsistent execution of irq hdlr. and the led toggle on/off very quickly
   and sometimes very slowly.

