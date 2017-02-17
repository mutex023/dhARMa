/*
Bare metal BeagleBone Black example for turning on led USR0.
Code taken from https://github.com/auselen/down-to-the-bone/tree/master/baremetal_led
@author: muteX023
*/

.equ CM_PER_GPIO1_CLKCTRL, 0x44e000AC
.equ GPIO1_RISINGDETECT, 0x44e00148
.equ GPIO1_OE, 0x4804C134
.equ GPIO1_SETDATAOUT, 0x4804C194
.equ GPIO1_CLEARDATAOUT, 0x4804C190
.equ GPIO1_DATAOUT, 0x4804C13C

_start:
    mrs r0, cpsr
    bic r0, r0, #0x1F @ clear mode bits
    orr r0, r0, #0x13 @ set SVC mode - supervisor mode
    orr r0, r0, #0xC0 @ disable FIQ and IRQ
    msr cpsr, r0

    /* set clock for GPIO1, TRM 8.1.12.1.31 */
    ldr r0, =CM_PER_GPIO1_CLKCTRL
    ldr r1, =0x40002
    str r1, [r0]

	/*clear out gpio1-21st pin using data out register first*/
    ldr r0, =GPIO1_CLEARDATAOUT
    ldr r1, =(1<<21)
    str r1, [r0]

    /* set pin 21 for output, led USR0, TRM 25.3.4.3 */
    /* don't touch other bits, as they're used for sd card IO !! */
    ldr r0, =GPIO1_OE
    ldr r1, [r0]
    bic r1, r1, #(1<<21)
    str r1, [r0]

    /*logical 1 turns on the led, TRM 25.3.4.2.2.2 */
    ldr r0, =GPIO1_SETDATAOUT
    ldr r1, =(1<<21)
    str r1, [r0]
	
	
.loop: b .loop	
