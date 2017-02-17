/* Simple C program which will turn on usr3 LED on BBB
* Assumed to be run as an MLO bootloader from SDcard
* in a bare metal environment
*/

#include "../common/bbb_hal.h"

void main()
{
	unsigned int val = 0;

	/* set clock for GPIO1, TRM 8.1.12.1.31 */
	WRITEREG32(CM_PER_GPIO1_CLKCTRL, 0x02);

    /* set pin 21,22,23,24 for output, led USR0,1,2,3, TRM 25.3.4.3 */
	val = READREG32(GPIO1_OE);
	val &= ~(0xF << 21); //clear the bits 21-24 to enable GPIO1 for output
	WRITEREG32(GPIO1_OE, val);

	/* clear out gpio1-21,22,23,24 pins using data out register first*/
    WRITEREG32(GPIO1_CLEARDATAOUT, 0x0F<<21);
	
	/* turn on usr3 led */
	WRITEREG32(GPIO1_SETDATAOUT, 0x01<<24);
	
	while(1);
}
