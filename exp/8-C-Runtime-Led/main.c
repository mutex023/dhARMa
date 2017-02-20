/* Simple C program which will turn on usr3 LED on BBB
* Assumed to be run as an MLO bootloader from SDcard
* in a bare metal environment
* @author: muteX023
*/

#include "../../src/common/bbb_hal.h"

unsigned char usr_led_on(unsigned char led_num)
{
	unsigned int val = 0;
	
	val = 0x1 << (led_num + 21);
	WRITEREG32(GPIO1_SETDATAOUT, val);
	
	return 1;
}

void main()
{
	unsigned int val = 0;

	/* set clock for GPIO1, TRM 8.1.12.1.31 */
	WRITEREG32(CM_PER_GPIO1_CLKCTRL, 0x02);

    /* set pin 21,22,23,24 for output, led USR0,1,2,3, TRM 25.3.4.3 */
	val = READREG32(GPIO1_OE);
	/* clear the bits 21-24 to enable GPIO1 for output
	* don't touch other bits, as they're used for sd card IO !!
	*/
	val &= ~(0xF << 21);
	WRITEREG32(GPIO1_OE, val);

	/* clear out gpio1-21,22,23,24 pins using data out register first*/
	WRITEREG32(GPIO1_CLEARDATAOUT, 0x0F<<21);
	
	/* turn on usr3 led */
	WRITEREG32(GPIO1_SETDATAOUT, 0x01<<24);
	
	/* turn on usr0 led via a function call - testing the stack & return val */
	val = usr_led_on(0);
	
	usr_led_on(val);
	
	while(1);
}
