CROSS = arm-none-eabi-
CFLAGS = -Wall -nostdlib -nostartfiles -ffreestanding -fstack-usage -Wstack-usage=1024
#$(CROSS)ld -o mlo -T memmap start.o main.o

all :
	$(CROSS)as start.s -o start.o
	$(CROSS)gcc $(CFLAGS) -c durga.c -o durga.o
	$(CROSS)gcc $(CFLAGS) -c ../common/utils.c -o utils.o
	$(CROSS)gcc $(CFLAGS) -c ../common/bbb_hal.c -o bbb_hal.o
	$(CROSS)ld -o mlo -T memmap start.o bbb_hal.o durga.o utils.o
	$(CROSS)objdump --disassemble-zeroes -M reg-names-raw -D mlo > mlo.obj
	$(CROSS)objcopy mlo mlo.bin -O binary

clean :
	@/bin/rm *.o *.obj *.bin *.su mlo MLO *.out
