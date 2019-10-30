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
#include <linux/errno.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/video.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <miiphy.h>
#include <netdev.h>
#include <asm/arch/mxc_hdmi.h>
#include <asm/arch/crm_regs.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <input.h>
#include <usb.h>
#include <usb/ehci-ci.h>
#include <environment.h>
#include <usb.h>
#include <pwm.h>
#include <version.h>
#include <watchdog.h>
#include <asm/arch/mx6-ddr.h>
#include <spl.h>
#include <asm/mach-imx/hab.h>
//#include <vsprintf.h>

DECLARE_GLOBAL_DATA_PTR;

#include "../common/mx6_common_defs.h"
#include "vdt6010_pins.h"
#include "vdt6010_gpio.h"


struct fsl_esdhc_cfg usdhc_cfg[1] = {
	{USDHC2_BASE_ADDR},
	{USDHC4_BASE_ADDR},
};

static int get_version(void)
{
	return 0;
}

int board_spi_cs_gpio(unsigned bus, unsigned cs)
{
	if (bus == 1 && cs == 0) {
		return SPI_NOR_CS;
	}

	return -1;
}

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	switch (cfg->esdhc_base) {
	case USDHC2_BASE_ADDR:
		ret = !gpio_get_value(USDHC2_CD);
		break;

	case USDHC4_BASE_ADDR:
		ret = 1;
		break;
	}

	return ret;
}

int board_mmc_init(bd_t *bis)
{
	struct src *psrc = (struct src *)SRC_BASE_ADDR;
	unsigned reg = readl(&psrc->sbmr1) >> 11;
	/*
	 * Upon reading BOOT_CFG register the following map is done:
	 * Bit 11 and 12 of BOOT_CFG register can determine the current
	 * mmc port
	 * 0x1                  SD1
	 * 0x2                  SD2
	 * 0x3                  SD4
	 */

	switch (reg & 0x3) {
	case 0x1:
		SETUP_IOMUX_PADS(usdhc2_pads);
		gpio_direction_input(USDHC2_CD);
		usdhc_cfg[0].esdhc_base = USDHC2_BASE_ADDR;
		usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);
		gd->arch.sdhc_clk = usdhc_cfg[0].sdhc_clk;
		break;
	case 0x3:
		SETUP_IOMUX_PADS(usdhc4_pads);
		usdhc_cfg[0].esdhc_base = USDHC4_BASE_ADDR;
		usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC4_CLK);
		gd->arch.sdhc_clk = usdhc_cfg[0].sdhc_clk;
		break;
	}

	return fsl_esdhc_initialize(bis, &usdhc_cfg[0]);
}

#define MX6DQ_DDR_TYPE_DD3 0x000C0000
#define MX6DQ_DDRPKE_DISABLED 0x00000000
#define MX6DQ_DS_48_OHM 0x00000028
#define MX6DQ_DS_40_OHM 0x00000030
#define MX6DQ_DS_GRP 0x00000000
#define MX6DQ_INPUT_DIFFERENTIAL 0x00020000

static const struct mx6dq_iomux_ddr_regs mx6dq_ddr_ioregs = {
	.dram_sdclk_0 = MX6DQ_INPUT_DIFFERENTIAL | MX6DQ_DS_40_OHM,
	.dram_sdclk_1 = MX6DQ_INPUT_DIFFERENTIAL | MX6DQ_DS_40_OHM,
	.dram_cas = MX6DQ_INPUT_DIFFERENTIAL | MX6DQ_DS_40_OHM,
	.dram_ras = MX6DQ_INPUT_DIFFERENTIAL | MX6DQ_DS_40_OHM,
	.dram_reset = MX6DQ_INPUT_DIFFERENTIAL | MX6DQ_DS_40_OHM,
	.dram_sdba2 = MX6DQ_DS_GRP,
	.dram_sdcke0 = 0x00003000, // reserved bits
	.dram_sdcke1 = 0x00003000, // reserved bits
	.dram_sdodt0 = 0x00003000 | MX6DQ_DS_40_OHM,
	.dram_sdodt1 = 0x00003000 | MX6DQ_DS_40_OHM,
	.dram_sdqs0 = MX6DQ_DS_40_OHM,
	.dram_sdqs1 = MX6DQ_DS_40_OHM,
	.dram_sdqs2 = MX6DQ_DS_40_OHM,
	.dram_sdqs3 = MX6DQ_DS_40_OHM,
	.dram_sdqs4 = MX6DQ_DS_40_OHM,
	.dram_sdqs5 = MX6DQ_DS_40_OHM,
	.dram_sdqs6 = MX6DQ_DS_40_OHM,
	.dram_sdqs7 = MX6DQ_DS_40_OHM,
	.dram_dqm0 = MX6DQ_INPUT_DIFFERENTIAL | MX6DQ_DS_40_OHM,
	.dram_dqm1 = MX6DQ_INPUT_DIFFERENTIAL | MX6DQ_DS_40_OHM,
	.dram_dqm2 = MX6DQ_INPUT_DIFFERENTIAL | MX6DQ_DS_40_OHM,
	.dram_dqm3 = MX6DQ_INPUT_DIFFERENTIAL | MX6DQ_DS_40_OHM,
	.dram_dqm4 = MX6DQ_INPUT_DIFFERENTIAL | MX6DQ_DS_40_OHM,
	.dram_dqm5 = MX6DQ_INPUT_DIFFERENTIAL | MX6DQ_DS_40_OHM,
	.dram_dqm6 = MX6DQ_INPUT_DIFFERENTIAL | MX6DQ_DS_40_OHM,
	.dram_dqm7 = MX6DQ_INPUT_DIFFERENTIAL | MX6DQ_DS_40_OHM,
};

static const struct mx6dq_iomux_grp_regs mx6dq_grp_ioregs = {
	.grp_ddr_type = MX6DQ_DDR_TYPE_DD3,
	.grp_ddrpke = MX6DQ_DDRPKE_DISABLED,
	.grp_addds = MX6DQ_DS_40_OHM,
	.grp_ctlds = MX6DQ_DS_40_OHM,
	.grp_ddrmode_ctl = MX6DQ_INPUT_DIFFERENTIAL,
	.grp_ddrmode = MX6DQ_INPUT_DIFFERENTIAL,
	.grp_b0ds = MX6DQ_DS_40_OHM,
	.grp_b1ds = MX6DQ_DS_40_OHM,
	.grp_b2ds = MX6DQ_DS_40_OHM,
	.grp_b3ds = MX6DQ_DS_40_OHM,
	.grp_b4ds = MX6DQ_DS_40_OHM,
	.grp_b5ds = MX6DQ_DS_40_OHM,
	.grp_b6ds = MX6DQ_DS_40_OHM,
	.grp_b7ds = MX6DQ_DS_40_OHM,
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

static void spl_dram_init(void)
{
	mx6dq_dram_iocfg(64, &mx6dq_ddr_ioregs, &mx6dq_grp_ioregs);
	mx6_dram_cfg(&sysinfo, &mx6dq_calib_NT5CC256M16EP_EK, &mem_ddr_NT5CC256M16EP_EK);
}

int board_early_init_f(void)
{
	SETUP_IOMUX_PADS(spi_nor_pads);
	SETUP_IOMUX_PADS(uart5_pads);

	return 0;
}

void board_init_f(ulong dummy)
{
	arch_cpu_init();
	ccgr_init();
	gpr_init();

#ifdef CONFIG_SPL_WATCHDOG_SUPPORT
	void hw_watchdog_init(void);
	hw_watchdog_init();
#endif

	board_early_init_f();

	// setup GP timer
	timer_init();

	// Setup console
	preloader_console_init();

	// Set DRAM config
	spl_dram_init();
}
