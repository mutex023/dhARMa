/* Bare metal C program to relocate stack, interrupt handlers and interrupt table to DDR3 RAM
* on BBB
* 
* @author: muteX023
*/

#include "../common/bbb_hal.h"

#define INTR_TABLE_START 0x9FFFFFC0 /* 32 + 32 bytes below the end of RAM */

void intr_table();
void reset_hdlr();
void fiq_hdlr();
void irq_hdlr(void);
void undef_hdlr();
void swi_hdlr();
void databort_hdlr();
void progabort_hdlr();

u32 *copy_intr_hdlrs(u32 *start, u32 *end, u32 *dest)
{
	--start;
	while (start >= end) {
		*dest = *start;
		--dest;
		--start;
	}
	return dest;
}

void init()
{
	u32 *p32 = NULL, *f32 = NULL;
	int i = 0;

	hal_init_led();
	
	hal_init_ddr3_ram();
	
	/* test the first 1MB DDR RAM onboard BBB
	* it will take long time to test all 512MB (around 20 min !)
	*/
	if(hal_ram_test(0xDEADCAFE, 1024 * 1024))
		hal_usr_led_on(3);
	
	/* set VBAR to point to DDR3 RAM end where we will
	 * relocate our interrupt table
	*/
	asm volatile (
		"mrc p15, #0, r0, c1, c0, #0 \n"
		"bic r0, #(1 << 13) \n"
		"mcr p15, #0, r0, c1, c0, #0 \n"
		"ldr r0, =0x9FFFFFC0 \n"
		"mcr p15, #0, r0, c12, c0, #0 \n"
	);

	/* copy the interrupt table to the end of DDR3 RAM */
	p32 = (u32 *)(INTR_TABLE_START);
	f32 = (u32 *)intr_table;
	for (i = 0; i < 8; ++i)
		p32[i] = f32[i];

	/* copy the individual interrupt handlers below the intr table
	 * in the DDR RAM - we're copying instructions inside each
	 * function in reverse order descending from the interrupt table
	*/
	p32 = (u32 *)(INTR_TABLE_START - 4);
	/* 1. reset_hdlr */
	p32 = copy_intr_hdlrs((u32 *)intr_table, (u32 *)reset_hdlr, p32);
	/* 2. undef_hdlr */
	p32 = copy_intr_hdlrs((u32 *)reset_hdlr, (u32 *)undef_hdlr, p32);	
	/* 3. swi_hdlr */
	p32 = copy_intr_hdlrs((u32 *)undef_hdlr, (u32 *)swi_hdlr, p32);
	/* 4. progabort_hdlr */
	p32 = copy_intr_hdlrs((u32 *)swi_hdlr, (u32 *)progabort_hdlr, p32);
	/* 5. databort_hdlr */
	p32 = copy_intr_hdlrs((u32 *)progabort_hdlr, (u32 *)databort_hdlr, p32);
	/* 6. irq_hdlr */
	p32 = copy_intr_hdlrs((u32 *)databort_hdlr, (u32 *)irq_hdlr, p32);	
	/* 7. fiq_hdlr */
	p32 = copy_intr_hdlrs((u32 *)irq_hdlr, (u32 *)fiq_hdlr, p32);

	/* initialize INTC registers */
	/* set intr 75 (RTC) as highest priority and type as irq -- TRM 6.5.1.44 */
	hal_init_intr(RTC_INTR_NUM, IRQ, 0);
	
	/* initialize RTC and RTC interrupts */
	hal_init_rtc_intr(EVERY_SEC, RTC_INTR_PERIODIC_ENABLE);

	/* return to asm startup code. 
	 * the ARM interrupts will be enabled by asm
	 * after shifting the stack to RAM 
	*/
	return;
}

void fiq_hdlr(void)
{
	asm volatile (
		"nop \n"
	);
}
/* see ARM gcc function attributes:
 * https://gcc.gnu.org/onlinedocs/gcc/ARM-Function-Attributes.html#ARM-Function-Attributes
 * 'naked' attribute does not generate stack prologue/epilogue instructions
*/
__attribute__ ((naked)) void irq_hdlr(void)
{
	/* save regs and link */
	asm volatile("stmfd sp!, {r0-r12, lr} \n");
	
	u32 val = 0;
	u32 led = 0;

	/* ignore spurious interrupts -- TRM 6.5.1.4 */
	val = READREG32(INTC_SIR_IRQ);
	if (val & 0xFFFFFF80)
		goto intr_xit;
  
	/* get the interrupt number and check if it is from RTC -- TRM 6.5.1.4 */
	if ((val & 0x7F) != RTC_INTR_NUM)
		goto intr_xit;

	/* disable RTC interrupts till you process current interrupt
	 * for this, set the interrupt mask bit for the RTC interrupt
	 * - i.e, the 11th bit in MIR2, bits0-63 are in MIR0-1 -- TRM 6.3 & 6.5.1.31
	*/
	WRITEREG32(INTC_MIR2_SET, 0x01 << 11);

	/* process the interrupt - here we just toggle usr0 led
	 * Note - we cannot directly call hal_usr_led_toggle from here
	 * because the irq hdlr has been relocated to DDR RAM
	 * whereas the hal_usr_led_toggle function resides on SRAM
	 * so the branch instr which uses pc relative addressing will fail
	 */
	led = 0x1 << 21;
	val = READREG32(GPIO1_SETDATAOUT);
	if (val & led)
		WRITEREG32(GPIO1_CLEARDATAOUT, led);
	else
		WRITEREG32(GPIO1_SETDATAOUT, led);
	
  
intr_xit:
	/* allow pending/new IRQ's to occur */
	WRITEREG32(INTC_CTRL, 1);

	/* restore regs and link */
	asm volatile ("ldmfd sp!, {r0-r12, lr} \n");
	
	/* re-enable RTC interrupts */
	WRITEREG32(INTC_MIR2_CLEAR, 0x01 << 11);

	/* return from interrupt -- see Cortex A8 TRM from ARM, section 2.15.1 */
	asm volatile("subs pc, lr, #4 \n");
}

void databort_hdlr(void)
{
	asm volatile (
		"nop \n"
	);
}

void progabort_hdlr(void)
{
	asm volatile (
		"nop \n"
	);	
}

void swi_hdlr(void)
{
	asm volatile (
		"nop \n"
	);	
}

void undef_hdlr(void)
{
	asm volatile (
		"nop \n"
	);	
}

void reset_hdlr(void)
{
	asm volatile (
		"nop \n"
	);
}

/* see ARM gcc function attributes:
 * https://gcc.gnu.org/onlinedocs/gcc/ARM-Function-Attributes.html#ARM-Function-Attributes
 * 'naked' attribute does not generate stack prologue/epilogue instructions
*/
__attribute__ ((naked)) void intr_table(void)
{
	asm volatile (
		"b reset_hdlr \n"
		"b undef_hdlr \n"
		"b swi_hdlr \n"
		"b progabort_hdlr \n"
		"b databort_hdlr \n"
		"nop \n"
		"b irq_hdlr \n"
		"b fiq_hdlr \n"
	);  
}

void main()
{
	while(1) {
		hal_usr_led_toggle(1);
		hal_delay(1);
	}
}
