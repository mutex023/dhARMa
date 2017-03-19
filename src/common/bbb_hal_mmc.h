/*
 * bbb_hal_mmc.h
 *  MMC/SD card related routines for BBB
 *  Created on: Mar 15, 2017
 *  Author: muteX023
 */

#ifndef __BBB_HAL_MMC_H__
#define __BBB_HAL_MMC_H__

#include "types.h"

#define MMC0_REGS_BASE 0x48060000
#define MMC_REF_FREQ 96000000 /* 96MHz */
#define MMC_OUTPUT_FREQ 400000   /* 400kHz */

#define SD_PSTATE_OFFSET 0x224
#define SD_SYSCONFIG_OFFSET 0x110
#define SD_SYSSTATUS_OFFSET 0x114
#define SD_SYSCTL_OFFSET 0x22C
#define SD_CAPA_OFFSET 0x240
#define SD_CON_OFFSET 0x12C
#define SD_HCTL_OFFSET 0x228
#define SD_IE_OFFSET 0x234
#define SD_CMD_OFFSET 0x20C
#define SD_STAT_OFFSET 0x230
#define SD_ISE_OFFSET 0x238
#define SD_BLK_OFFSET 0x204
#define SD_ARG_OFFSET 0x208
#define SD_RSP10_OFFSET 0x210

#define DATADIR_WRITE 0
#define DATADIR_READ 1
#define DATABLK_SINGLE 0
#define DATABLK_MULTI 1
#define DATA_TRANSFER_TIMEOUT 0xE

typedef enum {
	NO_RESP,
	RESP_LEN_136,
	RESP_LEN_48,
	RESP_LEN_48_BUSY
} resp_type_t;

typedef enum {
	OTHERS,
	CMD52_BUS_SUSPEND,
	CMD52_FUNCTION_SELECT,
	CMD12_52_IO_ABORT
} spl_cmd_type_t;

typedef struct {
	u8 dma_enable;
	u8 block_cnt_enable;
	u8 auto_cmd12_enable;
	u8 data_dir;
	u8 multi_single_blk;
	resp_type_t resp_type;
	u8 crc_chk_enable;
	u8 idx_chk_enable;
	u8 data_present;
	spl_cmd_type_t spl_cmd_type;
	u8 cmd_idx;

	u16 nblks;
	u16 blk_size;
	u32 args;
	u32 resp[4];
} sdcmd_t;

int hal_mmc_init();
int hal_mmc_sdcard_init();
#endif /* __BBB_HAL_MMC_H__ */
