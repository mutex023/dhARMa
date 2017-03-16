/* bbb_hal_mmc.c
 *  MMC/SD card related routines for BBB
 *  Created on: Mar 15, 2017
 *  Author: muteX023
 */

#include "bbb_hal_mmc.h"

#include "types.h"
#include "bbb_hal.h"

/*TODO - in the future change this to take the MMCx base addr as argument
 * currently it only configures mmc0
*/
void hal_mmc_init()
{
	u32 val = 0;
	u32 freq = 0;

	/* See AM335x TRM sec 18.4 on how to initialize MMC */

	/* 1. enable clock for MMC module - TRM 8.1.12.1.14 */
	WRITEREG32(CM_PER_REGS_BASE + CM_PER_MMC0_CLKCTRL_OFFSET, 0x2);
	/* wait for the module to be enabled */
	while (READREG32(CM_PER_REGS_BASE + CM_PER_MMC0_CLKCTRL_OFFSET)
			& 0x3 != 0x2)
		;
	/* 2. perform pad muxing and config for MMC and SPI (Serial port interface)
	 * select pullup type and enable receiver. Why is SPI required ? No idea...
	*/
	WRITEREG32(CTRL_MODULE_REG_BASE + CTRL_MODULE_CONF_MMC0_DAT3_OFFSET, 0x48);
	WRITEREG32(CTRL_MODULE_REG_BASE + CTRL_MODULE_CONF_MMC0_DAT2_OFFSET, 0x48);
	WRITEREG32(CTRL_MODULE_REG_BASE + CTRL_MODULE_CONF_MMC0_DAT1_OFFSET, 0x48);
	WRITEREG32(CTRL_MODULE_REG_BASE + CTRL_MODULE_CONF_MMC0_DAT0_OFFSET, 0x48);
	WRITEREG32(CTRL_MODULE_REG_BASE + CTRL_MODULE_CONF_MMC0_CLK_OFFSET, 0x48);
	WRITEREG32(CTRL_MODULE_REG_BASE + CTRL_MODULE_CONF_MMC0_CMD_OFFSET, 0x48);
	/* set signal mux select as 5 for spi0 conf */
	WRITEREG32(CTRL_MODULE_REG_BASE + CTRL_MODULE_CONF_SPI0_CS1_OFFSET, 0x53);

	/* 3. check if card is inserted - TRM 18.5.1.16 */
	if (READREG32(MMC0_REGS_BASE + SD_PSTATE_OFFSET) & 0x10000)
		hal_uart_putstr("card inserted \n");
	else
		hal_uart_putstr("no card inserted !\n");

	/* 4. perform a soft reset - TRM 18.4.2.2 */
	WRITEREG32(MMC0_REGS_BASE + SD_SYSCONFIG, 0x2);
	while ((READREG32(MMC0_REGS_BASE + SD_SYSSTATUS) & 0x1)
			!= 0x1)
		;

	/* 5. perform soft reset for all the controller lines - TRM 18.5.1.18 */
	WRITEREG32(MMC0_REGS_BASE + SD_SYSCTL, 0x1000000);
	while (READREG32(MMC0_REGS_BASE + SD_SYSCTL) & 0x1000000)
		;

	/* See TRM 18.4.2.5 for MMC host and bus config */
	/* 6. set supported card voltage to 1.8V and 3.0V - TRM 18.5.1.23 */
	val = READREG32(MMC0_REGS_BASE + SD_CAPA);
	val &= ~0x6000000;
	WRITEREG32(MMC0_REGS_BASE + SD_CAPA, val);
	val = READREG32(MMC0_REGS_BASE + SD_CAPA);
	val |= 0x6000000;
	WRITEREG32(MMC0_REGS_BASE + SD_CAPA, val);

	/* 7. disable auto-idle mode TRM 18.5.1.1 */
	WRITEREG32(MMC0_REGS_BASE + SD_SYSCONFIG, 0x0);

	/* 8. Set bus width to 1 bit - we can't set mmc0 to 8-bit width
	 * as per the BBB SRM 5.3.3, also 1-bit width is supported by all
	 * sd cards. Set bus voltage to 3V - TRM 18.5.1.5/17 */
	val = READREG32(MMC0_REGS_BASE + SD_CON);
	val &= ~0x20;
	WRITEREG32(MMC0_REGS_BASE + SD_CON, val);
	val = READREG32(MMC0_REGS_BASE + SD_HCTL);
	val &= ~(0x2 | 0xE00);
	val |= (0x6 << 9)
	WRITEREG32(MMC0_REGS_BASE + SD_HCTL, val);

	/* 9. turn on bus power - TRM 18.5.1.17 */
	val = READREG32(MMC0_REGS_BASE + SD_HCTL);
	val |= 0x100;
	WRITEREG32(MMC0_REGS_BASE + SD_HCTL, val);
	while (!(READREG32(MMC0_REGS_BASE + SD_HCTL) & 0x100))
		;

	/* 10. enable internal clock - TRM 18.5.1.18 */
	val = READREG32(MMC0_REGS_BASE + SD_SYSCTL);
	val |= 0x1;
	WRITEREG32(MMC0_REGS_BASE + SD_SYSCTL, val);
	/* wait for internal clock to stabilize */
	while (!(READREG32(MMC0_REGS_BASE + SD_SYSCTL) & 0x2))
		;

	/* 11. set the bus clock freq - TRM 18.5.1.18 */
	val = READREG32(MMC0_REGS_BASE + SD_SYSCTL);
	freq = MMC_REF_FREQ/MMC_OUTPUT_FREQ;
	val &= ~0xFFC0;
	val |= freq << 6;
	WRITEREG32(MMC0_REGS_BASE + SD_SYSCTL, val);
	/* wait for internal clock to stabilize */
	while (!(READREG32(MMC0_REGS_BASE + SD_SYSCTL) & 0x2))
		;

}
