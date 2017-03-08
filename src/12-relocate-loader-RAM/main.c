/* Bare metal C program to init DDR3 RAM and relocate itself there
* to begin execution from SDRAM
* 
* @author: muteX023
*/

#include "../common/bbb_hal.h"

u32 hola;

void init()
{
	u32 *src = NULL, *dest = NULL;
	u32 *end = NULL;

	hal_init_led();

	hal_init_ddr3_ram();
	
	/* test the first 1MB DDR RAM onboard BBB
	* it will take long time to test all 512MB (around 20 min !)
	*/
	if (!hal_ram_test(0xDEADFACE, 1024 * 1024)) {
		hal_assert();
	}
	
	/* relocate first 100kb of L3 RAM to DDR3 */
	src = (u32 *)LOAD_ADDR;
	dest = (u32 *)EMIF_DDR3_RAM_START_ADDR;
	end = (u32 *)(LOAD_ADDR + (100 * 1024));
	while(src < end) {
		*dest = *src;
		++src;
		++dest;
	}

	hal_usr_led_on(0);
	
	/* back to asm which will exec jump to DDR3 */
	return;
}

void main()
{
	static u32 jumba;

	/* test BSS - using the uninitalized static and global vars 
	* the startup C should take care of initializing global and static
	* uninitialized variables to zero - which is actually done by
	* just zeroing out the bss segment in the asm code 
	*/
	jumba += 3;
	hola += 3;
	if (jumba + hola == 6)
		hal_usr_led_on(2);

	while(1) {
		hal_usr_led_toggle(3);
		hal_delay(1);
	}
}
