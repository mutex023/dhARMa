/* 'main' function of durga
* currently does nothing but print a line to uart and toggle led3
* @author: muteX023
*/

#include "../common/bbb_hal.h"

void main()
{
	hal_uart_putstr("DURGA has booted \n");
	hal_uart_putstr("test hex val print on uart = ");
	hal_uart_put32(0x30E2FCD7);
	while(1) {
		//toggle usr3 led
		hal_usr_led_toggle(3);
		hal_delay(1);
	}
}
