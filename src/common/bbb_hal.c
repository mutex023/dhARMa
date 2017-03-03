/* BeagleBoneBlack HW Abstraction layer
* performs various BBB hw and register ops and initialization
* @author: muteX023
*/

#include "bbb_hal.h"

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
	val &= ~(0xF << 21);
	WRITEREG32(GPIO1_OE, val);

	/* clear out gpio1-21,22,23,24 pins using data out register first*/
	WRITEREG32(GPIO1_CLEARDATAOUT, 0x0F<<21);
}

void hal_usr_led_toggle(u8 led_num)
{
	u32 val = 0;
	u32 led = 0;

	led = 0x1 << (led_num + 21);
	val = READREG32(GPIO1_SETDATAOUT);
	if (val & led)
		WRITEREG32(GPIO1_CLEARDATAOUT, led);
	else
		WRITEREG32(GPIO1_SETDATAOUT, led);
}

void hal_usr_led_on(u8 led_num)
{
	u32 val = 0;
	
	val = 0x1 << (led_num + 21);
	WRITEREG32(GPIO1_SETDATAOUT, val);
}

void hal_usr_led_off(u8 led_num)
{
	u32 val = 0;
	
	val = 0x1 << (led_num + 21);
	WRITEREG32(GPIO1_CLEARDATAOUT, val);
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
    /* set free running interrupt ctrl clock -- TRM 6.5.1.2/8 */
	WRITEREG32(INTC_SYSCONFING, 0);
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

void hal_delay_1s()
{
/* Delay of approx. 1s - some dummy instructions executed in a tight loop
* ALU instructions take 1 cycle to exec, branch takes 3 cycles -- see Sec 6.3 in ARM system developers guide
* Extrapolate that to the 1ghz AM3358 processor on BBB for a loop count to get a 1s delay,
* but in reality the TI boot rom code would have set the proc to run at 500mhz instead of 1ghz 
*/	asm volatile (
		"PROC_DELAY: \n"
		"ldr r8, =4807692 \n"
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
	);
}

void hal_delay(u32 sec)
{
	while(sec--)
		hal_delay_1s();
}