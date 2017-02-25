@bare-metal startup asm code to setup stack and bss and branch to C code which will
@init the RAM and return, the asm code resumes and shifts the stack to RAM and jumps
@back to the C code 'main' function to resume execution utilising the stack on the RAM
@
@author: muteX023

.equ STACK_SIZE, 256
.equ STACK_SUPERVISOR_START, 0x4030CDFC
.equ DDR3_RAM_END, 0xA0000000
.equ GPIO1_SETDATAOUT, 0x4804C194

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
   
	@we do not initialize bss here, because we want the BSS to be in RAM
	@so the init C code *MUST NOT* access any global/static vars till
	@the BSS segment has been intialized in RAM !!

	@branch to C code, 'init' function -- must branch with link !
	@since we intend to return back to asm !
	bl init
	
	@set usr2 led on
	ldr r4, =GPIO1_SETDATAOUT
	mov r5, #(1<<23)
	str r5, [r4]

	@now we need to move the stack to the DDR3 RAM end at 0xA0000000
	@the DDR3 RAM starts at 0x80000000 and is 512MB size.
	@by convention ARM stacks are descending in nature, so place
	@the stack at the end of the RAM.

	@Setup supervisor mode stack 
	ldr sp, =DDR3_RAM_END
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
	
	@ now back to C-code 'main' function
	b main
