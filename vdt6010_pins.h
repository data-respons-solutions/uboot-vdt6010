#ifndef __VDT6010_PINS_H__
#define __VDT6010_PINS_H__

#include <asm/arch/mx6-pins.h>
#include <asm/arch/iomux.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/mxc_i2c.h>
#include "../common/include/mx6_common_defs.h"

static iomux_v3_cfg_t const uart5_pads[] = {
	IOMUX_PADS(PAD_CSI0_DAT14__UART5_TX_DATA	| MUX_PAD_CTRL(UART_PAD_CTRL)),
	IOMUX_PADS(PAD_CSI0_DAT15__UART5_RX_DATA	| MUX_PAD_CTRL(UART_PAD_CTRL)),
};

static iomux_v3_cfg_t const spi_nor_pads[] = {
	IOMUX_PADS(PAD_CSI0_DAT8__ECSPI2_SCLK | MUX_PAD_CTRL(SPI_PAD_CTRL)),
	IOMUX_PADS(PAD_CSI0_DAT9__ECSPI2_MOSI | MUX_PAD_CTRL(SPI_PAD_CTRL)),
	IOMUX_PADS(PAD_CSI0_DAT10__ECSPI2_MISO | MUX_PAD_CTRL(SPI_PAD_CTRL)),
	IOMUX_PADS(PAD_CSI0_DAT11__GPIO5_IO29 | MUX_PAD_CTRL(SPI_PAD_CTRL)), // cs
};

static iomux_v3_cfg_t const pwm_pads[] = {
	IOMUX_PADS(PAD_SD1_DAT3__PWM1_OUT | MUX_PAD_CTRL(OUT_LOW_PAD_CTRL)),
};

static iomux_v3_cfg_t const hw_rev_pads [] = {
	IOMUX_PADS(PAD_DISP0_DAT0__GPIO4_IO21 | MUX_PAD_CTRL(WEAK_PULLUP)), // HW_Setting_0
	IOMUX_PADS(PAD_DISP0_DAT1__GPIO4_IO22 | MUX_PAD_CTRL(WEAK_PULLUP)), // HW_Setting_1
	IOMUX_PADS(PAD_DISP0_DAT2__GPIO4_IO23 | MUX_PAD_CTRL(WEAK_PULLUP)), // HW_Setting_2
};

#endif
