#ifndef __BBB_HAL_H__
#define __BBB_HAL_H__

/*
* Commonly used memory mapped addresses of different periperals/registers on the beagle bone black (TI Am335x)
*/

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
#define INTC_ILR75 0X4820022C	@4h offset for each register 100h - 2fch 
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

/* read and write register macros */
#define WRITEREG32(addr, val) ( *(volatile unsigned int *)(addr) = (val))
#define READREG32(addr) ( *(volatile unsigned int *)(addr) )

#endif /*__BBB_HAL_H__*/
