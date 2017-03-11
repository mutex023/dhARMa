/* 'main' function of durga
* currently does nothing but print a line to uart and toggle led3
* @author: muteX023
*/

#include "../common/bbb_hal.h"

__attribute__ ((naked))
void dummy()
{
	asm volatile (
		"add r1, r2, r3 \n"
		"sub r5, r6, r7 \n"
		"mov r2, r3 \n" );
}

void main()
{
	u32 *p = (u32 *)dummy;
	
	hal_uart_putstr("DURGA has booted \n");
	while(1) {
		//toggle usr3 led
		hal_usr_led_toggle(3);
		hal_delay(1);
		*p = 0xFFFFFFFF;
		dummy();
	}
}
