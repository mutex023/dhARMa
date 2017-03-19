/* bbb_hal_mmc.c
 *  MMC/SD card related routines for BBB
 *  Created on: Mar 15, 2017
 *  Author: muteX023
 */

#include "bbb_hal_mmc.h"

#include "types.h"
#include "bbb_hal.h"
#include "utils.h"

int hal_mmc_send_sdcmd(sdcmd_t *cmd)
{
	u32 val = 0;
	int i = 0;

	if (!cmd)
		return -1;

	if (cmd->block_cnt_enable) {
		val |= cmd->blk_size;
		val |= (u32)cmd->nblks << 16;
		WRITEREG32(MMC0_REGS_BASE + SD_BLK_OFFSET, val);
	}

	WRITEREG32(MMC0_REGS_BASE + SD_ARG_OFFSET, cmd->args);

	val |= (u32)cmd->dma_enable;
	val |= (u32)cmd->block_cnt_enable << 1;
	val |= (u32)cmd->auto_cmd12_enable << 2;
	val |= (u32)cmd->data_dir << 4;
	val |= (u32)cmd->multi_single_blk << 5;
	val |= (u32)cmd->resp_type << 16;
	val |= (u32)cmd->crc_chk_enable << 19;
	val |= (u32)cmd->idx_chk_enable << 20;
	val |= (u32)cmd->data_present << 21;
	val |= (u32)cmd->spl_cmd_type << 22;
	val |= (u32)cmd->cmd_idx << 24;

	if (cmd->data_present) {
		/* clear transfer complete flag in SD_STAT reg */
		WRITEREG32(MMC0_REGS_BASE + SD_STAT_OFFSET, 0x2);
		/* set data transfer timeout */
		val = READREG32(MMC0_REGS_BASE + SD_SYSCTL_OFFSET);
		val &= ~0xF0000;
		val |= DATA_TRANSFER_TIMEOUT << 16;
		WRITEREG32(MMC0_REGS_BASE + SD_SYSCTL_OFFSET, val);
	}

	WRITEREG32(MMC0_REGS_BASE + SD_CMD_OFFSET, val);

	/* check status of the sent cmd */
	while (1) {
		val = READREG32(MMC0_REGS_BASE + SD_STAT_OFFSET);
		if (val & 0x1) {
			/* CC - cmd complete bit is set, get the response if required */
			if (cmd->resp_type != NO_RESP) {
				/* read all four response registers - SD_RSP10,32,54,76 */
				for (i = 0; i < 4; i++) {
					cmd->resp[i] = READREG32(MMC0_REGS_BASE + SD_RSP10_OFFSET + (4 * i));
				}
			}
			return 0;
		} else {
			if (val & 0x8000)
				return -1;
		}
	}

	return -1;
}

/*TODO - in the future change this to take the MMCx base addr as argument
 * currently it only configures mmc0
*/
int hal_mmc_sdcard_init()
{
	int ret = 0;
	sdcmd_t cmd;
	/* card detection, identification & selection phase2 - TRM 18.4.3.2 */
	/* send CMD0 - reset the card */

	memset(&cmd, 0, sizeof(cmd));
	cmd.cmd_idx = 0;
	cmd.data_dir = DATADIR_WRITE;
	cmd.multi_single_blk = DATABLK_SINGLE;
	cmd.resp_type = NO_RESP;

	hal_uart_putstr("sending CMD0 ....");
	if (hal_mmc_send_sdcmd(&cmd)) {
		hal_uart_putstr("FAILED !\n");
		ret = -1;
	}
	else
		hal_uart_putstr("success. \n");

	return ret;
}

/*TODO - in the future change this to take the MMCx base addr as argument
 * currently it only configures mmc0
*/
int hal_mmc_init()
{
	u32 val = 0;
	u32 freq = 0;

	/* See AM335x TRM sec 18.4 on how to initialize MMC */

	/* 1. enable clock for MMC module - TRM 8.1.12.1.14 */
	WRITEREG32(CM_PER_REGS_BASE + CM_PER_MMC0_CLKCTRL_OFFSET, 0x2);
	/* wait for the module to be enabled */
	while ((READREG32(CM_PER_REGS_BASE + CM_PER_MMC0_CLKCTRL_OFFSET)
			& 0x3) != 0x2)
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
	else {
		hal_uart_putstr("no card inserted !\n");
		return -1;
	}

	/* 4. perform a soft reset - TRM 18.4.2.2 */
	WRITEREG32(MMC0_REGS_BASE + SD_SYSCONFIG_OFFSET, 0x2);
	while ((READREG32(MMC0_REGS_BASE + SD_SYSSTATUS_OFFSET) & 0x1)
			!= 0x1)
		;

	/* 5. perform soft reset for all the controller lines - TRM 18.5.1.18 */
	WRITEREG32(MMC0_REGS_BASE + SD_SYSCTL_OFFSET, 0x1000000);
	while (READREG32(MMC0_REGS_BASE + SD_SYSCTL_OFFSET) & 0x1000000)
		;

	/* See TRM 18.4.2.5 for MMC host and bus config */
	/* 6. set supported card voltage to 1.8V and 3.0V - TRM 18.5.1.23 */
	val = READREG32(MMC0_REGS_BASE + SD_CAPA_OFFSET);
	val &= ~0x6000000;
	WRITEREG32(MMC0_REGS_BASE + SD_CAPA_OFFSET, val);
	val = READREG32(MMC0_REGS_BASE + SD_CAPA_OFFSET);
	val |= 0x6000000;
	WRITEREG32(MMC0_REGS_BASE + SD_CAPA_OFFSET, val);

	/* 7. disable auto-idle mode TRM 18.5.1.1 */
	WRITEREG32(MMC0_REGS_BASE + SD_SYSCONFIG_OFFSET, 0x0);

	/* 8. Set bus width to 1 bit - we can't set mmc0 to 8-bit width
	 * as per the BBB SRM 5.3.3, also 1-bit width is supported by all
	 * sd cards. Set bus voltage to 3V - TRM 18.5.1.5/17 */
	val = READREG32(MMC0_REGS_BASE + SD_CON_OFFSET);
	val &= ~0x20;
	WRITEREG32(MMC0_REGS_BASE + SD_CON_OFFSET, val);
	val = READREG32(MMC0_REGS_BASE + SD_HCTL_OFFSET);
	val &= ~(0x2 | 0xE00);
	val |= (0x6 << 9);
	WRITEREG32(MMC0_REGS_BASE + SD_HCTL_OFFSET, val);

	/* 9. turn on bus power - TRM 18.5.1.17 */
	val = READREG32(MMC0_REGS_BASE + SD_HCTL_OFFSET);
	val |= 0x100;
	WRITEREG32(MMC0_REGS_BASE + SD_HCTL_OFFSET, val);
	while (!(READREG32(MMC0_REGS_BASE + SD_HCTL_OFFSET) & 0x100))
		;

	/* 10. enable internal clock - TRM 18.5.1.18 */
	val = READREG32(MMC0_REGS_BASE + SD_SYSCTL_OFFSET);
	val |= 0x1;
	WRITEREG32(MMC0_REGS_BASE + SD_SYSCTL_OFFSET, val);
	/* wait for internal clock to stabilize */
	while (!(READREG32(MMC0_REGS_BASE + SD_SYSCTL_OFFSET) & 0x2))
		;

	/* 11. set the bus clock freq - TRM 18.5.1.18 */
	val = READREG32(MMC0_REGS_BASE + SD_SYSCTL_OFFSET);
	freq = MMC_REF_FREQ/MMC_OUTPUT_FREQ;
	val &= ~0xFFC0;
	val |= freq << 6;
	WRITEREG32(MMC0_REGS_BASE + SD_SYSCTL_OFFSET, val);
	/* wait for internal clock to stabilize */
	while (!(READREG32(MMC0_REGS_BASE + SD_SYSCTL_OFFSET) & 0x2))
		;
	/* enable the clock to the card */
	val = READREG32(MMC0_REGS_BASE + SD_SYSCTL_OFFSET);
	val |= 0x4;
	WRITEREG32(MMC0_REGS_BASE + SD_SYSCTL_OFFSET, val);

	/* 12. card detection, identification & selection phase1 - TRM 18.4.3.2 */
	/* enable interrupt on cmd complete (CC) - TRM 18.5.1.20 */
	val = READREG32(MMC0_REGS_BASE + SD_IE_OFFSET);
	val |= 0x1;
	WRITEREG32(MMC0_REGS_BASE + SD_IE_OFFSET, val);
	/* start initialization stream - set SD_CON_OFFSET init bit to 1
	 * and write 0 to SD_CMD_OFFSET
	 */
	val = READREG32(MMC0_REGS_BASE + SD_CON_OFFSET);
	val |= 0x2;
	WRITEREG32(MMC0_REGS_BASE + SD_CON_OFFSET, val);
	WRITEREG32(MMC0_REGS_BASE + SD_CMD_OFFSET, 0);
	/* wait for command to complete - check status reg */
	while (!(val = READREG32(MMC0_REGS_BASE + SD_STAT_OFFSET) & 0x1))
		;
	/* clear the status by writing 1 to CC bit */
	val &= ~0x1;
	WRITEREG32(MMC0_REGS_BASE + SD_STAT_OFFSET, val);
	/* end the initialization sequence by writing 0 to SD_CON_OFFSET init bit */
	val = READREG32(MMC0_REGS_BASE + SD_CON_OFFSET);
	val &= ~0x2;
	WRITEREG32(MMC0_REGS_BASE + SD_CON_OFFSET, val);
	/* clear the SD_STAT_OFFSET reg by writing 1's */
	WRITEREG32(MMC0_REGS_BASE + SD_STAT_OFFSET, 0xFFFFFFFF);

	/* enable interrupts and intr status bits for cmd completion, timeout, data timeout and
	 * transfer complete - TRM 18.5.1.21
	 */
	val = 0x110003;
	WRITEREG32(MMC0_REGS_BASE + SD_IE_OFFSET, val);
	WRITEREG32(MMC0_REGS_BASE + SD_ISE_OFFSET, val);

	hal_uart_putstr("mmc init done. \n");
	return 0;
}
