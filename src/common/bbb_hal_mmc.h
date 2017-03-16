/*
 * bbb_hal_mmc.h
 *  MMC/SD card related routines for BBB
 *  Created on: Mar 15, 2017
 *  Author: muteX023
 */

#ifndef __BBB_HAL_MMC_H__
#define __BBB_HAL_MMC_H__

#define MMC0_REGS_BASE 0x48060000
#define MMC_REF_FREQ 96000000 /* 96MHz */
#define MMC_OUTPUT_FREQ 400000   /* 400kHz */

#define SD_PSTATE_OFFSET 0x224
#define SD_SYSCONFIG 0x110
#define SD_SYSSTATUS 0x114
#define SD_SYSCTL 0x22C
#define SD_CAPA 0x240
#define SD_CON 0x12C
#define SD_HCTL 0x228

#endif /* __BBB_HAL_MMC_H__ */
