/*
Bare metal BeagleBone Black example for turning on usr led's in a grand prix/F1 traffic light fashion.
@author: muteX023
*/

.equ CM_PER_GPIO1_CLKCTRL, 0x44e000AC
.equ GPIO1_RISINGDETECT, 0x44e00148
.equ GPIO1_OE, 0x4804C134
.equ GPIO1_SETDATAOUT, 0x4804C194
.equ GPIO1_CLEARDATAOUT, 0x4804C190
.equ GPIO1_DATAOUT, 0x4804C13C
.equ LOOP_CTR, 9615384

_start:
    mrs r0, cpsr
    bic r0, r0, #0x1F @ clear mode bits
    orr r0, r0, #0x13 @ set SVC mode - supervisor mode
    orr r0, r0, #0xC0 @ disable FIQ and IRQ
    msr cpsr, r0

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
    
    ldr r0, =GPIO1_DATAOUT    
START:
	/*branch with link, addr. of the instr. next to the branch is stored in LR before branching*/
	bl DELAY

	/*turn on led 4, i.e, the 24th gpio1 pin => 24th bit in the reg*/
    ldr r1, =(0xFF000000)
    str r1, [r0]
    bl DELAY
    
    /*turn on led 3*/
    lsr r1, #1
    str r1, [r0]
    bl DELAY
    
    /*turn on led 2*/
    lsr r1, #1
    str r1, [r0]
    bl DELAY
    
    /*turn on led 1*/
    lsr r1, #1
    str r1, [r0]
    bl DELAY
    
    //clear all leds
    mov r1, #0
    str r1, [r0]
        
	b START 
    
    /*Delay of 1s - some dummy instructions executed in a tight loop
    ALU instructions take 1 cycle to exec, branch takes 3 cycles -- see Sec 6.3 in ARM system developers guide
    During the public ROM boot, the MPU (u-processor unit) clock is set at 500Mhz, take this into consideration
    before calculating the loop counter value below - see TRM sec 26.1.4.2*/
DELAY:
	ldr r8, =LOOP_CTR
DELAY_LOOP1:
    add r2, r3, r4
    add r5, r6, r7
    add r2, r3, r4
    add r5, r6, r7
    add r2, r3, r4
    add r5, r6, r7
    add r2, r3, r4
    add r5, r6, r7      
    sub r8, #1
    cmp r8, #0
	bne DELAY_LOOP1
   
    /*return to caller*/
    mov pc, lr
    
