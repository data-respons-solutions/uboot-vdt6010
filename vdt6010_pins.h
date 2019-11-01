#ifndef __VDT6010_PINS_H__
#define __VDT6010_PINS_H__

#include <asm/arch/mx6-pins.h>
#include <asm/arch/iomux.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/mxc_i2c.h>
#include "../common/mx6_common_defs.h"

static iomux_v3_cfg_t const uart1_pads[] = {
	IOMUX_PADS(PAD_EIM_D24__UART1_DTR_B | MUX_PAD_CTRL(UART_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT6__UART1_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D20__UART1_CTS_B | MUX_PAD_CTRL(UART_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_EB3__UART1_RI_B | MUX_PAD_CTRL(UART_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT7__UART1_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D25__UART1_DSR_B | MUX_PAD_CTRL(UART_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D19__UART1_RTS_B | MUX_PAD_CTRL(UART_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D23__UART1_DCD_B | MUX_PAD_CTRL(UART_PAD_CTRL)),
	IOMUX_PADS(PAD_KEY_ROW2__GPIO4_IO11 | MUX_PAD_CTRL(NO_PAD_CTRL)), // UART1_RS485EN2
	IOMUX_PADS(PAD_KEY_COL2__GPIO4_IO10 | MUX_PAD_CTRL(NO_PAD_CTRL)), // UART1_RS422EN2
	IOMUX_PADS(PAD_KEY_ROW1__GPIO4_IO09 | MUX_PAD_CTRL(NO_PAD_CTRL)), // UART1_RS232EN2
};

static iomux_v3_cfg_t const uart5_pads[] = {
	IOMUX_PADS(PAD_CSI0_DAT14__UART5_TX_DATA	| MUX_PAD_CTRL(UART_PAD_CTRL)),
	IOMUX_PADS(PAD_CSI0_DAT15__UART5_RX_DATA	| MUX_PAD_CTRL(UART_PAD_CTRL)),
};

static iomux_v3_cfg_t const gpio_pads[] = {
	IOMUX_PADS(PAD_DISP0_DAT9__GPIO4_IO30 | MUX_PAD_CTRL(NO_PAD_CTRL)), //GPIO1
	IOMUX_PADS(PAD_DISP0_DAT10__GPIO4_IO31 | MUX_PAD_CTRL(NO_PAD_CTRL)), // GPIO2
	IOMUX_PADS(PAD_DISP0_DAT11__GPIO5_IO05 | MUX_PAD_CTRL(NO_PAD_CTRL)), // GPIO3
	IOMUX_PADS(PAD_DISP0_DAT12__GPIO5_IO06 | MUX_PAD_CTRL(NO_PAD_CTRL)), // GPIO4
	IOMUX_PADS(PAD_DISP0_DAT13__GPIO5_IO07 | MUX_PAD_CTRL(NO_PAD_CTRL)), // GPIO5
	IOMUX_PADS(PAD_DISP0_DAT14__GPIO5_IO08 | MUX_PAD_CTRL(NO_PAD_CTRL)), // GPIO6
	IOMUX_PADS(PAD_DISP0_DAT15__GPIO5_IO09 | MUX_PAD_CTRL(NO_PAD_CTRL)), // GPIO7
	IOMUX_PADS(PAD_DISP0_DAT16__GPIO5_IO10 | MUX_PAD_CTRL(NO_PAD_CTRL)), // GPIO8
};

static iomux_v3_cfg_t const enet_pads[] = {
	IOMUX_PADS(PAD_ENET_MDC__ENET_MDC	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_ENET_MDIO__ENET_MDIO	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_ENET_CRS_DV__ENET_RX_EN	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_ENET_RX_ER__ENET_RX_ER | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_ENET_TX_EN__ENET_TX_EN | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_ENET_RXD0__ENET_RX_DATA0	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_ENET_RXD1__ENET_RX_DATA1	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_ENET_TXD0__ENET_TX_DATA0	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_ENET_TXD1__ENET_TX_DATA1	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_CS1__GPIO6_IO14 | MUX_PAD_CTRL(NO_PAD_CTRL)),	// LAN8710 nRST
	IOMUX_PADS(PAD_GPIO_9__GPIO1_IO09 | MUX_PAD_CTRL(NO_PAD_CTRL)),		// LAN8710 IRQ#
	IOMUX_PADS(PAD_GPIO_16__ENET_REF_CLK | MUX_PAD_CTRL(ENET_PAD_CTRL)),
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

static iomux_v3_cfg_t const other_pads[] = {
	IOMUX_PADS(PAD_GPIO_0__GPIO1_IO00 | MUX_PAD_CTRL(NO_PAD_CTRL)), // GPIO_MCU
	IOMUX_PADS(PAD_SD3_RST__GPIO7_IO08 | MUX_PAD_CTRL(NO_PAD_CTRL)), // DEBUG_LED
	IOMUX_PADS(PAD_GPIO_3__GPIO1_IO03 | MUX_PAD_CTRL(NO_PAD_CTRL)), // RTC_IRQ
	IOMUX_PADS(PAD_GPIO_17__GPIO7_IO12 | MUX_PAD_CTRL(NO_PAD_CTRL)), // LED_EN
	IOMUX_PADS(PAD_KEY_COL1__GPIO4_IO08 | MUX_PAD_CTRL(NO_PAD_CTRL)), // PCIE_RST_B
};

#endif
