/*
 * Copyright (C) 2012 Freescale Semiconductor, Inc.
 *
 * Configuration settings for the Freescale i.MX6Q SabreSD board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __VDT6010_CONFIG_H
#define __VDT6010_CONFIG_H

#define xstr(a) str(a)
#define str(a) #a

/* Debug for SPL */
#if 0
#ifdef CONFIG_SPL_BUILD
#define DEBUG
#endif
#endif

/* Debug for UBOOT */
#if 0
#ifndef CONFIG_SPL_BUILD
#define DEBUG
#define CONFIG_BOOT_RETRY_TIME -1 /* disable shell timeout */
#endif
#endif

#if defined(CONFIG_SPL)
#include "imx6_spl.h"
#endif

#if defined(CONFIG_SPL_BUILD)
/* Fix SPL build issues with SPI DM enabled as of 2019.01
 *   -- Keep until SPL migrates to FDT */
#undef CONFIG_DM_SPI
#undef CONFIG_DM_SPI_FLASH
#undef CONFIG_SPI_FLASH_MTD

/* offset of u-boot binary on SPI NOR */
#define CONFIG_SYS_SPI_U_BOOT_OFFS	0x30000

/* Define console UART until migrate to DM */
#define CONFIG_MXC_UART_BASE	UART5_BASE
#endif

/* Secure boot */
#define CONFIG_SYS_FSL_SEC_COMPAT 4

#include "mx6_common.h"

#if defined(CONFIG_SECURE_BOOT) && defined(CONFIG_SPL)
#define CONFIG_RESET_TO_RETRY
#ifndef CONFIG_BOOT_RETRY_TIME
#define CONFIG_BOOT_RETRY_TIME 100
#endif
#endif

/* environment */
#define CONFIG_LOADADDR			0x12000000
#define CONFIG_SYS_LOAD_ADDR	CONFIG_LOADADDR
#define CONFIG_SYS_MALLOC_LEN	(40 * SZ_1M)

/* uboot shell */
#define CONFIG_SYS_PROMPT_HUSH_PS2     "> "
#define CONFIG_SYS_BARGSIZE CONFIG_SYS_CBSIZE

/* linux parameters */
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_REVISION_TAG
#define CONFIG_MACH_TYPE	0xffffffff	/* Needed for newer kernels */

/* boot configuration */
#define DEFAULT_USB_DEV "0"
#define DEFAULT_USB_PART "1"
#define DEFAULT_MMC_DEV "3"
#define DEFAULT_MMC_PART "1"
#define DEFAULT_ZIMAGE "/boot/zImage"
#define DEFAULT_ZIMAGE_SECURE "/boot/zImage-ivt_signed"
#define DEFAULT_INITRD "/boot/initrd"
#define DEFAULT_INITRD_SECURE "/boot/initrd-ivt_signed"
#define DEFAULT_INITRD_ADDR "0x12C00000"
#define DEFAULT_FDT "/boot/vdt6010-q-revA.dtb"
#define DEFAULT_FDT_ADDR "0x11000000"
#define DEFAULT_LOGLEVEL "4"
#define DEFAULT_CONSOLEBLANK "0"

#include "datarespons.h"

#define CONFIG_EXTRA_ENV_SETTINGS \
	"bootretry="xstr(CONFIG_BOOT_RETRY_TIME)"\0" \
	DATARESPONS_BOOT_SCRIPTS

#define CONFIG_BOOTCOMMAND \
	"echo starting boot procedure...;" \
	"run bootpreloaded;" \
	"run bootusb;" \
	"run bootmmc;" \
	"echo no boot device found;"

/* Physical Memory Map */
#define CONFIG_SYS_SDRAM_BASE          MMDC0_ARB_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_ADDR       IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE       IRAM_SIZE
#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* pwm configs */
#define CONFIG_PWM_IMX
#define CONFIG_IMX6_PWM_PER_CLK	66000000

/* PCIe configs */
/*
#define CONFIG_PCI_SCAN_SHOW
#define CONFIG_PCIE_IMX_PERST_GPIO	IMX_GPIO_NR(4, 8)
#define CONFIG_PCIE_IMX
*/

/* I2C Configs */
#define CONFIG_SYS_I2C_SPEED	100000
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_MXC_I2C1
#define CONFIG_SYS_I2C_MXC_I2C2
#define CONFIG_SYS_I2C_MXC_I2C3

/* ethernet */
#define CONFIG_MII
#define CONFIG_FEC_XCV_TYPE		RMII
#define CONFIG_ETHPRIME			"FEC"

/* Framebuffer */
#define CONFIG_VIDEO_BMP_GZIP
#define CONFIG_SYS_VIDEO_LOGO_MAX_SIZE (4 * SZ_1M)
#define CONFIG_VIDEO_IPUV3
#define CONFIG_VIDEO_BMP_RLE8
#define CONFIG_BMP_24BPP
#define CONFIG_IMX_HDMI
#define CONFIG_IMX_VIDEO_SKIP

/* USB */
#if defined(CONFIG_USB)
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET
#define CONFIG_MXC_USB_PORTSC		(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_USB_MAX_CONTROLLER_COUNT	2 /* Enabled USB controller number */
#endif

#endif /* __VDT6010_CONFIG_H */
