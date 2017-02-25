/* Bare metal C program to initialize the onboard 512MB MIcron DDR3 RAM
* on BBB and place the stack on RAM.
* 
* @author: muteX023
*/

#include "../common/bbb_hal.h"

u32 hola;

void init()
{
	hal_init_led();
	hal_usr_led_on(0);
	
	hal_init_ddr3_ram();
	
	/* test the first 1MB DDR RAM onboard BBB
	* it will take long time to test all 512MB (around 20 min !)
	*/
	if(hal_ram_test(0xDEADCAFE, 1024 * 1024))
		hal_usr_led_on(1);
	
	/* return to asm startup code. */
	return;
}

void main()
{
	static u32 jumba;
	hal_usr_led_on(3);
	hal_usr_led_off(0);

	/* test BSS - using the uninitalized static and global vars 
	* the startup C should take care of initializing global and static
	* uninitialized variables to zero - which is actually done by
	* just zeroing out the bss segment in the asm code 
	*/
	jumba += 3;
	hola += 3;
	if (jumba + hola == 6)
		hal_usr_led_off(1);

	while(1);
}
