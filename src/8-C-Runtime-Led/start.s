.equ STACK_SIZE, 256
.equ STACK_SUPERVISOR_START, 0x4030CDFC
.equ CM_PER_GPIO1_CLKCTRL, 0x44e000AC
.equ GPIO1_OE, 0x4804C134
.equ GPIO1_SETDATAOUT, 0x4804C194
.equ GPIO1_CLEARDATAOUT, 0x4804C190
.equ GPIO1_DATAOUT, 0x4804C13C

_start:
    mrs r0, cpsr
    bic r0, r0, #0x1F	@ clear mode bits
    orr r0, r0, #0x13	@ set supervisor mode
    orr r0, r0, #0xC0	@ disable FIQ and IRQ
    msr cpsr, r0
	
	/* set clock for GPIO1, TRM 8.1.12.1.31 */
    ldr r0, =CM_PER_GPIO1_CLKCTRL
    ldr r1, =0x02
    str r1, [r0]

    /*clear out gpio1-21,22,23,24 pins using data out register first*/
    ldr r0, =GPIO1_CLEARDATAOUT
    mov r1, #(0x0F<<21)
    str r1, [r0]

    /* set pin 21,22,23,24 for output, led USR0,1,2,3, TRM 25.3.4.3 */
    ldr r0, =GPIO1_OE
    ldr r1, [r0]
    bic r1, r1, #(0x0F<<21)
    str r1, [r0]
    
    /* turn on usr0 led */
    ldr r0, =GPIO1_SETDATAOUT
    mov r1, #(1<<21)
    str r1, [r0]
    
    @Setup supervisor mode stack 
    ldr sp, =STACK_SUPERVISOR_START
    mov r3, sp
    
    @switch to undefined exception mode and setup undefined exception stack
    mrs r1, cpsr
    bic r1, r1, #0x1F
    orr r1, r1, #0x1B
    msr cpsr, r1
    sub r3, r3, #STACK_SIZE
    mov sp, r3    

    @switch to irq mode and setup irq stack -- ARM sys dev guide -- 9.2.4
    mrs r1, cpsr
    bic r1, r1, #0x1F	    
    orr r1, r1, #0x12
    msr cpsr, r1
    sub r3, r3, #STACK_SIZE
    mov sp, r3

    @back to supervisor mode
    mrs r1, cpsr
    bic r1, r1, #0x1F	
    orr r1, r1, #0x13
    msr cpsr, r1
    
    @gcc requires zeroed out BSS segment (Block started by symbol - stores uninitalized static vars)
    ldr r0, =__bss_start__
	ldr r1, =__bss_size__
	add r1, r0
	mov r2, #0
ZLOOP:
	str r2, [r0]
	add r0, #4
	cmp r0, r1
	blt ZLOOP
	
	@branch to C code, 'main' function
	b main
