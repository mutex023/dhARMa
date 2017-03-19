/* 'main' function of durga
* currently durga only prints the system time and ascii chart
* from the interrupt handler to the uart and toggle led3
* @author: muteX023
*/

#include "../common/bbb_hal.h"
#include "../common/bbb_hal_mmc.h"

void rtc_intr_hdlr (void *data);

void main()
{
	int cnt = 4;

	hal_uart_putstr("DURGA has booted \n");
	if (hal_mmc_init())
		hal_uart_putstr("error initializing mmc !\n");
	if (hal_mmc_sdcard_init())
		hal_uart_putstr("error initializing sd card !\n");
	while(1) {
		//toggle usr3 led
		hal_usr_led_toggle(3);
		hal_delay(1);
		if (cnt > 1)
			--cnt;
		else if (cnt == 1) {
			--cnt;
			hal_register_irq_handler(RTC_INTR_NUM, rtc_intr_hdlr, NULL);
		}
	}
}

void rtc_intr_hdlr (void *data)
{
	static u8 c = 33;
	static u8 hr = 0, min = 0, sec = 0;

	/* disable RTC interrupts till you process current interrupt
	 * for this, set the interrupt mask bit for the RTC interrupt
	 * - i.e, the 11th bit in MIR2, bits0-63 are in MIR0-1 -- TRM 6.3 & 6.5.1.31
	*/
	WRITEREG32(INTC_MIR2_SET, 0x01 << 11);

	/* process the interrupt - print to uart an ascii chart 
	 * and system up time 
	*/
	++sec;
	if (sec >= 60) {
		sec = 0;
		++min;
		if (min >= 60) {
			min = 0;
			++hr;
			if (hr >= 24) {
				hr = 0;
			}
		}
	} 
	hal_uart_putstr("RTC - ");
	hal_uart_putchar(hr/10 + '0');
	hal_uart_putchar(hr%10 + '0');
	hal_uart_putchar(':');
	hal_uart_putchar(min/10 + '0');
	hal_uart_putchar(min%10 + '0');
	hal_uart_putchar(':');
	hal_uart_putchar(sec/10 + '0');
	hal_uart_putchar(sec%10 + '0');
	hal_uart_putstr(" - ");
	hal_uart_putchar(c++);
	hal_uart_putchar('\n');
	if (c > 126)
		c = 33;

	/* re-enable RTC interrupts */
	WRITEREG32(INTC_MIR2_CLEAR, 0x01 << 11);
}
