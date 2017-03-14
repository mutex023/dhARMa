/*
* HW abstraction layer header for BeagleBoneBlack. Contains functions to init hw, operate peripherals and regs, and
* commonly used memory mapped addresses of different periperals/registers on the beagle bone black (TI Am335x)
* @author: muteX023
*/

#ifndef __BBB_HAL_H__
#define __BBB_HAL_H__

#include "types.h"

/* addresses - refer TRM of AM335x */
#define LOAD_ADDR 0x402F0400

#define CM_PER_GPIO1_CLKCTRL 0x44e000AC
#define GPIO1_OE 0x4804C134
#define GPIO1_USRLED_SHIFT 21
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
#define INTC_MIR2_SET 0x482000CC
#define INTC_MIR2_CLEAR 0x482000C8
#define INTC_MIR0_CLEAR 0x48200088
#define INTC_ILR_BASE 0x48200100
#define INTC_ILR75 0X4820022C	/* 4h offset for each register 100h - 2fch */
#define INTC_SIR_IRQ 0X48200040
#define INTC_IRQ_PRIORITY 0x48200060
#define INT_IRQ_HDLR_RAM 0x4030CE38
#define INT_VEC_BASE_RAM 0x4030CE00
#define INT_VEC_BASE_ROM 0x20000
#define INT_IRQ_DEFAULT_HDLR 0x4030CE18
#define INT_MAX_IRQS 128

#define RTC_CTRL_REG 0X44E3E040
#define RTC_STATUS_REG 0X44E3E044
#define RTC_INTERRUPTS_REG 0X44E3E048
#define RTC_OSC_REG 0x44E3E054
#define RTC_INT_MASK 0XFFFFF7FF
#define RTC_KICK0R 0x44E3E06C
#define RTC_KICK1R 0x44E3E070
#define RTC_WRENABLE_KEY1 0x83E70B13
#define RTC_WRENABLE_KEY2 0x95A4F1E0
#define RTC_INTR_NUM 75

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
#define CTRL_MODULE_CONF_UART0_RXD_OFFSET 0x970
#define CTRL_MODULE_CONF_UART0_TXD_OFFSET 0x974

#define CM_WKUP_REGS_BASE 0x44E00400
#define CM_WKUP_CLKSTCTRL_OFFSET 0x0
#define CM_WKUP_CM_CLKMODE_DPLL_DDR_OFFSET 0x94
#define CM_WKUP_CM_IDLEST_DPLL_DDR_OFFSET 0x34
#define CM_WKUP_CM_CLKSEL_DPLL_DDR_OFFSET 0x40
#define CM_WKUP_CM_DIV_M2_DPLL_DDR_OFFSET 0xA0
#define CM_WKUP_CONTROL_CLKCTRL_OFFSET 0x4
#define CM_WKUP_UART0_CLKCTRL_OFFSET 0xB4

#define DDR3_FREQ 303  /* if it doesn't work try 266 ? */
#define DDRPLL_N 23
#define DDRPLL_M2 1
#define DDR_PHY_REG_BASE 0x44E12000

#define CM_PER_REGS_BASE 0x44E00000
#define CM_PER_EMIF_FW_CLKCTRL_OFFSET 0xD0
#define CM_PER_EMIF_CLKCTRL_OFFSET 0x28
#define CM_PER_L3_CLKSTCTRL_OFFSET 0xC
#define CM_PER_L4HS_CLKSTCTRL_OFFSET 0x11C
#define CM_PER_UART1_CLKCTRL_OFFSET 0x6C

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

#define UART0_REGS_BASE 0x44E09000
#define UART0_SYSC_OFFSET 0x54
#define UART0_SYSS_OFFSET 0x58
#define UART0_LSR_OFFSET 0x14

/* read and write register macros */
#define WRITEREG32(addr, val) ( *(volatile unsigned int *)(addr) = (val))
#define READREG32(addr) ( *(volatile unsigned int *)(addr) )
#define WRITEREG8(addr, val) ( *(volatile unsigned char *)(addr) = (val))
#define READREG8(addr) ( *(volatile unsigned char *)(addr) )


typedef enum {
	IRQ,
	FIQ
} intr_type_t;

typedef enum {
	RTC_INTR_PERIODIC_DISABLE = 0x0,
	RTC_INTR_PERIODIC_ENABLE = 0x1
} rtc_intr_periodicity_t;

typedef enum {
	EVERY_SEC,
	EVERY_MIN,
	EVERY_HOUR,
	EVERY_DAY
} rtc_intr_period_t;

typedef enum {
	EXCEPTION_RESET = 8,
	EXCEPTION_UNDEF,
	EXCEPTION_SWI,
	EXCEPTION_PREFETCH_ABORT,
	EXCEPTION_DATABORT,
	EXCEPTION_IRQ,
	EXCEPTION_FIQ
} exception_type_t;

typedef enum {
	ALIGNMENT_FAULT = 0x1,
	DEBUG_EVENT = 0x2,
	SEC_ACCESS_FAULT = 0x3,
	INSTR_CACHE_FAULT = 0x4,
	SEC_TRANS_FAULT = 0x5,
	PAGE_ACCESS_FAULT = 0x6,
	PAGE_TRANS_FAULT = 0x7,
	NON_TRANS_ABORT = 0x8,
	SEC_DOM_FAULT = 0x9,
	PAGE_DOM_FAULT = 0xB,
	L1_EXTERNAL_ABORT = 0xC,
	SEC_PERM_FAULT = 0xD,
	L2_EXTERNAL_ABORT = 0xE,
	PAGE_PERM_FAULT = 0xF,
	IMPRECISE_EXT_ABORT = 0x16,
	IMPRECISE_ERR_ECC = 0x18,
	L1_PARITY_ERROR = 0x1C,
	L2_PARITY_ERROR = 0x1E,
	AXI_EXT_NON_TRANS_ABORT = 0x28,
	AXI_L1_EXT_ABORT = 0x2C,
	AXI_L2_EXT_ABORT = 0x2E,	
} fault_type_t;

typedef void (*fp_irq_hdlr_t)(void *data);
typedef struct {
	fp_irq_hdlr_t irq_hdlr_fn;
	void *data;
} irq_hdlr_t;

/* prototypes API*/
void hal_init_led();
void hal_usr_led_on(u8 led_num);
void hal_usr_led_off(u8 led_num);
void hal_usr_led_toggle(u8 led_num);
void hal_usr_led_print32(u32 val);
void hal_usr_led_print4(u8 val);
void hal_init_ddr3_ram();
u8 hal_ram_test(u32 val, u64 size);
void hal_init_intr(u32 intr_num, intr_type_t intr_type, u8 priority);
void hal_init_rtc_intr(rtc_intr_period_t period, rtc_intr_periodicity_t periodicity);
void hal_uart_putchar(u8 val);
void hal_uart_putstr(char *str);
void hal_uart_put32(u32 val);
void hal_init_uart();
void hal_disable_intr();
void hal_enable_intr();
void hal_register_exception_handler(exception_type_t type, u32 *handler);
int hal_register_irq_handler(u8 intr_num, fp_irq_hdlr_t handler, void *data);
void hal_delay(u32 sec);
void hal_assert();

#endif /*__BBB_HAL_H__*/
