/* Bare metal C program to initialize the onboard 512MB MIcron DDR3 RAM
* on BBB. The usr1 and usr2 led will be turned on if init is successful.
* The code to init the RAM is taken from TI Starterware code
* (~/bootloader/src/armv7a/am335x/bl_platform.c - DDRPLLInit(), EMIFInit(), DDR3PhyInit() & DDR3Init() )
* Initializing RAM involves setting different values to different registers
* and involves complex terminology like Phase Locked Loops (PLL), VTP (Voltage, temp, process)
* ODT (On die Termination), slave ratios, CAS/RAS (column/row addr strobe) etc..,
* It is best to understand as much as possible and if you find the going tough
* skip the things you don't understand :). As long as we are able to use the RAM for
* storing our data/code we are done.
* 
* @author: muteX023
*/

#include "../common/bbb_hal.h"

void usr_led_on(unsigned char led_num)
{
	unsigned int val = 0;
	
	val = 0x1 << (led_num + 21);
	WRITEREG32(GPIO1_SETDATAOUT, val);
}

void init_ddr_pll()
{
	unsigned int val = 0;
	
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

void init_emif()
{
	unsigned int val = 0;

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

void init_ddr_phy()
{
	unsigned int val = 0;

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

void init_ddr3_ram()
{
	unsigned int val = 0;
	
	/* initialize the DDR3 PLL */
	init_ddr_pll();
	
	/* enable the CM_WKUP clock control module - TRM 8.1.12.2.2 */
	WRITEREG32(CM_WKUP_REGS_BASE + CM_WKUP_CONTROL_CLKCTRL_OFFSET, 0x2);
	
	/* initialize the External mem interface EMIF */
	init_emif();
	
	/* initialize ddr VTP and phy regs */
	init_ddr_phy();

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

unsigned char ram_test(unsigned int start_val)
{
	int i = 0;
	unsigned int *p = (unsigned int *)EMIF_DDR3_RAM_START_ADDR;
	
	/* write incrementing values to the first 1Kb of RAM and read back */
	for (i = 0; i < 1024; ++i)
	{
		*p = start_val + i;
		++p;
	}
	p = (unsigned int *)EMIF_DDR3_RAM_START_ADDR;
	for (i = 0; i < 1024; ++i)
	{
		if (*p != (start_val + i))
			return 0;
		++p;
	}

	return 1;
}

void init_led()
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
}

void main()
{
	unsigned int *p = (unsigned int *)EMIF_DDR3_RAM_START_ADDR + 512;
	init_led();
	
	init_ddr3_ram();
	
	if(ram_test(0))
		usr_led_on(1);
	else
		usr_led_on(3); //FAIL !
	
	/* one more simple RAM test */
	*p = 3;
	*(p + 1) = 8;
	*(p + 2) = (*p) + (*(p + 1));
	if(*(p + 2) == 11)
		usr_led_on(2);
	else
		usr_led_on(3); //FAIL !
	
	while(1);
}
