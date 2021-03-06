/*
 * Copyright (C) 2019 Data Respons Solutions AB
 *
 * Author: Mikko Salomäki <ms@datarespons.se>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <errno.h>
#include <version.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/mxc_hdmi.h>
#include <asm/arch/crm_regs.h>
#include <asm/mach-imx/video.h>
#include <asm/arch/mx6-pins.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/sys_proto.h>
#include <dm.h>
#include <pwm.h>
#include <power/pmic.h>
#include <power/pfuze100_pmic.h>
#include <power/regulator.h>
#include "../common/include/mx6_common_defs.h"
DECLARE_GLOBAL_DATA_PTR;

#define BACKLIGHT_FREQ_HZ 1000
#define BACKLIGHT_PERIOD_NS 1000000000 / BACKLIGHT_FREQ_HZ

/*
 *
 * board_early_init_f
 *
 */

static int enable_regulator(const char* name)
{
	struct udevice* reg = NULL;

	int r = regulator_get_by_platname(name, &reg);
	if (r) {
		printf("%s: failed finding: %s [%d]: %s\n", __func__, name, r, errno_str(r));
		return r;
	}

	r = regulator_set_enable(reg, 1);
	if (r) {
		printf("%s: failed enabling: %s [%d]: %s\n", __func__, name, r, errno_str(r));
		return r;
	}

	return 0;
}

static int request_gpio(struct gpio_desc* desc, const char* name, const char* label)
{
	int r = dm_gpio_lookup_name(name, desc);
	if (r) {
		printf("%s: failed finding: %s [%d]: %s\n", __func__, name, r, errno_str(r));
		return r;
	}

	r = dm_gpio_request(desc, label);
	if (r) {
		printf("%s: failed requesting: %s [%d]: %s\n", __func__, name, r, errno_str(r));
		return r;
	}

	return 0;
}

static int set_gpio(struct gpio_desc* desc, const char* name, int value)
{
	int r = dm_gpio_set_dir_flags(desc, GPIOD_IS_OUT);
	if (r) {
		printf("%s: failed setting output: %s [%d]: %s\n", __func__, name, r, errno_str(r));
		return r;
	}

	r = dm_gpio_set_value(desc, value);
	if (r) {
		printf("%s: failed setting value: %s->%d [%d]: %s\n", __func__, name, value, r, errno_str(r));
		return r;
	}

	return 0;
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

static const iomux_v3_cfg_t pwm_pads[] = {
	IOMUX_PADS(PAD_SD1_DAT3__PWM1_OUT | MUX_PAD_CTRL(OUT_LOW_PAD_CTRL)),
};

static void enable_lvds(struct display_info_t const *dev)
{
	SETUP_IOMUX_PADS(pwm_pads);

	enable_regulator("lcd_backlight");
	enable_regulator("lcd_vdd");

	struct gpio_desc led_en;
	request_gpio(&led_en, "GPIO7_12", "LED_EN");
	set_gpio(&led_en, "GPIO7_12", 1);

	if (pwm_config(0, BACKLIGHT_PERIOD_NS, BACKLIGHT_PERIOD_NS)) {
		printf("%s: pwm_config failed\n", __func__);
	}

	pwm_enable(0);
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

int board_early_init_f(void)
{
	setup_display();

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
 */
static void setup_usb(void)
{
	// Set daisy chain - USB_OTG_ID to GPIO_1
	if (is_mx6dq()) {
		imx_iomux_set_gpr_register(1, IOMUXC_GPR1_USB_OTG_ID_OFFSET, 1, 1);
	}
}

static void setup_enet(void)
{
	/*
	 * Set GPR1[21] to configure ENET_REF_CLK as output.
	 * ENET_REF_CLOCK from anatop->enet_pll
	 * Expects GPR1[13] already cleared (setup_usb()) to select ENET_RX_ER
	 */
	imx_iomux_set_gpr_register(1, IOMUXC_GPR1_ENET_CLK_SEL_OFFSET, 1, 1);

	/* Set anatop->enet_pll as 50MHz */
	int r = enable_fec_anatop_clock(0, ENET_50MHZ);
	if (r) {
		printf("%s: enable_fec_anatop_clock: %d\n", __func__, r);
	}
}

int board_init(void)
{
	// address of boot parameters
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	setup_usb();
	setup_enet();

	return 0;
}

/*
 *
 * power_init_board
 *
 */

int power_init_board(void)
{
	struct udevice *dev = NULL;
	int r = 0;
	int dev_id = 0;
	int rev_id = 0;

	r = pmic_get("pfuze100@08", &dev);
	if (r) {
		printf("pmic: pfuze100: not found [%d]: %s\n", r, errno_str(r));
		return r;
	}

	dev_id = pmic_reg_read(dev, PFUZE100_DEVICEID);
	if (dev_id < 0) {
		printf("pmic: pfuze100: dev_id [%d]: %s\n", dev_id, errno_str(dev_id));
		return dev_id;
	}
	rev_id = pmic_reg_read(dev, PFUZE100_REVID);
	if (dev_id < 0) {
		printf("pmic: pfuze100: rev_id [%d]: %s\n", rev_id, errno_str(rev_id));
		return rev_id;
	}
	printf("pmic: pfuze100: dev_id: 0x%04x: rev_id: 0x%04x\n", dev_id, rev_id);

	/* VDDCORE to 1425 mV */
	r = pmic_reg_write(dev, PFUZE100_SW1ABVOL, SW1x_1_425V);
	if (!r) {
		printf("pmic: pfuze100: SW1ABVOL: 1425 mV\n");
	}
	else {
		printf("pmic: pfuze100: SW1ABVOL [%d]: %s\n", r, errno_str(r));
		return r;
	}

	/* VDDSOC to 1425 mV */
	r = pmic_reg_write(dev, PFUZE100_SW1CVOL, SW1x_1_425V);
	if (!r) {
		printf("pmic: pfuze100: SW1CVOL: 1425 mV\n");
	}
	else {
		printf("pmic: pfuze100: SW1CVOL [%d]: %s\n", r, errno_str(r));
		return r;
	}

	/* DDR to 1350 mV */
	r = pmic_reg_write(dev, PFUZE100_SW3AVOL, SW1x_1_350V);
	if (!r) {
		printf("pmic: pfuze100: SW3AVOL: 1350 mV\n");
	}
	else {
		printf("pmic: pfuze100: SW3AVOL [%d]: %s\n", r, errno_str(r));
		return r;
	}
	r = pmic_reg_write(dev, PFUZE100_SW3BVOL, SW1x_1_350V);
	if (!r) {
		printf("pmic: pfuze100: SW3BVOL: 1350 mV\n");

	}
	else {
		printf("pmic: pfuze100: SW3BVOL [%d]: %s\n", r, errno_str(r));
		return r;
	}

	/* delay needed? */
	mdelay(10);

	return 0;
}
