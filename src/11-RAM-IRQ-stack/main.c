/* Bare metal C program to initialize the onboard 512MB MIcron DDR3 RAM
* on BBB and place the stack on RAM.
* 
* @author: muteX023
*/

#include "../common/bbb_hal.h"

#define INTR_TABLE_START 0x9FFFFF8A /* 32 + 44 bytes below the end of RAM */

void init()
{
	u32 *p32 = NULL:
	u32 *f32 = NULL:
	int i = 0;

	hal_init_led();
	hal_usr_led_on(3);
	
	hal_init_ddr3_ram();
	
	/* test the first 1MB DDR RAM onboard BBB
	* it will take long time to test all 512MB (around 20 min !)
	*/
	if(hal_ram_test(0xDEADCAFE, 1024 * 1024))
		hal_usr_led_on(2);
	
	/* set VBAR to point to DDR3 RAM end where we will
	 * relocate our interrupt table
	*/
	asm volatile (
		"mrc p15, #0, r0, c1, c0, #0 \n"
		"bic r0, #(1 << 13) \n"
		"mcr p15, #0, r0, c1, c0, #0 \n"
		"ldr r0, =INTR_TABLE_START \n"
		"mcr p15, #0, r0, c12, c0, #0 \n"
	);
	
	/* copy the interrupt table to the end of DDR3 RAM */
	
	/* copy the individual interrupt handlers below the intr table
	 * in the DDR RAM
	*/
	p32 = (u32 *)INTR_TABLE_START;
	f32 = (u32 *)intr_table;
	for (i = 0; i < 8; ++i)
		p32[i] = f32[i];
	
	/* return to asm startup code. */
	return;
}

void fiq_hdlr()
{
	
}

void irq_hdlr()
{
	
}

void databort_hdlr()
{
	
}

void progabort_hdlr()
{
	
}

void swi_hdlr()
{
	
}

void undef_hdlr()
{
	
}

void reset_hdlr()
{
	
}

/* note - the order of exceptions in the intr table is reversed
 so that when copied to the RAM they will be in correct order
 */
void intr_table()
{
	asm volatile (
		"ldr pc, fiq_hdlr \n"
		"ldr pc, irq_hdlr \n"
		"nop \n"
		"ldr pc, databort_hdlr \n"
		"ldr pc, progabort_hdlr \n"
		"ldr pc, swi_hdlr \n"
		"ldr pc, undef_hdlr \n"
		"ldr pc, reset_hdlr \n"
	);
}

void main()
{


	while(1);
}
