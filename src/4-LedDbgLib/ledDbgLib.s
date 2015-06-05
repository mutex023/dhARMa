/*
Bare metal BeagleBone Black usr led debugging library
*/

.include "../common/bbbAm335xAddr.s"
.equ LOOP_CTR, 4807692

@@@@@@@@@@@@@@@@@@@ PROC_DELAY @@@@@@@@@@@@@@@@@@@@@@@@@@ 
@procedure to cause an n-sec blocking delay
@params - r7 - how many secs delay ?
@regs used - r7, r8, r9
@DOES NOT SAVE REGS BEFORE USING THEM!!   
/*Delay of 1s - some dummy instructions executed in a tight loop
ALU instructions take 1 cycle to exec, branch takes 3 cycles -- see Sec 6.3 in ARM system developers guide
Extrapolate that to the 1ghz AM3358 processor on BBB for a loop count to get a 1s delay,
but in reality the TI boot rom code would have set the proc to run at 500mhz instead of 1ghz*/

PROC_DELAY:
	ldr r9, =LOOP_CTR
	mul r8, r9, r7
DELAY_LOOP1:
    add r7, r7, r9
    add r7, r7, r9
    add r7, r7, r9
    add r7, r7, r9
    add r7, r7, r9
    add r7, r7, r9
    add r7, r7, r9
    add r7, r7, r9      
    sub r8, #1
    cmp r8, #0
	bne DELAY_LOOP1
   
    /*return to caller*/
    mov pc, lr
    
@@@@@@@@@@@@@@@@ PROC_LED_INIT @@@@@@@@@@@@@@@@@@@@@@@@@@@
@procedure to initialise the usr0-3 leds, must be called first before calling any of the procedures below !
@params - none
@regs used - r0, r1
@DOES NOT SAVE REGS BEFORE USING THEM!!

PROC_LED_INIT:
    /* set clock for GPIO1, TRM 8.1.12.1.31 */
    ldr r0, =CM_PER_GPIO1_CLKCTRL
    mov r1, #0x02
    str r1, [r0]

	/*set logical 0 on gpio1-21,22,23,24 pins to turn off all led's first*/
    ldr r0, =GPIO1_DATAOUT
    mov r1, #0
    str r1, [r0]

    /* set pins 21,22,23,24  for output TRM 25.3.4.3 */
    ldr r0, =GPIO1_OE
    ldr r1, [r0]
    bic r1, r1, #(0x0F<<21)
    str r1, [r0]	
    
    mov pc, lr
    
    
@@@@@@@@@@@@@@@@@@ PROC_LED_BINARY @@@@@@@@@@@@@@@@@@@@@@@@@@@    
@procedure to output a binary number on the usr leds
@params - r12 - a 4 bit binary number in bits0-3, rest should be zeroed out.
@regs used - r12, r11
@DOES NOT SAVE REGS BEFORE USING THEM!!

PROC_LED_BINARY:
	ldr r11, =GPIO1_DATAOUT
	lsl r12, #21
	str r12, [r11]
	
	mov pc, lr	

@@@@@@@@@@@@@@ PROC_LED_ONOFF @@@@@@@@@@@@@@@@@@@@@@@@@        
@procedure to turn on/off a specific usr led
@params - r12 - a value from 0-3 which among the usr0-3 leds should be turned off/on
@params - r11 - a value 0 or 1, indicating whether the led should be off(0) or on(1)
@regs used - r12, r11, r10, r9
@DOES NOT SAVE REGS BEFORE USING THEM!!

PROC_LED_ONOFF:
	add r12, #21
    mov r10, #1
    mov r10, r10, lsl r12
    ldr r12, =GPIO1_DATAOUT
    ldr r9, [r12]
    cmp r11, #1		@whether to turn on or off
    beq LEDON
    bic r9, r9, r10
    b DONE
LEDON:
    orr r9, r9, r10
DONE:    
    str r9, [r12] 
    
    mov pc, lr


@@@@@@@@@@@@@@@ PROC_LED_TOGGLE @@@@@@@@@@@@@@@@@@@@@@@@@@@
@procedure to toggle a specific usr led
@params - r12 - a value from 0-3 indicating which among the usr0-3 leds should be toggled
@regs used - r12, r11, r10
@DOES NOT SAVE REGS BEFORE USING THEM!!

PROC_LED_TOGGLE:
	add r12, #21
    mov r11, #1
    mov r11, r11, lsl r12
    ldr r12, =GPIO1_DATAOUT
  	ldr r10, [r12]
  	eor r10, r10, r11
	str r10, [r12]
	
	mov pc, lr


@@@@@@@@@@@@@@@ PROC_LED_BLINK @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@   
@procedure to blink a binary pattern on the usr leds
@params - r12 - a 4 bit binary number in bits0-3, rest should be zeroed out.
@params - r11 - the number of times to blink
@regs used - r12, r11, r10, r5, r6
@DOES NOT SAVE REGS BEFORE USING THEM!!

PROC_LED_BLINK:
	@store lr
	mov r6, lr
	
	ldr r10, =GPIO1_DATAOUT
	lsl r12, #21
	str r12, [r10]	@set the pattern first
	
	add r11, r11
LOOP_FLASH:
	ldr r5, [r10]
	eor r5, r5, r12
	str r5, [r10]
	
	mov r7, #1
	bl PROC_DELAY
	
	subs r11, #1
	bne LOOP_FLASH
	
	mov lr, r6
	mov pc, lr	
	
@@@@@@@@@@@@@@@@@ PROC_LED_PANIC @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@procedure to signal panic/assert, RETURNS AFTER A VERY LONG TIME !
@params - none
@regs used - r12, r11, r4
@DOES NOT SAVE REGS BEFORE USING THEM!!

PROC_LED_PANIC:
	mov r4, lr
	mov r12, #0x0F
	ldr r11, =0x0FFFFFFF
	bl PROC_LED_BLINK

	mov lr, r4
	mov pc, lr
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@	 


    
