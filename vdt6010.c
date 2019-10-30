// #define CONFIG_PANIC_HANG
// CONFIG_SPL_NOR_SUPPORT
// SPL_SPI_FLASH_SUPPORT
// SPL_SPI_LOAD
// CONFIG_SPL_POWER_SUPPORT
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
#include <asm/mach-imx/mxc_i2c.h>
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
#include <i2c.h>
#include <input.h>
#include <usb.h>
#include <usb/ehci-ci.h>
#include <environment.h>
#include <usb.h>
#include <pwm.h>
#include <version.h>
#include <watchdog.h>

#include <asm/mach-imx/hab.h>
#include <vsprintf.h>

DECLARE_GLOBAL_DATA_PTR;

#include "../common/mx6_common_defs.h"
#include "../common/dev/pfuze100.h"
#include "vdt6010_pins.h"
#include "vdt6010_gpio.h"

#define BACKLIGHT_FREQ_HZ 1000
#define BACKLIGHT_PERIOD_NS 1000000000 / BACKLIGHT_FREQ_HZ

/*
 *
 * board_early_init_f
 *
 */

static void setup_backlight()
{
	SETUP_IOMUX_PADS(pwm_pads);

	gpio_set_value(LED_VCC_EN, 1);
	gpio_set_value(LED_EN, 1);

	if (pwm_config(0, BACKLIGHT_PERIOD_NS, BACKLIGHT_PERIOD_NS)) {
		printf("%s: pwm_config failed\n", __func__);
	}

	pwm_enable(0);
}

static void disable_lvds(struct display_info_t const *dev)
{
	struct iomuxc *iomux = (struct iomuxc *)IOMUXC_BASE_ADDR;

	int reg = readl(&iomux->gpr[2]);

	reg &= ~(IOMUXC_GPR2_LVDS_CH0_MODE_MASK |
		 IOMUXC_GPR2_LVDS_CH1_MODE_MASK);

	writel(reg, &iomux->gpr[2]);
}

static void do_enable_hdmi(struct display_info_t const *dev)
{
	disable_lvds(dev);
	imx_enable_hdmi_phy();
}

static void enable_lvds(struct display_info_t const *dev)
{
	gpio_set_value(LCD_VDD_EN, 1);
	setup_backlight();
}


struct display_info_t const displays[] = {{
	.bus	= -1,
	.addr	= 0,
	.pixfmt	= IPU_PIX_FMT_RGB24,
	.detect	= NULL,
	.enable	= enable_lvds,
	.mode	= {
			.name           = "AM-1280800NTZQW-T10H",
			.refresh        = 60,
			.xres           = 1280,
			.yres           = 800,
			.pixclock       = 1000000 / 71100000,
			.left_margin    = 50,
			.right_margin   = 50,
			.upper_margin   = 10,
			.lower_margin   = 10,
			.hsync_len      = 60,
			.vsync_len      = 3,
			.sync           = FB_SYNC_EXT,
			.vmode          = FB_VMODE_NONINTERLACED
} }, {
	.bus	= -1,
	.addr	= 0,
	.pixfmt	= IPU_PIX_FMT_RGB24,
	.detect	= detect_hdmi,
	.enable	= do_enable_hdmi,
	.mode	= {
			.name           = "HDMI",
			.refresh        = 60,
			.xres           = 1280,
			.yres           = 720,
			.pixclock       = 15686,
			.left_margin    = 48,
			.right_margin   = 32,
			.upper_margin   = 3,
			.lower_margin   = 5,
			.hsync_len      = 80,
			.vsync_len      = 13,
			.sync           = FB_SYNC_EXT,
			.vmode          = FB_VMODE_NONINTERLACED
} },  };
size_t display_count = ARRAY_SIZE(displays);

static void setup_display(void)
{
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	struct iomuxc *iomux = (struct iomuxc *)IOMUXC_BASE_ADDR;
	int reg;

	/* Setup HSYNC, VSYNC, DISP_CLK for debugging purposes */
	//SETUP_IOMUX_PADS(di0_pads);

	enable_ipu_clock();
	imx_setup_hdmi();

	/* Turn on LDB0, LDB1, IPU,IPU DI0 clocks */
	reg = readl(&mxc_ccm->CCGR3);
	reg |=  MXC_CCM_CCGR3_LDB_DI0_MASK | MXC_CCM_CCGR3_LDB_DI1_MASK;
	writel(reg, &mxc_ccm->CCGR3);

	/* set LDB0, LDB1 clk select to 011/011 */
	reg = readl(&mxc_ccm->cs2cdr);
	reg &= ~(MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_MASK
		 | MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_MASK);
	reg |= (3 << MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_OFFSET)
	      | (3 << MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_OFFSET);
	writel(reg, &mxc_ccm->cs2cdr);

	reg = readl(&mxc_ccm->cscmr2);
	reg |= MXC_CCM_CSCMR2_LDB_DI0_IPU_DIV | MXC_CCM_CSCMR2_LDB_DI1_IPU_DIV;
	writel(reg, &mxc_ccm->cscmr2);

	reg = readl(&mxc_ccm->chsccdr);
	reg |= (CHSCCDR_CLK_SEL_LDB_DI0
		<< MXC_CCM_CHSCCDR_IPU1_DI0_CLK_SEL_OFFSET);
	reg |= (CHSCCDR_CLK_SEL_LDB_DI0
		<< MXC_CCM_CHSCCDR_IPU1_DI1_CLK_SEL_OFFSET);
	writel(reg, &mxc_ccm->chsccdr);

	reg = IOMUXC_GPR2_BGREF_RRMODE_EXTERNAL_RES
	     | IOMUXC_GPR2_DI1_VS_POLARITY_ACTIVE_LOW
	     | IOMUXC_GPR2_DI0_VS_POLARITY_ACTIVE_LOW
	     | IOMUXC_GPR2_BIT_MAPPING_CH1_SPWG
	     | IOMUXC_GPR2_DATA_WIDTH_CH1_24BIT
	     | IOMUXC_GPR2_BIT_MAPPING_CH0_SPWG
	     | IOMUXC_GPR2_DATA_WIDTH_CH0_24BIT
	     | IOMUXC_GPR2_LVDS_CH0_MODE_DISABLED
	     | IOMUXC_GPR2_LVDS_CH1_MODE_ENABLED_DI0;
	writel(reg, &iomux->gpr[2]);

	reg = readl(&iomux->gpr[3]);
	reg = (reg & ~(IOMUXC_GPR3_LVDS1_MUX_CTL_MASK
			| IOMUXC_GPR3_HDMI_MUX_CTL_MASK))
	    | (IOMUXC_GPR3_MUX_SRC_IPU1_DI0
	       << IOMUXC_GPR3_LVDS1_MUX_CTL_OFFSET);
	writel(reg, &iomux->gpr[3]);
}

int overwrite_console(void)
{
	return 1;
}

int board_early_init_f(void)
{
	SETUP_IOMUX_PADS(uart1_pads);
	SETUP_IOMUX_PADS(gpio_pads);
	SETUP_IOMUX_PADS(other_pads);

	gpio_direction_output(LED_VCC_EN, 0);
	gpio_direction_output(LED_EN, 0);
	gpio_direction_output(LCD_VDD_EN, 0);

	setup_i2c(0, CONFIG_SYS_I2C_SPEED, 0x7f, &mx6dq_i2c_pad_info0);
	setup_i2c(1, CONFIG_SYS_I2C_SPEED, 0x7f, &mx6dq_i2c_pad_info1);
	setup_i2c(2, CONFIG_SYS_I2C_SPEED, 0x7f, &mx6dq_i2c_pad_info2);

	setup_display();

	return 0;
}

/*
 *
 * checkboard
 *
 */

static const char* BOARD = "VDT6010";
static const char* VENDOR = "Data Respons Solutions AB";

static const char* hw_string[8] = {
	"REVA",
	"REVB",
	"REVC",
	"REVD",
	"REVE",
	"FUTURE",
	"FUTURE",
	"FUTURE",
};

static int get_version(void)
{
	return 0;
}

int checkboard(void)
{
	printf("Vendor: %s\n", VENDOR);
	printf("Board: %s\n", BOARD);
	printf("HW version: %s\n", hw_string[get_version()]);

	return 0;
}

/*
 *
 * dram_init
 *
 */

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();
	return 0;
}

/*
 *
 * board_init
 *
 * Setup chipselects
 *
 */


int board_init(void)
{
	// address of boot parameters
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	return 0;
}

/*
 *
 * power_init_board
 *
 */

struct _pmic_setup {
	pf100_regs	reg;
	int			mV;
} pmic_setup[] = {
		{SW1AB, 1425},	// VDDCORE
		{SW1C, 1425},	// VDDSOC
		{SW3AB, 1350},	// DDR_1V5
		{0, 0}
};

int power_init_board(void)
{
	const int i2c = POWER_PFUZE100_I2C;
	const int addr = CONFIG_POWER_PFUZE100_I2C_ADDR;
	int ret = 0;

	ret = pfuze100_setup(i2c, addr);
	if (ret) {
		printf("%s: no pmic: i2c: %d: addr: %d\n", __func__, i2c, addr);
		return ret;
	}

	for (int i = 0; pmic_setup[i].reg; ++i) {
		printf("Setting PMIC register %s to %d mV\n", pf100_reg_str(pmic_setup[i].reg), pmic_setup[i].mV);
		ret = pfuze100_set(i2c, addr, pmic_setup[i].reg, pmic_setup[i].mV);
		if (ret) {
			printf("%s: pfuze100_set: %d\n", __func__, ret);
			return ret;
		}
	}

	udelay(10000);

	return 0;
}


/*
 *
 * mmc
 *
 */


struct fsl_esdhc_cfg usdhc_cfg[2] = {
	{USDHC2_BASE_ADDR},
	{USDHC4_BASE_ADDR},
};

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
	int ret;

	SETUP_IOMUX_PADS(usdhc2_pads);
	gpio_direction_input(USDHC2_CD);
	usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);
	ret = fsl_esdhc_initialize(bis, &usdhc_cfg[0]);
	if (ret) {
		return ret;
	}

	SETUP_IOMUX_PADS(usdhc4_pads);
	usdhc_cfg[1].sdhc_clk = mxc_get_clock(MXC_ESDHC4_CLK);
	ret = fsl_esdhc_initialize(bis, &usdhc_cfg[1]);
	if (ret) {
		return ret;
	}

	return 0;
}

/*
 *
 * board_late_init
 *
 */

#define USBNC_USB_OTG_CTRL 		(USB_BASE_ADDR + 0x800)
#define USBNC_CTRL_OVER_CUR_POL	(1 << 8) // Polarity of OC pin
#define USBNC_CTRL_PWR_POL		(1 << 9) // Polarity of pwr pin

//FIXME:
int board_ehci_hcd_init(int port)
{
	u32 *usbnc_usb_ctrl;

	if (port > 1) {
		return -EINVAL;
	}

	usbnc_usb_ctrl = (u32 *)(USBNC_USB_OTG_CTRL + port * 4);
	// FIXME: POWER ANC OC POLARITY?
	setbits_le32(usbnc_usb_ctrl, USBNC_CTRL_PWR_POL | USBNC_CTRL_OVER_CUR_POL);

	return 0;
}
/*
int board_usb_phy_mode(int port)
{
	unsigned int bmode = readl(&src_base->sbmr2);

	switch(port) {
	case 0:
		if (((bmode >> 24) & 0x03) == 0x01)	{
			printf("USB OTG in serial download mode\n");
			return USB_INIT_DEVICE;
		}

		return USB_INIT_HOST;

	case 1:
		return USB_INIT_HOST;

	default:
		return USB_INIT_DEVICE;
	}
}
*/

static void setup_usb(void)
{
	SETUP_IOMUX_PADS(usb_otg_pads);
	SETUP_IOMUX_PADS(usb_host_pads);

	// Set daisy chain - USB_OTG_ID to GPIO_1
	if (is_mx6dq()) {
		imx_iomux_set_gpr_register(1, IOMUXC_GPR1_USB_OTG_ID_OFFSET, 1, 1);
	}

	// Reset USB hub and devices
	gpio_direction_output(USB_HUB_RST, 0);
	gpio_direction_output(USB_PWR_EN1, 0);
	gpio_direction_output(USB_PWR_EN2, 0);
	mdelay(10);
	gpio_set_value(USB_HUB_RST, 1);
	gpio_set_value(USB_PWR_EN1, 1);
	gpio_set_value(USB_PWR_EN2, 1);
}

/**
 * board_ehci_power - enables/disables usb vbus voltage
 * @port:      usb otg port
 * @on:        on/off vbus voltage
 *
 * Enables/disables supply vbus voltage for usb otg port.
 * Machine board file overrides board_ehci_power
 *
 * Return: 0 Success
 */
int board_ehci_power(int port, int on)
{
	switch (port) {
	case 0:
		break;
	case 1:
		if (on)
			gpio_direction_output(IMX_GPIO_NR(1, 29), 1);
		else
			gpio_direction_output(IMX_GPIO_NR(1, 29), 0);
		break;
	default:
		printf("MXC USB port %d not yet supported\n", port);
		return -EINVAL;
	}

	return 0;
}

static char * const usbcmd[] = {"usb", "start"};

int board_late_init(void)
{
	int rep;
	ulong ticks;

	//FIXME: Move setup_usb() to
	setup_usb();

	cmd_process(0, 2, usbcmd, &rep, &ticks);

	env_set("fdt_file", "/boot/datarespons-vdt6010-revA.dtb");

	if (imx_hab_is_enabled())
	{
		printf("HAB enabled, setting up secure bootscript\n");
		env_set("bootscript", BOOTSCRIPT_SECURE);
		env_set("zimage", ZIMAGE_SECURE);
		env_set("initrd_file", "/boot/initrd-ivt_signed");
#ifdef CONFIG_BOOT_USB_INITRD
		env_set("bootscript_usb", BOOTSCRIPT_USB_SECURE);
#else
		env_set("bootscript_usb", BOOTSCRIPT_SECURE);
#endif
		//env_set("bootdelay", "0");
	}
	else
	{
		printf("HAB disabled, setting up regular bootscript\n");
		env_set("bootscript", BOOTSCRIPT_NOSECURE);
		env_set("bootscript_usb", BOOTSCRIPT_NOSECURE);
	}

	return 0;
}

/*
 *
 * enet
 *
 */

int board_phy_config(struct phy_device *phydev)
{
	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

static void setup_iomux_enet(void)
{
	int ret = 0;

	SETUP_IOMUX_PADS(enet_pads);

	/* Set LAN8710 PHY reset */
	gpio_direction_output(LAN8710_RST , 0);

	/*
	 * Set GPR1[21] to configure ENET_REF_CLK as output.
	 * ENET_REF_CLOCK from anatop->enet_pll
	 * Expects GPR1[13] already cleared (setup_usb) to select ENET_RX_ER
	 */
	imx_iomux_set_gpr_register(1, IOMUXC_GPR1_ENET_CLK_SEL_OFFSET, 1, 1);

	/* Set anatop->enet_pll as 50MHz */
	ret = enable_fec_anatop_clock(0, ENET_50MHZ);
	if (ret) {
		printf("%s: enable_fec_anatop_clock: %d\n", __func__, ret);
	}

	mdelay(1);

	/* Release LAN8710 PHY reset */
	gpio_set_value(LAN8710_RST , 1);
}

int board_eth_init(bd_t *bis)
{
	setup_iomux_enet();
	return cpu_eth_init(bis);
}
