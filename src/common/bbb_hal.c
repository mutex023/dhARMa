/* BeagleBoneBlack HW Abstraction layer
* performs various BBB hw and register ops and initialization
* @author: muteX023
*/

#include "bbb_hal.h"
#include "utils.h"

#define INTR_TABLE_START 0x9FFFFFC0 /* 32 + 32 bytes below the end of RAM */

void hal_intr_table();
void hal_default_reset_hdlr();
void hal_default_fiq_hdlr();
void hal_default_irq_hdlr(void);
void hal_default_undef_hdlr();
void hal_default_swi_hdlr();
void hal_default_databort_hdlr();
void hal_default_prefetch_abort_hdlr();

irq_hdlr_t g_irq_table[INT_MAX_IRQS];

void hal_init_led()
{
	u32 val = 0;

	/* set clock for GPIO1, TRM 8.1.12.1.31 */
	WRITEREG32(CM_PER_GPIO1_CLKCTRL, 0x02);

    /* set pin 21,22,23,24 for output, led USR0,1,2,3, TRM 25.3.4.3 */
	val = READREG32(GPIO1_OE);
	/* clear the bits 21-24 to enable GPIO1 for output
	* don't touch other bits, as they're used for sd card IO !!
	*/
	val &= ~(0xF << GPIO1_USRLED_SHIFT);
	WRITEREG32(GPIO1_OE, val);

	/* clear out gpio1-21,22,23,24 pins using data out register first*/
	WRITEREG32(GPIO1_CLEARDATAOUT, 0x0F<<GPIO1_USRLED_SHIFT);
}

void hal_usr_led_toggle(u8 led_num)
{
	u32 val = 0;
	u32 led = 0;

	led = 0x1 << (led_num + GPIO1_USRLED_SHIFT);
	val = READREG32(GPIO1_SETDATAOUT);
	if (val & led)
		WRITEREG32(GPIO1_CLEARDATAOUT, led);
	else
		WRITEREG32(GPIO1_SETDATAOUT, led);
}

void hal_usr_led_on(u8 led_num)
{
	u32 val = 0;
	
	val = 0x1 << (led_num + GPIO1_USRLED_SHIFT);
	WRITEREG32(GPIO1_SETDATAOUT, val);
}

void hal_usr_led_off(u8 led_num)
{
	u32 val = 0;
	
	val = 0x1 << (led_num + GPIO1_USRLED_SHIFT);
	WRITEREG32(GPIO1_CLEARDATAOUT, val);
}

/* 'prints' a 4-bit value using usr gpio leds */
void hal_usr_led_print4(u8 val)
{
	WRITEREG32(GPIO1_CLEARDATAOUT, (0xF << GPIO1_USRLED_SHIFT));
	WRITEREG32(GPIO1_SETDATAOUT, (val & 0xF) << GPIO1_USRLED_SHIFT);
}

/* 'prints' a 32-bit value using usr gpio leds
* a 2 second gap (indicated by a usr0 led toggle) is given between 'printing' each 4 bit nibble */
void hal_usr_led_print32(u32 val)
{
	u32 mask = 0xF;
	u32 led = 0;
	u8 cnt = 0;
	
	WRITEREG32(GPIO1_CLEARDATAOUT, (0xF << GPIO1_USRLED_SHIFT));
	while (mask) {
		led = val & mask;
		led = led >> cnt;
		WRITEREG32(GPIO1_SETDATAOUT, led << GPIO1_USRLED_SHIFT);
		mask = mask << 4;
		cnt += 4;
		hal_delay(2);
		WRITEREG32(GPIO1_CLEARDATAOUT, (0xF << GPIO1_USRLED_SHIFT));
		hal_delay(1);
		WRITEREG32(GPIO1_SETDATAOUT, 0x1 << GPIO1_USRLED_SHIFT);
		hal_delay(1);
		WRITEREG32(GPIO1_CLEARDATAOUT, (0xF << GPIO1_USRLED_SHIFT));
		hal_delay(1);
	}
} 

void hal_init_ddr_pll()
{
	u32 val = 0;
	
	/* put the DPLL in MN bypass mode - TRM 8.1.12.2.38 */
	val = READREG32(CM_WKUP_REGS_BASE + CM_WKUP_CM_CLKMODE_DPLL_DDR_OFFSET);
	val &= ~0x7;
	val |= 0x4;
	WRITEREG32(CM_WKUP_REGS_BASE + CM_WKUP_CM_CLKMODE_DPLL_DDR_OFFSET, val);
	/* wait till DPLL switches to bypass mode - TRM 8.1.12.2.14	*/
	while ( !(READREG32(CM_WKUP_REGS_BASE + CM_WKUP_CM_IDLEST_DPLL_DDR_OFFSET)
		& 0x100) );
	
	/* clear and set the MULT and DIV factors for the DPLL - TRM 8.1.12.2.17 */
	val = READREG32(CM_WKUP_REGS_BASE + CM_WKUP_CM_CLKSEL_DPLL_DDR_OFFSET);
	val &= ~0x7FF7F;
	WRITEREG32(CM_WKUP_REGS_BASE + CM_WKUP_CM_CLKSEL_DPLL_DDR_OFFSET, val);
	val = READREG32(CM_WKUP_REGS_BASE + CM_WKUP_CM_CLKSEL_DPLL_DDR_OFFSET);
	val |= (DDR3_FREQ << 8) | DDRPLL_N;
	WRITEREG32(CM_WKUP_REGS_BASE + CM_WKUP_CM_CLKSEL_DPLL_DDR_OFFSET, val);
	
	/* set M2 clkout post divider factor for the DPLL - TRM 8.1.12.2.41 */
	val = READREG32(CM_WKUP_REGS_BASE + CM_WKUP_CM_DIV_M2_DPLL_DDR_OFFSET);
	val &= ~0x1F;
	val |= DDRPLL_M2;
	WRITEREG32(CM_WKUP_REGS_BASE + CM_WKUP_CM_DIV_M2_DPLL_DDR_OFFSET, val);
	
	/* lock the DPLL - TRM 8.1.12.2.38 */
	val = READREG32(CM_WKUP_REGS_BASE + CM_WKUP_CM_CLKMODE_DPLL_DDR_OFFSET);
	val &= ~0x7;
	val |= 0x7;
	WRITEREG32(CM_WKUP_REGS_BASE + CM_WKUP_CM_CLKMODE_DPLL_DDR_OFFSET, val);
	/* wait for DPLL to be locked - TRM 8.1.12.2.14 */
	while ( !(READREG32(CM_WKUP_REGS_BASE + CM_WKUP_CM_IDLEST_DPLL_DDR_OFFSET)
		& 0x1) );
}

void hal_init_emif()
{
	u32 val = 0;

	/* enable EMIF Firewall Clock - TRM 8.1.12.1.38 */
	val = READREG32(CM_PER_REGS_BASE + CM_PER_EMIF_FW_CLKCTRL_OFFSET);
	val &= ~0x3;
	val |= 0x2;
	WRITEREG32(CM_PER_REGS_BASE + CM_PER_EMIF_FW_CLKCTRL_OFFSET, val);

	/* enable EMIF clock - TRM 8.1.12.1.9 */
	val = READREG32(CM_PER_REGS_BASE + CM_PER_EMIF_CLKCTRL_OFFSET);
	val &= ~0x3;
	val |= 0x2;
	WRITEREG32(CM_PER_REGS_BASE + CM_PER_EMIF_CLKCTRL_OFFSET, val);
	
	/* wait for the clocks EMIF_GCLK & L3_GCLK to go active - TRM 8.1.12.1.4 */
	while ((READREG32(CM_PER_REGS_BASE + CM_PER_L3_CLKSTCTRL_OFFSET) & 0x14) != 0x14)
		;
}

void hal_init_ddr_phy()
{
	u32 val = 0;

	/* enable VTP (Voltage, Temp, Process) control, TRM - 9.3.54 */
	val = READREG32(CTRL_MODULE_REG_BASE + CTRL_MODULE_VTP_CTRL_OFFSET);
	val |= 0x40;
	WRITEREG32(CTRL_MODULE_REG_BASE + CTRL_MODULE_VTP_CTRL_OFFSET, val);
	
	/* clear flops, TRM - 9.3.54 */
	val = READREG32(CTRL_MODULE_REG_BASE + CTRL_MODULE_VTP_CTRL_OFFSET);
	val &= ~0x1;
	WRITEREG32(CTRL_MODULE_REG_BASE + CTRL_MODULE_VTP_CTRL_OFFSET, val);
	val = READREG32(CTRL_MODULE_REG_BASE + CTRL_MODULE_VTP_CTRL_OFFSET);
	val |= 0x1;
	WRITEREG32(CTRL_MODULE_REG_BASE + CTRL_MODULE_VTP_CTRL_OFFSET, val);
	
	/* wait for VTP ready - training sequence complete */
	while(!(val = READREG32(CTRL_MODULE_REG_BASE + CTRL_MODULE_VTP_CTRL_OFFSET)
		 & 0x20))
		; //wait
	
	/* set DDR phy cmd slave ratio0 to 0x80 - no idea what this means ! - TRM 7.3.6.1 */
	WRITEREG32(DDR_PHY_REG_BASE + CMD0_REG_PHY_CTRL_SLAVE_RATIO_0_OFFSET,
		 DDR3_CMD0_SLAVE_RATIO_0_VAL);
	/* pass the core clock directly to RAM without inverting - TRM 7.3.6.3*/
	WRITEREG32(DDR_PHY_REG_BASE + CMD0_REG_PHY_INVERT_CLKOUT_0_OFFSET,
		DDR3_CMD0_REG_PHY_INVERT_CLKOUT_0_VAL);

	WRITEREG32(DDR_PHY_REG_BASE + CMD1_REG_PHY_CTRL_SLAVE_RATIO_0_OFFSET,
		 DDR3_CMD1_SLAVE_RATIO_0_VAL);
	WRITEREG32(DDR_PHY_REG_BASE + CMD1_REG_PHY_INVERT_CLKOUT_0_OFFSET,
		DDR3_CMD1_REG_PHY_INVERT_CLKOUT_0_VAL);

	WRITEREG32(DDR_PHY_REG_BASE + CMD2_REG_PHY_CTRL_SLAVE_RATIO_0_OFFSET,
		 DDR3_CMD2_SLAVE_RATIO_0_VAL);
	WRITEREG32(DDR_PHY_REG_BASE + CMD2_REG_PHY_INVERT_CLKOUT_0_OFFSET,
		DDR3_CMD2_REG_PHY_INVERT_CLKOUT_0_VAL);

	/* set DQS slave ratios - TRM 7.3.6.4 */
	WRITEREG32(DDR_PHY_REG_BASE + DATA0_REG_PHY_RD_DQS_SLAVE_RATIO_0_OFFSET,
		DDR3_DATA0_REG_PHY_RD_DQS_SLAVE_RATIO_0_VAL);
	WRITEREG32(DDR_PHY_REG_BASE + DATA0_REG_PHY_WR_DQS_SLAVE_RATIO_0_OFFSET,
		DDR3_DATA0_REG_PHY_WR_DQS_SLAVE_RATIO_0_VAL);
	WRITEREG32(DDR_PHY_REG_BASE + DATA0_REG_PHY_FIFO_WE_SLAVE_RATIO_0_OFFSET,
		DDR3_DATA0_REG_PHY_FIFO_WE_SLAVE_RATIO_0_VAL);
	WRITEREG32(DDR_PHY_REG_BASE + DATA0_REG_PHY_WR_DATA_SLAVE_RATIO_0_OFFSET,
		DDR3_DATA0_REG_PHY_WR_DATA_SLAVE_RATIO_0_VAL);
	
	WRITEREG32(DDR_PHY_REG_BASE + DATA1_REG_PHY_RD_DQS_SLAVE_RATIO_0_OFFSET,
		DDR3_DATA1_REG_PHY_RD_DQS_SLAVE_RATIO_0_VAL);
	WRITEREG32(DDR_PHY_REG_BASE + DATA1_REG_PHY_WR_DQS_SLAVE_RATIO_0_OFFSET,
		DDR3_DATA1_REG_PHY_WR_DQS_SLAVE_RATIO_0_VAL);
	WRITEREG32(DDR_PHY_REG_BASE + DATA1_REG_PHY_FIFO_WE_SLAVE_RATIO_0_OFFSET,
		DDR3_DATA1_REG_PHY_FIFO_WE_SLAVE_RATIO_0_VAL);
	WRITEREG32(DDR_PHY_REG_BASE + DATA1_REG_PHY_WR_DATA_SLAVE_RATIO_0_OFFSET,
		DDR3_DATA1_REG_PHY_WR_DATA_SLAVE_RATIO_0_VAL);

}

void hal_init_ddr3_ram()
{
	u32 val = 0;
	
	/* initialize the DDR3 PLL */
	hal_init_ddr_pll();
	
	/* enable the CM_WKUP clock control module - TRM 8.1.12.2.2 */
	WRITEREG32(CM_WKUP_REGS_BASE + CM_WKUP_CONTROL_CLKCTRL_OFFSET, 0x2);
	
	/* initialize the External mem interface EMIF */
	hal_init_emif();
	
	/* initialize ddr VTP and phy regs */
	hal_init_ddr_phy();

	/* set IO for DDR3 - TRM 9.3.53 */
	val = READREG32(CTRL_MODULE_REG_BASE + CTRL_MODULE_DDR_IO_CTRL_OFFSET);
	val &= DDR3_CTRL_MODULE_DDR_IO_CTRL_VAL;
	WRITEREG32(CTRL_MODULE_REG_BASE + CTRL_MODULE_DDR_IO_CTRL_OFFSET, val);

	/* set clock enable to be controlled by EMIF/DDR - TRM 9.3.77 */
	val = READREG32(CTRL_MODULE_REG_BASE + CTRL_MODULE_DDR_CKE_CTRL_OFFSET);
	val |= 0x1;
	WRITEREG32(CTRL_MODULE_REG_BASE + CTRL_MODULE_DDR_CKE_CTRL_OFFSET, val);

	/* set read latency and dynamic pwr down in EMIF - TRM 7.3.5.33/34 */
	WRITEREG32(EMIF_REG_BASE + EMIF_DDR_PHY_CTRL_1_OFFSET, DDR3_DDR_PHY_CTRL_1_VAL);
	WRITEREG32(EMIF_REG_BASE + EMIF_DDR_PHY_CTRL_1_SHDW_OFFSET, DDR3_DDR_PHY_CTRL_1_VAL);

	/* configure SDRAM clk cycle values - TRM 7.3.5.7-12 */
	WRITEREG32(EMIF_REG_BASE + EMIF_SDRAM_TIM_1_OFFSET, DDR3_EMIF_SDRAM_TIM_1_VAL);
	WRITEREG32(EMIF_REG_BASE + EMIF_SDRAM_TIM_1_SHDW_OFFSET, DDR3_EMIF_SDRAM_TIM_1_VAL);
	WRITEREG32(EMIF_REG_BASE + EMIF_SDRAM_TIM_2_OFFSET, DDR3_EMIF_SDRAM_TIM_2_VAL);
	WRITEREG32(EMIF_REG_BASE + EMIF_SDRAM_TIM_2_SHDW_OFFSET, DDR3_EMIF_SDRAM_TIM_2_VAL);
	WRITEREG32(EMIF_REG_BASE + EMIF_SDRAM_TIM_3_OFFSET, DDR3_EMIF_SDRAM_TIM_3_VAL);
	WRITEREG32(EMIF_REG_BASE + EMIF_SDRAM_TIM_3_SHDW_OFFSET, DDR3_EMIF_SDRAM_TIM_3_VAL);
	
	/* set SDRAM refresh rate - TRM 7.3.5.5/6 */
	WRITEREG32(EMIF_REG_BASE + EMIF_SDRAM_REF_CTRL_OFFSET, DDR3_EMIF_SDRAM_REF_CTRL_VAL);
	WRITEREG32(EMIF_REG_BASE + EMIF_SDRAM_REF_CTRL_SHDW_OFFSET, DDR3_EMIF_SDRAM_REF_CTRL_VAL);
	
	/* perform ZQ (zero quantum ?) configuration - TRM 7.3.5.29 */
	WRITEREG32(EMIF_REG_BASE + EMIF_ZQ_CONFIG_OFFSET, DDR3_EMIF_ZQ_CONFIG_VAL);
	
	/* configure SDRAM - TRM 7.3.5.3
	* termination = 1 (RZQ/4)
	* dynamic ODT = 2 (RZQ/2)
	* SDRAM drive = 0 (RZQ/6)
	* CWL = 0 (CAS write latency = 5)
	* CL = 2 (CAS latency = 5)
	* ROWSIZE = 5 (14 row bits)
	* PAGESIZE = 2 (10 column bits)
	*/
	WRITEREG32(EMIF_REG_BASE + EMIF_SDRAM_CONFIG_OFFSET, DDR3_EMIF_SDRAM_CONFIG_VAL);
	
	/* export the sdram config to EMIF - TRM 9.3.5 */
	WRITEREG32(CTRL_MODULE_REG_BASE + CTRL_MODULE_EMIF_SDRAM_CONFIG, DDR3_EMIF_SDRAM_CONFIG_VAL);
}

u8 hal_ram_test(u32 val, u64 size)
{
	u64 i = 0;
	u32 *p = (u32 *)EMIF_DDR3_RAM_START_ADDR;
	
	/* write the value to the RAM and read back */
	for (i = 0; i < size; ++i)
	{
		*p = val;
		++p;
	}
	p = (u32 *)EMIF_DDR3_RAM_START_ADDR;
	for (i = 0; i < size; ++i)
	{
		if (*p != val)
			return 0;
		++p;
	}

	return 1;
}

void hal_init_intr(u32 intr_num, intr_type_t intr_type, u8 priority)
{
	u32 val = 0;

    /* set free running interrupt ctrl clock -- TRM 6.5.1.2/8 */
	val = READREG32(INTC_SYSCONFING);
	if (val != 0)
		WRITEREG32(INTC_SYSCONFING, 0);

	val = READREG32(INTC_IDLE);
	if (val != 0x1)
		WRITEREG32(INTC_IDLE, 0x01);
	
	/* set priority and type(fiq/irq) for the interrupt -- TRM 6.5.1.44 */
    WRITEREG32(INTC_ILR_BASE + (intr_num * 4), ((priority << 2) | intr_type));
}

void hal_init_rtc_intr(rtc_intr_period_t period, rtc_intr_periodicity_t periodicity)
{
	/* enable the RTC clock module -- TRM 8.1.12.6 */
    WRITEREG32(CM_RTC_CLKSCTRL, 0x02);
    WRITEREG32(CM_RTC_RTC_CLKCTRL, 0x02);

	/* disable RTC register write protection -- TRM 20.3.5.23 */
    WRITEREG32(RTC_KICK0R, RTC_WRENABLE_KEY1);
    WRITEREG32(RTC_KICK1R, RTC_WRENABLE_KEY2);
    
	/* enable the RTC -- TRM 20.3.5.14 */
    WRITEREG32(RTC_CTRL_REG, 0x01);
	
    /* set RTC to use the more accurate external 32khz oscillator -- TRM 20.3.5.19 */
    WRITEREG32(RTC_OSC_REG, 1 << 6);
	
	/* must wait for RTC BUSY period to end before enabling RTC timer interrupt -- TRM 20.3.5.15/16 */
	while (!(READREG32(RTC_STATUS_REG) & 0x01))
		;

	/* enable the RTC interrupt -- TRM 20.3.5.16 */
    WRITEREG32(RTC_INTERRUPTS_REG, (periodicity << 2) | period);

    /* clear the interrupt mask bit for the RTC interrupt
	 * i.e, the 11th bit in MIR2, bits0-63 are in MIR0-1
	 * -- TRM 6.3 & 6.5.1.29
	*/
    WRITEREG32(INTC_MIR2_CLEAR, 0x01 << 11);
}

void hal_uart_putchar(u8 val)
{
	/* Wait for Tx hold reg to be empty - TRM 19.5.1.12 */
	while (!(READREG8(UART0_REGS_BASE + 0x14) & 0x20))
		;
	if (val == '\n')
		WRITEREG8(UART0_REGS_BASE, '\r');
	WRITEREG8(UART0_REGS_BASE, val);
}

void hal_uart_putstr(char *str)
{
	if (!str)
		return;
	
	while(*str) {
		hal_uart_putchar(*str);
		str++;
	}
}

void hal_uart_put32(u32 val)
{
	u32 mask = 0xF0000000;
	u8 cnt = 28;
	u32 op = 0;
	
	hal_uart_putchar('0');
	hal_uart_putchar('x');
	
	while (mask) {
		op = val & mask;
		op = op >> cnt;
		if (op >= 0 && op <= 9)
			hal_uart_putchar('0' + op);
		else if (op >= 0xA && op <= 0xF)
			hal_uart_putchar('A' + (op - 0xA));
		mask = mask >> 4;
		cnt -= 4;
	}

	hal_uart_putchar('\n');
}

void hal_init_uart()
{
	u32 val = 0;

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

	/* wait for TX FIFO and shift regs to be empty - TRM 19.5.1.12 */
	while (!(READREG8(UART0_REGS_BASE + UART0_LSR_OFFSET) & 0x40))
		;

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
	
	/* turn on usr1 led */
	WRITEREG32(GPIO1_SETDATAOUT, 0x01<<22);
}

char *hal_get_fault_typestr(u16 fault)
{
	switch (fault) {
		case ALIGNMENT_FAULT:
			return "alignment fault";
		case DEBUG_EVENT:
			return "debug event";
		case SEC_ACCESS_FAULT:
			return "section access flag fault";
		case INSTR_CACHE_FAULT:
			return "instruction cache maintainence fault";
		case SEC_TRANS_FAULT:
			return "section translation fault";
		case PAGE_ACCESS_FAULT:
			return "page access flag fault";
		case PAGE_TRANS_FAULT:
			return "page translation fault";
		case SEC_DOM_FAULT:
			return "section domain fault";
		case PAGE_DOM_FAULT:
			return "page domain fault";
		case L1_EXTERNAL_ABORT:
			return "L1 translation, precise external abort";
		case SEC_PERM_FAULT:
			return "section permission fault";
		case L2_EXTERNAL_ABORT:
			return "L2 translation, precise external abort";
		case PAGE_PERM_FAULT:
			return "page permission fault";
		case NON_TRANS_ABORT:
			return "non-translation, precise external abort";
		case IMPRECISE_EXT_ABORT:
			return "imprecise external abort";
		case IMPRECISE_ERR_ECC:
			return "imprecise error, parity or ECC";
		case L1_PARITY_ERROR:
			return "L1 translation, precise parity error";
		case L2_PARITY_ERROR:
			return "L2 translation, precise parity error";
		case AXI_EXT_NON_TRANS_ABORT:
			return "AXI slave error, non-translation, precise external abort";
		case AXI_L1_EXT_ABORT:
			return "AXI slave error, L1 translation, precise external abort";
		case AXI_L2_EXT_ABORT:
			return "AXI slave error, L2 translation, precise external abort";
	}
	return "unknown fault !";
}

/* this sets up uart, DDR3 and relocates durga to DDR3 RAM
 * this code is execed from internal L3 RAM
*/
void hal_init_platform_stage1()
{
	u32 *src = NULL, *dest = NULL;
	u32 *end = NULL;
	char str[32];

	hal_init_led();
	
	hal_init_uart();
	
	/* Note - we cannot pass const strings like below :
	 * -- hal_uart_putstr("DURGA stage 1 is loading...\n"); --
	 * because .rodata segment resides on address > 0x80000000 (in DDR3)
	 * due to the linker script, and these strings are alloc'd
	 * on the .rodata (read only data segment)
	 * so using these kind of strings will result in a crash
	 * we can however write such code in stage2 after DDR3 has been initialized
	*/
	str[0] = 'D'; str[1] ='U';
	str[2] = 'R'; str[3] = 'G';
	str[4] = 'A'; str[5] = ' ';
	str[6] = 's'; str[7] = 't';
	str[8] = 'a'; str[9] = 'g';
	str[10] = 'e'; str[11] = '1';
	str[12] = ' '; str[13] =  'l';
	str[14] = 'o'; str[15] = 'a';
	str[16] = 'd'; str[17] = 'i';
	str[18] = 'n'; str[19] = 'g';
	str[20] = '.'; str[21] = '.';
	str[22] = '.'; str[23] = '\n';
	str[24] = 0;
	hal_uart_putstr(str);
	
	hal_init_ddr3_ram();
	
	/* test the first 1MB DDR RAM onboard BBB
	* it will take long time to test all 512MB (around 20 min !)
	*/
	if (!hal_ram_test(0xDEADFACE, 1024 * 1024)) {
		str[0] = '*'; str[1] ='R';
		str[2] = 'A'; str[3] = 'M';
		str[4] = ' '; str[5] = 'c';
		str[6] = 'h'; str[7] = 'e';
		str[8] = 'c'; str[9] = 'k';
		str[10] = ' '; str[11] = 'f';
		str[12] = 'a'; str[13] =  'i';
		str[14] = 'l'; str[15] = '!';
		str[16] = 0;
		hal_uart_putstr(str);
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

__attribute__ ((interrupt ("FIQ")))
void hal_default_fiq_hdlr(void)
{
	hal_uart_putstr("FIQ ! \n");
}

/* see ARM gcc function attributes:
 * https://gcc.gnu.org/onlinedocs/gcc/ARM-Function-Attributes.html#ARM-Function-Attributes
*/
__attribute__ ((interrupt ("IRQ")))
void hal_default_irq_hdlr(void)
{
	u32 val = 0;

	/* ignore spurious interrupts -- TRM 6.5.1.4 */
	val = READREG32(INTC_SIR_IRQ);
	if (val & 0xFFFFFF80)
		goto intr_xit;
	
	/* chk the irq number - TRM 6.5.1.4 */
	val &= 0x7F;
	if (val >= INT_MAX_IRQS)
		goto intr_xit;
	
	/* jmp to specific IRQ handler */
	if (g_irq_table[val].irq_hdlr_fn)
		g_irq_table[val].irq_hdlr_fn(g_irq_table[val].data);
	else {
		hal_uart_putstr("No handler for IRQ - ");
		hal_uart_put32(val);
	}

intr_xit:
	/* allow pending/new IRQ's to occur */
	WRITEREG32(INTC_CTRL, 1);
}

u32 g_reg_cp15 = 0xDEADFACE;
u32 g_fault_addr = 0xFACEDEAD;

__attribute__ ((interrupt ("ABORT")))
void hal_default_databort_hdlr(void)
{
	u16 val = 0;
	char *type = NULL;

	/* in case of data abort LR-4 gives the addr of the faulting instr
	* - ARM A8 prog guide - 11.3.1
	*/
	asm volatile(
		"push {r5, r6} \n"
		"ldr r5, =g_fault_addr \n"
		"sub r6, lr, #4 \n"
		"str r6, [r5] \n"
		"pop {r5, r6} \n"
	);

	/* read data fault status reg c5 in cp15 - ARM A8 TRM - 3.2.35 */
	asm volatile(
		"push {r5, r6} \n"
		"ldr r5, =g_reg_cp15 \n"
		"mrc p15, 0, r6, c5, c0, 0 \n"
		"str r6, [r5] \n"
		"pop {r5, r6} \n"
	);
	/* combine bits 0-3 and 10,12 to get fault number */
	val = g_reg_cp15 & 0x140F;
	val |= (val & 0x400) >> 6;
	val |= (val & 0x1000) >> 7;
	val &= 0x3F;
	type = hal_get_fault_typestr(val);
	
	hal_uart_putstr("=========DATA ABORT !========= \nc5 reg = ");
	hal_uart_put32(g_reg_cp15);
	hal_uart_putstr("fault type = ");
	hal_uart_put32(val);
	hal_uart_putstr(type);
	if (g_reg_cp15 & 0x800)
		hal_uart_putstr("\ncaused by write access @ ");
	else
		hal_uart_putstr("\ncaused by read access @ ");
	
	
	/* read data fault addr. reg c6 in CP15 - ARM A8 TRM 3.2.38 */
	asm volatile(
		"push {r5, r6} \n"
		"ldr r5, =g_reg_cp15 \n"
		"mrc p15, 0, r6, c6, c0, 0 \n"
		"str r6, [r5] \n"
		"pop {r5, r6} \n"
	);
	
	hal_uart_put32(g_reg_cp15);
	hal_uart_putstr("fault caused by instruction @ ");
	hal_uart_put32(g_fault_addr);
	hal_uart_putstr("============================== \n");
	hal_assert();
}

__attribute__ ((interrupt ("ABORT")))
void hal_default_prefetch_abort_hdlr(void)
{
	u16 val = 0;
	char *type = NULL;

	/* in case of prefetch abort LR-8 gives the addr of the faulting instr
	* - ARM A8 prog guide - 11.3.1
	*/
	asm volatile(
		"push {r5, r6} \n"
		"ldr r5, =g_fault_addr \n"
		"sub r6, lr, #8 \n"
		"str r6, [r5] \n"
		"pop {r5, r6} \n"
	);

	/* read instr fault status reg c5 in cp15 - ARM A8 TRM - 3.2.36
	* note the '1' at the end of the mrc instr, its not the same as
	* reading the data fault status reg !
	*/
	asm volatile(
		"push {r5, r6} \n"
		"ldr r5, =g_reg_cp15 \n"
		"mrc p15, 0, r6, c5, c0, 1 \n"
		"str r6, [r5] \n"
		"pop {r5, r6} \n"
	);
	/* combine bits 0-3 and 10,12 to get fault number */
	val = g_reg_cp15 & 0x140F;
	val |= (val & 0x400) >> 6;
	val |= (val & 0x1000) >> 7;
	val &= 0x3F;
	type = hal_get_fault_typestr(val);
	
	hal_uart_putstr("=========PREFETCH ABORT !========= \nc5 reg = ");
	hal_uart_put32(g_reg_cp15);
	hal_uart_putstr("fault type = ");
	hal_uart_put32(val);
	hal_uart_putstr(type);
	hal_uart_putstr("\ncaused by trying to fetch instruction @ ");

	/* read instr fault addr. reg c6 in CP15 - ARM A8 TRM 3.2.39
	* note the '2' at the end of the mrc instr, its not the same as
	* reading the data fault addr reg !
	*/
	asm volatile(
		"push {r5, r6} \n"
		"ldr r5, =g_reg_cp15 \n"
		"mrc p15, 0, r6, c6, c0, 2 \n"
		"str r6, [r5] \n"
		"pop {r5, r6} \n"
	);
	
	hal_uart_put32(g_reg_cp15);
	hal_uart_putstr("fault caused by instruction @ ");
	hal_uart_put32(g_fault_addr);
	hal_uart_putstr("============================== \n");
	hal_assert();
	hal_assert();
}

__attribute__ ((interrupt ("SWI")))
void hal_default_swi_hdlr(void)
{
	hal_uart_putstr("SWI ! \n");
}

__attribute__ ((interrupt ("UNDEF")))
void hal_default_undef_hdlr(void)
{
	u32 *instr = NULL;
	/* in case of undef exception LR-4 gives the addr of the faulting instr	*/
	asm volatile(
		"push {r5, r6} \n"
		"ldr r5, =g_fault_addr \n"
		"sub r6, lr, #4 \n"
		"str r6, [r5] \n"
		"pop {r5, r6} \n"
	);
	instr = (u32 *)g_fault_addr;
	hal_uart_putstr("============UNDEFINED INSTRUCTION !========= \n");
	hal_uart_putstr("caused by trying to decode instruction: ");
	hal_uart_put32(*instr);
	hal_uart_putstr("at address: ");
	hal_uart_put32(g_fault_addr);
	hal_uart_putstr("============================================ \n");
	hal_assert();
}

void hal_default_reset_hdlr(void)
{
	hal_uart_putstr("RESET ! \n");
	hal_assert();
}

/* see ARM gcc function attributes:
 * https://gcc.gnu.org/onlinedocs/gcc/ARM-Function-Attributes.html#ARM-Function-Attributes
 * 'naked' attribute does not generate stack prologue/epilogue instructions
*/
__attribute__ ((naked))
void hal_intr_table(void)
{
	asm volatile (
		"ldr pc, [pc, #24] \n"
		"ldr pc, [pc, #24] \n"
		"ldr pc, [pc, #24] \n"
		"ldr pc, [pc, #24] \n"
		"ldr pc, [pc, #24] \n"
		"nop \n"
		"ldr pc, [pc, #20] \n"
		"ldr pc, [pc, #20] \n"
	);  
}

void hal_register_exception_handler(exception_type_t type, u32 *handler)
{
	u32 *p = (u32 *)INTR_TABLE_START;
	p += type;
	*p = (u32)handler;	
}

int hal_register_irq_handler(u8 intr_num, fp_irq_hdlr_t handler, void *data)
{
	if (intr_num >= INT_MAX_IRQS || !handler)
		return -1;
	
	hal_disable_intr();
	g_irq_table[intr_num].irq_hdlr_fn = handler;
	g_irq_table[intr_num].data = data;
	hal_enable_intr();
	return 0;
}

void hal_disable_intr()
{
	asm volatile (
		"stmfd sp!, {r0} \n"
		"mrs r0, cpsr \n"
		"orr r0, r0, #0xC0 \n" /* disable FIQ and IRQ */
		"msr cpsr, r0 \n"
		"ldmfd sp!, {r0} \n"
	);
}

void hal_enable_intr()
{
	asm volatile (
		"stmfd sp!, {r0} \n"
		"mrs r0, cpsr \n"
		"bic r0, r0, #0x80 \n" /* disable FIQ, but enable IRQ */
		"msr cpsr, r0 \n"
		"ldmfd sp!, {r0} \n"
	);
}

/* this initializes interrupts and intr handlers 
 * this code is execed from DDR3 RAM
*/
void hal_init_platform_stage2()
{
	u32 *p32 = NULL, *f32 = NULL;
	int i = 0;
	
	memset(g_irq_table, 0, sizeof(g_irq_table));

	hal_uart_putstr("DURGA stage 2 is loading.... \n");
	hal_uart_putstr("set VBAR...\n");
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

	hal_uart_putstr("copy intr table...\n");
	/* copy the interrupt table to the end of DDR3 RAM */
	p32 = (u32 *)(INTR_TABLE_START);
	f32 = (u32 *)hal_intr_table;
	for (i = 0; i < 8; ++i)
		p32[i] = f32[i];

	hal_uart_putstr("install default exception handlers...\n");
	/* install default exception handlers */
	hal_register_exception_handler(EXCEPTION_RESET, (u32 *)hal_default_reset_hdlr);
	hal_register_exception_handler(EXCEPTION_UNDEF, (u32 *)hal_default_undef_hdlr);
	hal_register_exception_handler(EXCEPTION_SWI, (u32 *)hal_default_swi_hdlr);
	hal_register_exception_handler(EXCEPTION_PREFETCH_ABORT, (u32 *)hal_default_prefetch_abort_hdlr);
	hal_register_exception_handler(EXCEPTION_DATABORT, (u32 *)hal_default_databort_hdlr);
	hal_register_exception_handler(EXCEPTION_IRQ, (u32 *)hal_default_irq_hdlr);
	hal_register_exception_handler(EXCEPTION_FIQ, (u32 *)hal_default_fiq_hdlr);
	
	hal_uart_putstr("init RTC...\n");
	/* initialize INTC registers */
	/* set intr 75 (RTC) as highest priority and type as irq -- TRM 6.5.1.44 */
	hal_init_intr(RTC_INTR_NUM, IRQ, 0);
	
	/* initialize RTC and RTC interrupts */
	hal_init_rtc_intr(EVERY_SEC, RTC_INTR_PERIODIC_ENABLE);
	
	hal_uart_putstr("enabling ARM interrupts...\n");
	/* enable interrupts on ARM side */
	hal_enable_intr();
	
	hal_uart_putstr("DONE \n");

	/* back to asm which will jump to main */
	return;
}

void hal_delay_1s()
{
/* Delay of approx. 1s - some dummy instructions executed in a tight loop
* ALU instructions take 1 cycle to exec, branch takes 3 cycles -- see Sec 6.3 in ARM system developers guide
* Extrapolate that to the 1ghz AM3358 processor on BBB for a loop count to get a 1s delay,
* but in reality the TI boot rom code would have set the proc to run at 500mhz instead of 1ghz 
*/	asm volatile (
		"stmfd sp!, {r7-r9} \n"
		"PROC_DELAY: \n"
		"ldr r8, =2403846 \n"
		"DELAY_LOOP1: \n"
		"add r7, r7, r9 \n"
		"add r7, r7, r9 \n"
		"add r7, r7, r9 \n"
		"add r7, r7, r9 \n"
		"add r7, r7, r9 \n"
		"add r7, r7, r9 \n"
		"add r7, r7, r9 \n"
		"add r7, r7, r9 \n"
		"sub r8, #1 \n"
		"cmp r8, #0 \n"
		"bne DELAY_LOOP1 \n"
		"ldmfd sp!, {r7-r9} \n"
	);
}

void hal_delay(u32 sec)
{
	while(sec--)
		hal_delay_1s();
}

void hal_assert()
{
	while (1) {
		hal_usr_led_print4(0xF);
		hal_delay(1);
		hal_usr_led_print4(0x0);
		hal_delay(1);
	}
}
