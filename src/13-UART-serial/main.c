/* Bare metal C to initialize UART0 on BBB
* and output a string to the serial console
* UART init code is based on - https://github.com/auselen/down-to-the-bone/tree/master/baremetal_runtime
* @author: muteX023
*/

#include "../common/bbb_hal.h"

void uart_putchar(u8 val)
{
	/* Wait for Tx hold reg to be empty - TRM 19.5.1.12 */
	while (!(READREG8(UART0_REGS_BASE + 0x14) & 0x20))
		;
	WRITEREG8(UART0_REGS_BASE, val);
}

void main()
{
	u32 val = 0;

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
	
	/* turn on usr0 led */
	WRITEREG32(GPIO1_SETDATAOUT, 0x01<<21);
	
	/* enable receiver and pullup in uart0 rxd conf 
	 * disable receiver and pullup in uart0 txd conf - TRM 9.3.51 */
	WRITEREG32(CTRL_MODULE_REG_BASE + CTRL_MODULE_CONF_UART0_RXD_OFFSET, 0x30);
	WRITEREG32(CTRL_MODULE_REG_BASE + CTRL_MODULE_CONF_UART0_TXD_OFFSET, 0x0);
	
	/* start a sw forced wake up on the pwr domain - TRM 8.1.12.2.1 & 8.1.12.1.53 */
	val = READREG32(CM_WKUP_REGS_BASE + CM_WKUP_CLKSTCTRL_OFFSET);
	val &= ~0x3;
	val |= 0x2;
	WRITEREG32(CM_WKUP_REGS_BASE + CM_WKUP_CLKSTCTRL_OFFSET, val);

	val = READREG32(CM_PER_REGS_BASE + CM_PER_L4HS_CLKSTCTRL_OFFSET);
	val &= ~0x3;
	val |= 0x2;
	WRITEREG32(CM_PER_REGS_BASE + CM_PER_L4HS_CLKSTCTRL_OFFSET, val);


	/* enable UART0 - uart module-1 clk - TRM 8.1.12.2.46 & 8.1.12.1.23 */
	val = READREG32(CM_WKUP_REGS_BASE + CM_WKUP_UART0_CLKCTRL_OFFSET);
	val &= ~0x3;
	val |= 0x2;
	WRITEREG32(CM_WKUP_REGS_BASE + CM_WKUP_UART0_CLKCTRL_OFFSET, val);

	val = READREG32(CM_PER_REGS_BASE + CM_PER_UART1_CLKCTRL_OFFSET);
	val &= ~0x3;
	val |= 0x2;
	WRITEREG32(CM_PER_REGS_BASE + CM_PER_UART1_CLKCTRL_OFFSET, val);

	/* see TRM 19.4.1 for programming UART */
	
	/* perform a soft reset of uart module - TRM 19.5.1.31 */
	val = READREG32(UART0_REGS_BASE + UART0_SYSC_OFFSET);
	val |= 0x2;
	WRITEREG32(UART0_REGS_BASE + UART0_SYSC_OFFSET, val);

	/* wait for reset to complete */
	while ( !(READREG32(UART0_REGS_BASE + UART0_SYSS_OFFSET) & 0x1) )
		;

	/* disable pwr mgmt idle request ack in UART0 - TRM 19.5.1.31 */
	val = READREG32(UART0_REGS_BASE + UART0_SYSC_OFFSET);
	val |= 0x1 << 3;
	WRITEREG32(UART0_REGS_BASE + UART0_SYSC_OFFSET, val);

	/* turn on usr1 led */
	WRITEREG32(GPIO1_SETDATAOUT, 0x01<<22);

	/* wait for TX FIFO and shift regs to be empty - TRM 19.5.1.12 */
	while (!(READREG8(UART0_REGS_BASE + UART0_LSR_OFFSET) & 0x40))
		;
	
	/* turn on usr2 led */
	WRITEREG32(GPIO1_SETDATAOUT, 0x01<<23);

	/* disable all UART interrupts and sleep mode - TRM 19.5.1.3 */
	WRITEREG8(UART0_REGS_BASE + 0x4, 0);
	
	/* disable UART - TRM 19.5.1.19 */
	WRITEREG8(UART0_REGS_BASE + 0x20, 7);
	
	/* configure - 
	 * Baud 115,200
	 * Bits 8
	 * Parity N
	 * Stop Bits 1
	 * FLow ctrl None
	*/
    WRITEREG8(UART0_REGS_BASE + 0xC, ~0x7c);
    WRITEREG8(UART0_REGS_BASE + 0x0, 0);
    WRITEREG8(UART0_REGS_BASE + 0x4, 0);
    WRITEREG8(UART0_REGS_BASE + 0xC, 3);
    WRITEREG8(UART0_REGS_BASE + 0x10, 3);
    WRITEREG8(UART0_REGS_BASE + 0x8, 7);
    WRITEREG8(UART0_REGS_BASE + 0xC, ~0x7c);
    WRITEREG8(UART0_REGS_BASE + 0x0, 26);
    WRITEREG8(UART0_REGS_BASE + 0x4, 0);
    WRITEREG8(UART0_REGS_BASE + 0xC, 3);
    WRITEREG8(UART0_REGS_BASE + 0x20, 0);

	uart_putchar('d');
	uart_putchar('h');
	uart_putchar('A');
	uart_putchar('R');
	uart_putchar('M');
	uart_putchar('a');
	uart_putchar(' ');
	uart_putchar('!');
	uart_putchar('\r');
	uart_putchar('\n');
	/* turn on usr3 led */
	WRITEREG32(GPIO1_SETDATAOUT, 0x01<<24);

	while(1);
}
