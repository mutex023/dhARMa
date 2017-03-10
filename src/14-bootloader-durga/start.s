@bare-metal startup asm code to setup stack and bss and branch to C code which will
@init the RAM and return, the asm code resumes and shifts the stack to RAM and jumps
@back to the C code 'main' function to resume execution utilising the stack on the RAM
@
@author: muteX023

.equ STACK_SIZE, 1024
.equ STACK_SUPERVISOR_START, 0x4030CDFC

.equ CM_PER_GPIO1_CLKCTRL, 0x44e000AC
.equ GPIO1_OE, 0x4804C134
.equ GPIO1_SETDATAOUT, 0x4804C194
.equ GPIO1_CLEARDATAOUT, 0x4804C190
.equ GPIO1_DATAOUT, 0x4804C13C

.equ CM_WKUP_REGS_BASE, 0x44E00400
.equ CM_WKUP_CM_IDLEST_DPLL_DDR_OFFSET, 0x34

.equ STACK_RAM_START, 0x9FFFFFBC
.equ EMIF_DDR3_RAM_START_ADDR, 0x80000000

_start:
    mrs r0, cpsr
    bic r0, r0, #0x1F	@ clear mode bits
    orr r0, r0, #0x13	@ set supervisor mode
    orr r0, r0, #0xC0	@ disable FIQ and IRQ
    msr cpsr, r0

    /* set clock for GPIO1, TRM 8.1.12.1.31 */
    ldr r0, =CM_PER_GPIO1_CLKCTRL
    mov r1, #0x02
    str r1, [r0]

    @check GPIO1_SETDATAOUT usr led pin 0 to see if we're in pwr on reset or have relocated already
    @by default on POR, all leds are off
    ldr r0, =GPIO1_SETDATAOUT
    ldr r1, [r0]
	tst r1, #(0x1 << 21)
	bne RAM_STACK

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

    @switch to abort mode and setup abort mode stack
    mrs r1, cpsr
    bic r1, r1, #0x1F	    
    orr r1, r1, #0x17
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
    
    @switch to fiq mode and setup fiq stack
    mrs r1, cpsr
    bic r1, r1, #0x1F	    
    orr r1, r1, #0x11
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
	bl hal_init_platform_stage1

	@jump to DDR3 RAM and begin execution of relocated code
	ldr r0, =EMIF_DDR3_RAM_START_ADDR
	mov pc, r0

RAM_STACK:
	@set usr2 led on
	ldr r4, =GPIO1_SETDATAOUT
	mov r5, #(1<<23)
	str r5, [r4]

	@now we need to move the stack to the DDR3 RAM end
	@at 0x9FFFFFBC below the interrupt table
	@the DDR3 RAM starts at 0x80000000 and is 512MB size.
	@by convention ARM stacks are descending in nature, so place
	@the stack at the end of the RAM.

	@Setup supervisor mode stack 
	ldr sp, =STACK_RAM_START
	mov r3, sp

	@switch to undefined exception mode and setup undefined exception stack
	mrs r1, cpsr
	bic r1, r1, #0x1F
	orr r1, r1, #0x1B
	msr cpsr, r1
	sub r3, r3, #STACK_SIZE
	mov sp, r3    

    @switch to abort mode and setup abort mode stack
    mrs r1, cpsr
    bic r1, r1, #0x1F	    
    orr r1, r1, #0x17
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

    @switch to fiq mode and setup fiq stack
    mrs r1, cpsr
    bic r1, r1, #0x1F	    
    orr r1, r1, #0x11
    msr cpsr, r1
    sub r3, r3, #STACK_SIZE
    mov sp, r3

	@back to supervisor mode
	mrs r1, cpsr
	bic r1, r1, #0x1F	
	orr r1, r1, #0x13
	msr cpsr, r1

	@gcc assumes a zeroed out BSS segment (Block started by symbol - stores uninitalized static vars)
	ldr r0, =__bss_start__
	ldr r1, =__bss_size__
	add r1, r0
	mov r2, #0
ZLOOP:
	cmp r0, r1
	strlt r2, [r0]
	add r0, #4
	blt ZLOOP
	
	@ now back to C-code to continue rest of the platform init
	bl hal_init_platform_stage2
	
	@finally all init is done, jmp to main
	b main
