/*
* HW abstraction layer header for BeagleBoneBlack. Contains functions to init hw, operate peripherals and regs, and
* commonly used memory mapped addresses of different periperals/registers on the beagle bone black (TI Am335x)
* @author: muteX023
*/

#ifndef __BBB_HAL_H__
#define __BBB_HAL_H__

#include "types.h"

/* prototypes API*/
void hal_init_led();
void hal_usr_led_on(u8 led_num);
void hal_usr_led_off(u8 led_num);
void hal_init_ddr3_ram();
u8 hal_ram_test(u32 val, u64 size);


/* addresses - refer TRM of AM335x */
#define LOAD_ADDR 0x402f0400

#define CM_PER_GPIO1_CLKCTRL 0x44e000AC
#define GPIO1_OE 0x4804C134
#define GPIO1_SETDATAOUT 0x4804C194
#define GPIO1_CLEARDATAOUT 0x4804C190
#define GPIO1_DATAOUT 0x4804C13C

#define INTC_BASE 0x48200000 
#define RTC_BASE 0x44E3E000 
#define CM_RTC_BASE 0x44E00800 

#define INTC_SYSCONFING 0x48200010
#define INTC_CTRL 0X48200048
#define INTC_IDLE 0x48200050
#define INTC_ISR_SET0 0x48200090
#define INTC_ISR_CLEAR0 0x48200094
#define INTC_MIR2 0x482000C4
#define INTC_MIR2_CLEAR 0x482000C8
#define INTC_MIR0_CLEAR 0x48200088
#define INTC_ILR75 0X4820022C	/* 4h offset for each register 100h - 2fch */
#define INTC_SIR_IRQ 0X48200040
#define INTC_IRQ_PRIORITY 0x48200060
#define INT_IRQ_HDLR_RAM 0x4030CE38
#define INT_VEC_BASE_RAM 0x4030CE00
#define INT_VEC_BASE_ROM 0x20000
#define INT_IRQ_DEFAULT_HDLR 0x4030CE18

#define RTC_CTRL_REG 0X44E3E040
#define RTC_STATUS_REG 0X44E3E044
#define RTC_INTERRUPTS_REG 0X44E3E048
#define RTC_OSC_REG 0x44E3E054
#define RTC_INT_MASK 0XFFFFF7FF
#define RTC_KICK0R 0x44E3E06C
#define RTC_KICK1R 0x44E3E070
#define RTC_WRENABLE_KEY1 0x83E70B13
#define RTC_WRENABLE_KEY2 0x95A4F1E0

#define CM_RTC_RTC_CLKCTRL 0x44E00800
#define CM_RTC_CLKSCTRL 0x44E00804

#define DMTIMER7_TCLR 0x4804A038
#define CM_PER_TIMER7_CLKCTRL 0x44E0007C

#define CTRL_MODULE_REG_BASE 0x44E10000
#define CTRL_MODULE_VTP_CTRL_OFFSET 0xE0C
#define CTRL_MODULE_DDR_IO_CTRL_OFFSET 0xE04
#define DDR3_CTRL_MODULE_DDR_IO_CTRL_VAL 0xefffffff
#define CTRL_MODULE_DDR_CKE_CTRL_OFFSET 0x131C
#define CTRL_MODULE_EMIF_SDRAM_CONFIG 0x110

#define CM_WKUP_REGS_BASE 0x44E00400
#define CM_WKUP_CM_CLKMODE_DPLL_DDR_OFFSET 0x94
#define CM_WKUP_CM_IDLEST_DPLL_DDR_OFFSET 0x34
#define CM_WKUP_CM_CLKSEL_DPLL_DDR_OFFSET 0x40
#define CM_WKUP_CM_DIV_M2_DPLL_DDR_OFFSET 0xA0
#define CM_WKUP_CONTROL_CLKCTRL_OFFSET 0x4

#define DDR3_FREQ 303  /* if it doesn't work try 266 ? */
#define DDRPLL_N 23
#define DDRPLL_M2 1
#define DDR_PHY_REG_BASE 0x44E12000

#define CM_PER_REGS_BASE 0x44E00000
#define CM_PER_EMIF_FW_CLKCTRL_OFFSET 0xD0
#define CM_PER_EMIF_CLKCTRL_OFFSET 0x28
#define CM_PER_L3_CLKSTCTRL_OFFSET 0xC

#define CMD0_REG_PHY_CTRL_SLAVE_RATIO_0_OFFSET 0X1C
#define DDR3_CMD0_SLAVE_RATIO_0_VAL 0x40 
#define CMD0_REG_PHY_INVERT_CLKOUT_0_OFFSET 0x2C
#define DDR3_CMD0_REG_PHY_INVERT_CLKOUT_0_VAL 0x1

#define CMD1_REG_PHY_CTRL_SLAVE_RATIO_0_OFFSET 0X50
#define DDR3_CMD1_SLAVE_RATIO_0_VAL 0x40 
#define CMD1_REG_PHY_INVERT_CLKOUT_0_OFFSET 0x60
#define DDR3_CMD1_REG_PHY_INVERT_CLKOUT_0_VAL 0x1

#define CMD2_REG_PHY_CTRL_SLAVE_RATIO_0_OFFSET 0X84
#define DDR3_CMD2_SLAVE_RATIO_0_VAL 0x40
#define CMD2_REG_PHY_INVERT_CLKOUT_0_OFFSET 0x94
#define DDR3_CMD2_REG_PHY_INVERT_CLKOUT_0_VAL 0x1

#define DATA0_REG_PHY_RD_DQS_SLAVE_RATIO_0_OFFSET 0xC8
#define DDR3_DATA0_REG_PHY_RD_DQS_SLAVE_RATIO_0_VAL 0x3B
#define DATA0_REG_PHY_WR_DQS_SLAVE_RATIO_0_OFFSET 0xDC
#define DDR3_DATA0_REG_PHY_WR_DQS_SLAVE_RATIO_0_VAL 0x85
#define DATA0_REG_PHY_FIFO_WE_SLAVE_RATIO_0_OFFSET 0x108
#define DDR3_DATA0_REG_PHY_FIFO_WE_SLAVE_RATIO_0_VAL 0x100
#define DATA0_REG_PHY_WR_DATA_SLAVE_RATIO_0_OFFSET 0x120
#define DDR3_DATA0_REG_PHY_WR_DATA_SLAVE_RATIO_0_VAL 0xC1

#define DATA1_REG_PHY_RD_DQS_SLAVE_RATIO_0_OFFSET 0x16C
#define DDR3_DATA1_REG_PHY_RD_DQS_SLAVE_RATIO_0_VAL 0x3B
#define DATA1_REG_PHY_WR_DQS_SLAVE_RATIO_0_OFFSET 0x180
#define DDR3_DATA1_REG_PHY_WR_DQS_SLAVE_RATIO_0_VAL 0x85
#define DATA1_REG_PHY_FIFO_WE_SLAVE_RATIO_0_OFFSET 0x1AC
#define DDR3_DATA1_REG_PHY_FIFO_WE_SLAVE_RATIO_0_VAL 0x100
#define DATA1_REG_PHY_WR_DATA_SLAVE_RATIO_0_OFFSET 0x1C4
#define DDR3_DATA1_REG_PHY_WR_DATA_SLAVE_RATIO_0_VAL 0xC1


#define DATA0_REG_PHY_RD_DQS_SLAVE_RATIO_0_OFFSET 0xC8
#define DDR3_DATA0_REG_PHY_RD_DQS_SLAVE_RATIO_0_VAL 0x3B

#define DATA0_REG_PHY_RD_DQS_SLAVE_RATIO_0_OFFSET 0xC8
#define DDR3_DATA0_REG_PHY_RD_DQS_SLAVE_RATIO_0_VAL 0x3B

#define EMIF_REG_BASE 0x4C000000
#define EMIF_DDR_PHY_CTRL_1_OFFSET 0xE4
#define EMIF_DDR_PHY_CTRL_1_SHDW_OFFSET 0xE8
#define DDR3_DDR_PHY_CTRL_1_VAL 0x100006
#define EMIF_SDRAM_TIM_1_OFFSET 0x18
#define EMIF_SDRAM_TIM_1_SHDW_OFFSET 0x1C
#define DDR3_EMIF_SDRAM_TIM_1_VAL 0x0888A39B
#define EMIF_SDRAM_TIM_2_OFFSET 0x20
#define EMIF_SDRAM_TIM_2_SHDW_OFFSET 0x24
#define DDR3_EMIF_SDRAM_TIM_2_VAL 0x26337FDA
#define EMIF_SDRAM_TIM_3_OFFSET 0x28
#define EMIF_SDRAM_TIM_3_SHDW_OFFSET 0x2C
#define DDR3_EMIF_SDRAM_TIM_3_VAL 0x501F830F
#define EMIF_SDRAM_REF_CTRL_OFFSET  0x10
#define EMIF_SDRAM_REF_CTRL_SHDW_OFFSET  0x14
#define DDR3_EMIF_SDRAM_REF_CTRL_VAL 0x0000093B
#define EMIF_ZQ_CONFIG_OFFSET 0xC8
#define DDR3_EMIF_ZQ_CONFIG_VAL 0x50074BE4
#define EMIF_SDRAM_CONFIG_OFFSET 0x8
#define DDR3_EMIF_SDRAM_CONFIG_VAL 0x61C04AB2

#define EMIF_DDR3_RAM_START_ADDR 0x80000000

/* read and write register macros */
#define WRITEREG32(addr, val) ( *(volatile unsigned int *)(addr) = (val))
#define READREG32(addr) ( *(volatile unsigned int *)(addr) )

#endif /*__BBB_HAL_H__*/
