/* Bare metal C program to initialize the onboard 512MB MIcron DDR3 RAM
* on BBB. The usr0 led will be turned on if init is successful.
* The code to init the RAM is taken from TI Starterware code
* (~/bootloader/src/armv7a/am335x/bl_platform.c - DDRPLLInit(), EMIFInit(), DDR3PhyInit() & DDR3Init() )
* Initializing RAM involves setting different values to different registers
* and involves complex terminology like Phase Locked Loops (PLL),
* ODT (On die Termination), slave ratios, CAS/RAS (column/row addr strobe) etc..,
* It is best to understand as much as possible and if you find the going tough
* skip the things you don't understand :). As long as we are able to use the RAM for
* storing our data/code we are done.
* 
* @author: muteX023
*/

#include "../common/bbb_hal.h"

void init_ddr_pll()
{
	unsigned int val = 0;
	
	/* put the DPLL in MN bypass mode - no idea what this means - TRM 8.1.12.2.38 */
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

int init_ddr3_ram()
{
	unsigned int val = 0;
	
	init_ddr_pll();
	
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
	
	while(1);
}
