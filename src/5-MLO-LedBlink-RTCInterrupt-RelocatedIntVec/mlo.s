/*
Bare metal BeagleBone Black example for blinking led USR0
every second using the RTC 1-second interrupt using a custom relocated interrupt vector table
Some code from - https://github.com/auselen/down-to-the-bone/tree/master/baremetal_irq
has been referred/used here.
@author: muteX023
*/

.equ LOAD_ADDR, 0x402f0400

.equ STACK_SIZE, 256
.equ STACK_SUPERVISOR_START, 0x4030CDFC

.equ CM_PER_GPIO1_CLKCTRL, 0x44e000AC
.equ GPIO1_OE, 0x4804C134
.equ GPIO1_SETDATAOUT, 0x4804C194
.equ GPIO1_CLEARDATAOUT, 0x4804C190
.equ GPIO1_DATAOUT, 0x4804C13C

.equ INTC_BASE, 0x48200000 
.equ RTC_BASE, 0x44E3E000 
.equ CM_RTC_BASE, 0x44E00800 

.equ INTC_SYSCONFING, 0x48200010
.equ INTC_CTRL, 0X48200048
.equ INTC_IDLE, 0x48200050
.equ INTC_ISR_SET0, 0x48200090
.equ INTC_ISR_CLEAR0, 0x48200094
.equ INTC_MIR2, 0x482000C4
.equ INTC_MIR2_CLEAR, 0x482000C8
.equ INTC_MIR0_CLEAR, 0x48200088
.equ INTC_ILR75, 0X4820022C	@4h offset for each register 100h - 2fch 
.equ INTC_SIR_IRQ, 0X48200040
.equ INTC_IRQ_PRIORITY, 0x48200060

.equ INT_IRQ_HDLR_RAM, 0x4030CE38
.equ INT_VEC_BASE_RAM, 0x4030CE00
.equ INT_VEC_BASE_ROM, 0x20000
.equ INT_IRQ_DEFAULT_HDLR, 0x4030CE18
.equ INTVEC_TABLE_BASE, 0x4030FD00	

.equ RTC_CTRL_REG, 0X44E3E040
.equ RTC_STATUS_REG, 0X44E3E044
.equ RTC_INTERRUPTS_REG, 0X44E3E048
.equ RTC_OSC_REG, 0x44E3E054
.equ RTC_INT_MASK, 0XFFFFF7FF
.equ RTC_KICK0R, 0x44E3E06C
.equ RTC_KICK1R, 0x44E3E070
.equ RTC_WRENABLE_KEY1, 0x83E70B13
.equ RTC_WRENABLE_KEY2, 0x95A4F1E0

.equ CM_RTC_RTC_CLKCTRL, 0x44E00800
.equ CM_RTC_CLKSCTRL, 0x44E00804

.equ DMTIMER7_TCLR, 0x4804A038
.equ CM_PER_TIMER7_CLKCTRL, 0x44E0007C

_start:
    mrs r0, cpsr
    bic r0, r0, #0x1F	@ clear mode bits
    orr r0, r0, #0x13	@ set supervisor mode
    orr r0, r0, #0xC0	@ disable FIQ and IRQ
    msr cpsr, r0    
    
    /*
    NOTE: Stack setup is required, only if you are going to use a stack in your irq processing
    otherwise there is no need to setup stacks
    
    @Setup supervisor mode stack 
    ldr sp, =STACK_SUPERVISOR_START
    mov r0, sp
    
    @switch to undefined exception mode and setup undefined exception stack
    mrs r1, cpsr
	bic r0, r0, #0x1B	    
    orr r1, r1, #0x12
    msr cpsr, r1
    sub r0, r0, #STACK_SIZE
    mov sp, r0    

    @switch to irq mode and setup irq stack -- ARM sys dev guide -- 9.2.4
    mrs r1, cpsr
	bic r0, r0, #0x1F	    
    orr r1, r1, #0x12
    msr cpsr, r1
    sub r0, r0, #STACK_SIZE
    mov sp, r0   
	
	@back to supervisor mode
	mrs r1, cpsr
	bic r0, r0, #0x1F	
    orr r1, r1, #0x13
    msr cpsr, r1
    */
    
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
    
    @set free running interrupt ctrl clock -- TRM 6.5.1.2
    ldr r0, =INTC_SYSCONFING
    mov r1, #0
    str r1, [r0]
    
    @set free running interrupt ctrl clock -- TRM 6.5.1.8
    ldr r0, =INTC_IDLE
    mov r1, #0x01
    str r1, [r0]
    
    @set highest priority(0) for RTC interrupt(75) and set it as an IRQ -- TRM 6.5.1.44
    ldr r0, =INTC_ILR75
    mov r1, #0
    str r1, [r0]  
    
    /*
    Need to move the interrupt vector to a custom address at the end of OCMC RAM
    */
    @Calculate the absolute address of the vector table and move the instructions one by one
    ldr r0, =INTVEC_TABLE
    ldr r1, =LOAD_ADDR
    add r0, r1
    ldr r2, =INTVEC_TABLE_BASE
	
    @Calculate the absolute 32-bit address of the custom IRQ handler and NO_HDLR and store it in a memory word at the end of the intvec table
    ldr r5, =IRQ_HDLR
    add r5, r1
    str r5, [r0, #32]
    ldr r5, =NO_HDLR
    add r5, r1
    str r5, [r0, #36]
    
    
    @copy the intvec table, irq handler address and NO_HDLR address to the custom address    
    mov r3, #10
COPY_LOOP:
	ldr r4, [r0]
	str r4, [r2]
	add r2, #4
	add r0, #4
	subs r3, #1
	bne COPY_LOOP        
    
    @set usr2 led on to indicate copy finished successfully
    mov r7, #2
    bl PROC_LEDON

SKIP:
    /*
    NOTE: Setting VBAR is required if you decide to relocate your vector table to an address
    other than the one provided by the initial boot code.
    ex:- TI startware relocates it to 0x4030FC00
    */  
    @Set the interrupt vector base address via the sysctrl register -- CortexA series prog guide 3.1.4
    mrc p15, #0, r0, c1, c0, #0    @ Read CP15 SCTRL Register
    bic r0, #(1 << 13)     		 @ V = 0 -- set this bit to zero to relocate interrupt table
	mcr p15, #0, r0, c1, c0, #0    @ Write CP15 SCTRL Register
	ldr r0, =INTVEC_TABLE_BASE
    mcr p15, #0, r0, c12, c0, #0   @ Set VBAR -- note that the last 5 bits of the address MUST be zero -- sec B4.1.156 in ARMv7-A-R Arch. Ref. Manual
    
    @Just verify if the VBAR is set correctly
    mrc p15, #0, r3, c12, c0, #0
    cmp r0, r3
    bne NEXT
    
    @set usr1 led on
    mov r7, #1
    bl PROC_LEDON
    
NEXT:    
    @enable the RTC clock module -- TRM 8.1.12.6
    ldr r0, =CM_RTC_CLKSCTRL
    mov r1, #0x02
    str r1, [r0]    
    ldr r0, =CM_RTC_RTC_CLKCTRL
    mov r1, #0x02
    str r1, [r0]
    
    @disable RTC register write protection -- TRM 20.3.5.23
    ldr r0, =RTC_KICK0R
    ldr r1, =RTC_WRENABLE_KEY1
    str r1, [r0]   
    ldr r0, =RTC_KICK1R
    ldr r1, =RTC_WRENABLE_KEY2
    str r1, [r0]   
    
    @enable the RTC -- TRM 20.3.5.14
    ldr r0, =RTC_CTRL_REG
    mov r1, #0x01
    str r1, [r0] 
    
    @set RTC to use the more accurate external 32khz oscillator -- TRM 20.3.5.19
    ldr r0, =RTC_OSC_REG
    mov r1, #(1<<6)
    str r1, [r0]

       
    ldr r8, =GPIO1_DATAOUT
    mov r9, #(1<<21)
	
        
    @must wait for RTC BUSY period to end before enabling RTC timer interrupt -- TRM 20.3.5.15/16
WAIT_BUSY:
    ldr r0, =RTC_STATUS_REG
    ldr r1, [r0]
    tst r1, #0x01
    bne WAIT_BUSY 
    
    @enable the RTC periodic 1 sec interrupt -- TRM 20.3.5.16
    ldr r0, =RTC_INTERRUPTS_REG
    mov r1, #0x04
    str r1, [r0]  
    
    /*
    @install the custom interrupt handler at the specified internal RAM addr -- TRM 26.1.3.2
    @Note - even though the Am335x TRM itself says (6.2.2) that ARM blindly branches to 0x00000018 for IRQ
    @in reality for cortex processors, ARM will branch to VBAR+18h if VBAR is configured (which is done by the TI ROM boot code)
    @this is found in Cortex A series programming guide -- 3.1.4 and online in the ARM documentation
    @see - http:@infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0433b/CIHHDAIH.html
    ldr r0, =INT_IRQ_HDLR_RAM
    ldr r1, =IRQ_HDLR
    ldr r2, =LOAD_ADDR	@need to add the load address, because the assembler will only generate a relative address
    add r1, r2
    str r1, [r0]      
    */
        
    @finally enable interrupts on ARM side
    mrs r0, cpsr
    bic r0, r0, #0x80	@ disable FIQ, but enable IRQ
    msr cpsr, r0
    
    @clear the interrupt mask bit for the RTC interrupt - i.e, the 11th bit in MIR2, bits0-63 are in MIR0-1 -- TRM 6.3 & 6.5.1.29
    ldr r0, =INTC_MIR2_CLEAR
    mov r1, #(0x01<<11)
    str r1, [r0]

   
END:
    nop
    b END
    

/*
The IRQ handler -- see TRM 6.2.2 for more details on how to implement an IRQ/interrupt handler
*/
IRQ_HDLR:	
  @save return address
  mov r4, lr  
  
  @save ctx -- currently not used
  @stmfd sp!, {r0-r12, lr}
  @mrs r11, spsr
  
  @ignore spurious interrupts
  ldr r5, =INTC_SIR_IRQ
  ldr r6, [r5]
  ldr r7, =0xFFFFFF80
  ands r6, r6, r7
  bne INT_XIT
  
  @Get the number of the highest priority active IRQ -- TRM 6.5.1.4
  ldr r5, =INTC_SIR_IRQ
  ldr r5, [r5] 
  
  @Apply the mask to get the active IRQ number
  and r5, r5, #0x7F 
  cmp r5, #75
  bne INT_XIT	@If some other interrupt, exit. (should never be the case, but still..)
  
  @ideally we must disable RTC interrupts here before processing
  @but since the speed of the cortex processor is quite high
  @the handler would certainly have finished exec. before the next
  @1-sec interrupt, so we don't bother
  
  @toggle the usr0 LED
  ldr r0, =GPIO1_DATAOUT
  ldr r1, [r0]
  eor r1, r1, #(1<<21)
  str r1, [r0]
  
INT_XIT:
  @allow pending/new IRQ's to occur
  ldr r0, =INTC_CTRL
  mov r1, #1
  str r1, [r0]

  @data sync barrier -- dunno what this is, need to research
  mcr p15, #0, r0, c7, c10, #4

  @restore ctx
  @msr spsr, r11
  @ldmfd sp!, {r0-r12, lr}
  
  @restore return address
  mov lr, r4
  
  @return from interrupt
  subs pc, lr, #4


@param in r7 = which usr led to turn on = 0/1/2/3
PROC_LEDON:
    add r7, r7, #21
    mov r6, #1
    mov r6, r6, lsl r7
    ldr r0, =GPIO1_DATAOUT
    ldr r5, [r0]
    orr r5, r5, r6
    str r5, [r0] 
    mov pc, lr

/*
Default interrupt handler, just a dead loop
*/
NO_HDLR:
	b NO_HDLR

/*
The custom interrupt vector table.
Currently handles only IRQ, other interrupts result in a jump to NO_HDLR where
the processor will simply execute a dead loop.
The addresses of the IRQ handler and the default handler are stored in two 32-bit words at the end of this
table so that pc relative addressing can be used to execute the jump.
*/
INTVEC_TABLE:
		ldr pc, [pc, #28]    	/* reset - _start  		*/
        ldr pc, [pc, #24]       /* undefined - _undf    */
        ldr pc, [pc, #20]       /* SWI - _swi           */
        ldr pc, [pc, #16]     	/* program abort - _pabt*/
        ldr pc, [pc, #12]     	/* data abort - _dabt   */
        nop					    /* reserved             */
		ldr pc, [pc, #0]		/* IRQ - branch to IRQ_HDLR address stored at dummy1  -- PC always points to current instr. + 8 bytes due to historical reasons related to pipelining*/      		
        ldr pc, [pc, #0]        /* FIQ - _fiq           */    
     	nop						/* dummy1 - used to store absolute 32-bit addr. of IRQ handler */
     	nop						/* dummy2 - used to store absolute 32-bit addr. of NO_HDLR     */     	

