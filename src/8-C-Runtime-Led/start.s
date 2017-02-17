@bare-metal startup asm code to setup stack and bss and branch to C code.
@author: muteX023

.equ STACK_SIZE, 256
.equ STACK_SUPERVISOR_START, 0x4030CDFC

_start:
    mrs r0, cpsr
    bic r0, r0, #0x1F	@ clear mode bits
    orr r0, r0, #0x13	@ set supervisor mode
    orr r0, r0, #0xC0	@ disable FIQ and IRQ
    msr cpsr, r0

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
	cmp r0, r1
	strlt r2, [r0]
	add r0, #4
	blt ZLOOP
	
	@branch to C code, 'main' function
	b main
