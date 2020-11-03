/*
 * Copyright (C) 2019 Data Respons Solutions AB
 *
 * Author: Mikko Salomäki <ms@datarespons.se>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/mach-imx/spi.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/arch/crm_regs.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <version.h>
#include <watchdog.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/mx6-ddr.h>
#include <spl.h>
#include <asm/mach-imx/hab.h>
#include "../common/include/mx6_common_defs.h"

DECLARE_GLOBAL_DATA_PTR;

#define SPI_NOR_CS		IMX_GPIO_NR(5, 29)

int board_spi_cs_gpio(unsigned bus, unsigned cs)
{
	if (bus == 1 && cs == 0) {
		return SPI_NOR_CS;
	}

	return -1;
}

#define MX6DQ_DDR_TYPE_DD3 0x000C0000
#define MX6DQ_DDRPKE_DISABLED 0x00000000
#define MX6DQ_DS_48_OHM 0x00000028
#define MX6DQ_DS_40_OHM 0x00000030
#define MX6DQ_DS_GRP 0x00000000
#define MX6DQ_INPUT_DIFFERENTIAL 0x00020000

static const struct mx6dq_iomux_ddr_regs mx6dq_ddr_ioregs = {
	.dram_sdclk_0 = MX6DQ_INPUT_DIFFERENTIAL | MX6DQ_DS_48_OHM,
	.dram_sdclk_1 = MX6DQ_INPUT_DIFFERENTIAL | MX6DQ_DS_48_OHM,
	.dram_cas = MX6DQ_INPUT_DIFFERENTIAL | MX6DQ_DS_48_OHM,
	.dram_ras = MX6DQ_INPUT_DIFFERENTIAL | MX6DQ_DS_48_OHM,
	.dram_reset = MX6DQ_INPUT_DIFFERENTIAL | MX6DQ_DS_48_OHM,
	.dram_sdba2 = MX6DQ_DS_GRP,
	.dram_sdcke0 = 0x00003000, // reserved bits
	.dram_sdcke1 = 0x00003000, // reserved bits
	.dram_sdodt0 = 0x00003000 | MX6DQ_DS_48_OHM,
	.dram_sdodt1 = 0x00003000 | MX6DQ_DS_48_OHM,
	.dram_sdqs0 = MX6DQ_DS_48_OHM,
	.dram_sdqs1 = MX6DQ_DS_48_OHM,
	.dram_sdqs2 = MX6DQ_DS_48_OHM,
	.dram_sdqs3 = MX6DQ_DS_48_OHM,
	.dram_sdqs4 = MX6DQ_DS_48_OHM,
	.dram_sdqs5 = MX6DQ_DS_48_OHM,
	.dram_sdqs6 = MX6DQ_DS_48_OHM,
	.dram_sdqs7 = MX6DQ_DS_48_OHM,
	.dram_dqm0 = MX6DQ_INPUT_DIFFERENTIAL | MX6DQ_DS_48_OHM,
	.dram_dqm1 = MX6DQ_INPUT_DIFFERENTIAL | MX6DQ_DS_48_OHM,
	.dram_dqm2 = MX6DQ_INPUT_DIFFERENTIAL | MX6DQ_DS_48_OHM,
	.dram_dqm3 = MX6DQ_INPUT_DIFFERENTIAL | MX6DQ_DS_48_OHM,
	.dram_dqm4 = MX6DQ_INPUT_DIFFERENTIAL | MX6DQ_DS_48_OHM,
	.dram_dqm5 = MX6DQ_INPUT_DIFFERENTIAL | MX6DQ_DS_48_OHM,
	.dram_dqm6 = MX6DQ_INPUT_DIFFERENTIAL | MX6DQ_DS_48_OHM,
	.dram_dqm7 = MX6DQ_INPUT_DIFFERENTIAL | MX6DQ_DS_48_OHM,
};

static const struct mx6dq_iomux_grp_regs mx6dq_grp_ioregs = {
	.grp_ddr_type = MX6DQ_DDR_TYPE_DD3,
	.grp_ddrpke = MX6DQ_DDRPKE_DISABLED,
	.grp_addds = MX6DQ_DS_48_OHM,
	.grp_ctlds = MX6DQ_DS_48_OHM,
	.grp_ddrmode_ctl = MX6DQ_INPUT_DIFFERENTIAL,
	.grp_ddrmode = MX6DQ_INPUT_DIFFERENTIAL,
	.grp_b0ds = MX6DQ_DS_48_OHM,
	.grp_b1ds = MX6DQ_DS_48_OHM,
	.grp_b2ds = MX6DQ_DS_48_OHM,
	.grp_b3ds = MX6DQ_DS_48_OHM,
	.grp_b4ds = MX6DQ_DS_48_OHM,
	.grp_b5ds = MX6DQ_DS_48_OHM,
	.grp_b6ds = MX6DQ_DS_48_OHM,
	.grp_b7ds = MX6DQ_DS_48_OHM,
};

static const struct mx6_mmdc_calibration mx6dq_calib_NT5CC256M16EP_EK = {
	.p0_mpwldectrl0 =  0x001E001D,
	.p0_mpwldectrl1 =  0x001E0019,
	.p1_mpwldectrl0 =  0x000F0024,
	.p1_mpwldectrl1 =  0x000D001F,
	.p0_mpdgctrl0 =  0x03100324,
	.p0_mpdgctrl1 =  0x030C0304,
	.p1_mpdgctrl0 =  0x03100320,
	.p1_mpdgctrl1 =  0x030C0258,
	.p0_mprddlctl =  0x4A3A4244,
	.p1_mprddlctl =  0x3E3E364C,
	.p0_mpwrdlctl =  0x3A3E4440,
	.p1_mpwrdlctl =  0x4432463A,
};

static const struct mx6_ddr3_cfg mem_ddr_NT5CC256M16EP_EK = {
	.mem_speed = 1600,
	.density = 4,
	.width = 16,
	.banks = 8,
	.rowaddr = 15,
	.coladdr = 10,
	.pagesz = 2,
	.trcd = 1375,
	.trcmin = 4875,
	.trasmin = 3500,
	/*.SRT = 1,*/
};

static const struct mx6_mmdc_calibration mx6dq_calib_V73CBG08168RFPK13 = {
	.p0_mpwldectrl0 =  0x002A0027,
	.p0_mpwldectrl1 =  0x00380030,
	.p1_mpwldectrl0 =  0x001D002E,
	.p1_mpwldectrl1 =  0x001A002E,
	.p0_mpdgctrl0 =  0x03340348,
	.p0_mpdgctrl1 =  0x03340328,
	.p1_mpdgctrl0 =  0x032C033C,
	.p1_mpdgctrl1 =  0x03280274,
	.p0_mprddlctl =  0x483C4240,
	.p1_mprddlctl =  0x403E384C,
	.p0_mpwrdlctl =  0x34343E3A,
	.p1_mpwrdlctl =  0x422C4238,
};

static const struct mx6_ddr3_cfg mem_ddr_V73CBG08168RFPK13 = {
	.mem_speed = 1600,
	.density = 8,
	.width = 16,
	.banks = 8,
	.rowaddr = 16,
	.coladdr = 10,
	.pagesz = 2,
	.trcd = 1100,
	.trcmin = 3900,
	.trasmin = 2800,
	/*.SRT = 1,*/
};

static const struct mx6_mmdc_calibration mx6dq_calib_MT41K512M16HA_107 = {
	.p0_mpwldectrl0 =  0x00260028,
	.p0_mpwldectrl1 =  0x0030002A,
	.p1_mpwldectrl0 =  0x00190029,
	.p1_mpwldectrl1 =  0x0017002B,
	.p0_mpdgctrl0 =  0x03240334,
	.p0_mpdgctrl1 =  0x03200318,
	.p1_mpdgctrl0 =  0x03280340,
	.p1_mpdgctrl1 =  0x03200268,
	.p0_mprddlctl =  0x44364040,
	.p1_mprddlctl =  0x423C384A,
	.p0_mpwrdlctl =  0x363C3C36,
	.p1_mpwrdlctl =  0x4232463E,
};

static const struct mx6_ddr3_cfg mem_ddr_MT41K512M16HA_107 = {
	.mem_speed = 1600,
	.density = 8,
	.width = 16,
	.banks = 8,
	.rowaddr = 16,
	.coladdr = 10,
	.pagesz = 2,
	.trcd = 1375,
	.trcmin = 4875,
	.trasmin = 3500,
	.SRT = 1,
};

static struct mx6_ddr_sysinfo sysinfo = {
	/* width of data bus:0=16,1=32,2=64 */
	.dsize = 2,
	/* config for full 4GB range so that get_mem_size() works */
	.cs_density = 32, /* 32Gb per CS */
	/* single chip select */
	.ncs = 1,
	.cs1_mirror = 0,
	.rtt_wr = 1 /*DDR3_RTT_60_OHM*/,	/* RTT_Wr = RZQ/4 */
#ifdef RTT_NOM_120OHM
	.rtt_nom = 2 /*DDR3_RTT_120_OHM*/,	/* RTT_Nom = RZQ/2 */
#else
	.rtt_nom = 1 /*DDR3_RTT_60_OHM*/,	/* RTT_Nom = RZQ/4 */
#endif
	.walat = 1,	/* Write additional latency */
	.ralat = 5,	/* Read additional latency */
	.mif3_mode = 3,	/* Command prediction working mode */
	.bi_on = 1,	/* Bank interleaving enabled */
	.sde_to_rst = 0x10,	/* 14 cycles, 200us (JEDEC default) */
	.rst_to_cke = 0x23,	/* 33 cycles, 500us (JEDEC default) */
	.ddr_type = DDR_TYPE_DDR3,
	.refsel = 1,	/* Refresh cycles at 32KHz */
	.refr = 7,	/* 8 refresh commands per refresh cycle */
};

static void ccgr_init(void)
{
	struct mxc_ccm_reg *ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	writel(0x00C03F3F, &ccm->CCGR0);
	writel(0x0030FF0F, &ccm->CCGR1);
	writel(0x0FFFC000, &ccm->CCGR2);
	writel(0x3FF00000, &ccm->CCGR3);
	writel(0x00FFF300, &ccm->CCGR4);
	writel(0x0F0000C3, &ccm->CCGR5);
	writel(0x00000339, &ccm->CCGR6);
}

static const char* hw_string[8] = {
	"A",
	"B",
	"C",
	"D",
	"E",
	"F",
	"G",
	"H",
};

#define HW_SETTING_0	IMX_GPIO_NR(4, 21)
#define HW_SETTING_1	IMX_GPIO_NR(4, 22)
#define HW_SETTING_2	IMX_GPIO_NR(4, 23)

static int get_version(void)
{
	return (((!gpio_get_value(HW_SETTING_0)) << 2) |
			((!gpio_get_value(HW_SETTING_1)) << 1) |
			(!gpio_get_value(HW_SETTING_2)) ) & 7;
}

static void spl_dram_init(void)
{
	if (get_version() == 3) { // Rev D
		printf("DDR3L: Micron MT41K512M16HA-107\n");
		mx6dq_dram_iocfg(64, &mx6dq_ddr_ioregs, &mx6dq_grp_ioregs);
		mx6_dram_cfg(&sysinfo, &mx6dq_calib_MT41K512M16HA_107, &mem_ddr_MT41K512M16HA_107);
	}
	else { // Rev A -> Before hw_rev gpio implemented
		if (is_cpu_type(MXC_CPU_MX6Q)) {
			printf("DDR3L: Promos V73CBG08168RFPK13\n");
			mx6dq_dram_iocfg(64, &mx6dq_ddr_ioregs, &mx6dq_grp_ioregs);
			mx6_dram_cfg(&sysinfo, &mx6dq_calib_V73CBG08168RFPK13, &mem_ddr_V73CBG08168RFPK13);
		}
		else {
			printf("DDR3L: Nanya NT5CC256M16EP-EK\n");
			mx6dq_dram_iocfg(64, &mx6dq_ddr_ioregs, &mx6dq_grp_ioregs);
			mx6_dram_cfg(&sysinfo, &mx6dq_calib_NT5CC256M16EP_EK, &mem_ddr_NT5CC256M16EP_EK);
		}
	}
}

static const iomux_v3_cfg_t uart5_pads[] = {
	IOMUX_PADS(PAD_CSI0_DAT14__UART5_TX_DATA	| MUX_PAD_CTRL(UART_PAD_CTRL)),
	IOMUX_PADS(PAD_CSI0_DAT15__UART5_RX_DATA	| MUX_PAD_CTRL(UART_PAD_CTRL)),
};

static const iomux_v3_cfg_t spi_nor_pads[] = {
	IOMUX_PADS(PAD_CSI0_DAT8__ECSPI2_SCLK | MUX_PAD_CTRL(SPI_PAD_CTRL)),
	IOMUX_PADS(PAD_CSI0_DAT9__ECSPI2_MOSI | MUX_PAD_CTRL(SPI_PAD_CTRL)),
	IOMUX_PADS(PAD_CSI0_DAT10__ECSPI2_MISO | MUX_PAD_CTRL(SPI_PAD_CTRL)),
	IOMUX_PADS(PAD_CSI0_DAT11__GPIO5_IO29 | MUX_PAD_CTRL(SPI_PAD_CTRL)), // cs
};


static const iomux_v3_cfg_t hw_rev_pads [] = {
	IOMUX_PADS(PAD_DISP0_DAT0__GPIO4_IO21 | MUX_PAD_CTRL(WEAK_PULLUP)), // HW_Setting_0
	IOMUX_PADS(PAD_DISP0_DAT1__GPIO4_IO22 | MUX_PAD_CTRL(WEAK_PULLUP)), // HW_Setting_1
	IOMUX_PADS(PAD_DISP0_DAT2__GPIO4_IO23 | MUX_PAD_CTRL(WEAK_PULLUP)), // HW_Setting_2
};

int board_early_init_f(void)
{
	SETUP_IOMUX_PADS(spi_nor_pads);
	SETUP_IOMUX_PADS(uart5_pads);
	SETUP_IOMUX_PADS(hw_rev_pads);

	return 0;
}

void board_init_f(ulong dummy)
{
	arch_cpu_init();
	ccgr_init();
	gpr_init();
/*
#ifdef CONFIG_SPL_WATCHDOG_SUPPORT
	void hw_watchdog_init(void);
	hw_watchdog_init();
#endif
*/
	board_early_init_f();

	// setup GP timer
	timer_init();

	// Setup console
	preloader_console_init();

	printf("version: %s\n", hw_string[get_version()]);

	// Set DRAM config
	spl_dram_init();
}
