/*
Bare metal BeagleBone Black example for blinking led USR0
every second using the RTC 1-second interrupt
Some code from - https://github.com/auselen/down-to-the-bone/tree/master/baremetal_irq
has been referred/used here.
@author: muteX023
*/

/* The load address is hardcoded in the signGP utility to be the start of the internal SRAM*/
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
.equ INTC_MIR2_SET, 0x482000CC
.equ INTC_ILR75, 0X4820022C	@4h offset for each register 100h - 2fch 
.equ INTC_SIR_IRQ, 0X48200040
.equ INTC_IRQ_PRIORITY, 0x48200060
.equ INT_IRQ_HDLR_RAM, 0x4030CE38
.equ INT_VEC_BASE_RAM, 0x4030CE00
.equ INT_VEC_BASE_ROM, 0x20000
.equ INT_IRQ_DEFAULT_HDLR, 0x4030CE18

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
    
    
    @NOTE: Stack setup is required, only if you are going to use a stack in your irq processing
    @otherwise there is no need to setup stacks
    
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
    NOTE: Setting VBAR is required if you decide to relocate your vector table to an address
    other than the one provided by the initial boot code.
    
    @Set the interrupt vector base address via the sysctrl register -- CortexA series prog guide 3.1.4
    mrc p15, 0, r0, c1, c0, 0   @ Read CP15 SCTRL Register
    bic r0, #(1 << 13)          @ V = 0
    mcr p15, 0, r0, c1, c0, 0   @ Write CP15 SCTRL Register
    ldr r0, =INTVEC_BASE_RAM	
    mcr p15, 0, r0, c12, c0, 0  @ Set VBAR 
    */    
    
    @Just verify if the boot rom has set the VBAR correctly to 20000h and turn on usr led -- TRM 26.1.3.1
    mrc p15, 0, r0, c12, c0, 0
    ldr r1, =INT_VEC_BASE_ROM
    cmp r0, r1
    beq NEXT
    @set usr3 led on to indicate bad VBAR !
    mov r7, #3
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
    mov r1, #0x48
    @mov r1, #(1<<6)
    str r1, [r0]
    
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
    

@see TRM 6.2.2
IRQ_HDLR:

	@save regs and link
	stmfd sp!, {r0-r11, lr}

  @ignore spurious interrupts
  ldr r10, =INTC_SIR_IRQ
  ldr r9, [r10]
  ldr r8, =0xFFFFFF80
  ands r9, r9, r8
  bne INT_XIT
  
  @Get the number of the highest priority active IRQ -- TRM 6.5.1.4
  ldr r10, =INTC_SIR_IRQ
  ldr r10, [r10] 
  
  @Apply the mask to get the active IRQ number
  and r10, r10, #0x7F
  cmp r10, #75
  bne INT_XIT	@If some other interrupt, exit. (should never be the case, but still..)
  
	@set the interrupt mask bit for the RTC interrupt - i.e, the 11th bit in MIR2, bits0-63 are in MIR0-1 -- TRM 6.3 & 6.5.1.31
	@i.e, disable RTC interrupts till you process current interrupt
    ldr r0, =INTC_MIR2_SET
    ldr r1, [r0]
    orr r1, r1, #(0x01<<11)
    str r1, [r0]
  
	@toggle the usr0 LED
	bl PROC_LEDTOGGLE
  
 INT_XIT:
  @allow pending/new IRQ's to occur, i.e re-enable ARM interrupts
  ldr r0, =INTC_CTRL
  mov r1, #1
  str r1, [r0]

	@restore regs and link
	ldmfd sp!, {r0-r11, lr}

	@re-enable RTC interrupts - we should do this after the memory (stack) access on the off chance
	@that memory is slower resulting in another interrupt before current one is handled.
	@this scenario is observed if for example we change the stack location to the slower internal SRAM address
	@between 0x402F0400 - 0x402FFFFF. Also keep in mind that in bare metal mode the public ROM would have set
	@processor clock speed to only 500mhz instead of 1ghz, so the irq handler may take more than a second to exec.
    ldr r0, =INTC_MIR2_CLEAR
    ldr r1, [r0]
    orr r1, r1, #(0x01<<11)
    str r1, [r0]

  @return from interrupt -- see Cortex A8 TRM from ARM, section 2.15.1
  subs pc, lr, #4


@param in r7 = which usr led to turn on = 0/1/2/3
PROC_LEDON:
    add r7, r7, #21
    mov r6, #1
    mov r6, r6, lsl r7
    ldr r0, =GPIO1_SETDATAOUT
    str r6, [r0]
    mov pc, lr
    
@toggles usr0 led
PROC_LEDTOGGLE:
	ldr r0, =GPIO1_SETDATAOUT
	ldr r1, [r0]
	tst r1, #(1<<21)
	beq LEDOP
	ldr r0, =GPIO1_CLEARDATAOUT
LEDOP:
	mov r1, #(1<<21)
	str r1, [r0]
	mov pc, lr

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@currently unused    
NO_HDLR:
    b NO_HDLR

@currently unused     
INTVEC_TABLE:
    b     _start        /* reset - _start  		*/
    ldr   pc, NO_HDLR   /* undefined - _undf    */
    ldr   pc, IRQ_HDLR  /* SWI - _swi           */
    ldr   pc, NO_HDLR   /* program abort - _pabt*/
    ldr   pc, NO_HDLR   /* data abort - _dabt   */
    nop                 /* reserved             */
    ldr   pc, IRQ_HDLR  /* IRQ - read the VIC   */
    ldr   pc, NO_HDLR   /* FIQ - _fiq           */    
     
    
