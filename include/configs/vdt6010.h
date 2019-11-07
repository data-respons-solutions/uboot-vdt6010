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
#define CONFIG_ENV_SIZE			0x10000
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
#ifndef CONFIG_LOGLEVEL
#define CONFIG_LOGLEVEL 4
#endif
#define CONSOLE_DEV		"ttymxc4"

/* boot configuration */
#define DEFAULT_USB_DEV "0"
#define DEFAULT_USB_PART "1"
#define DEFAULT_USB_ROOT "/dev/sda1"
#define DEFAULT_MMC_DEV "3"
#define DEFAULT_MMC_PART "1"
#define DEFAULT_MMC_ROOT "/dev/mmcblk3p1"
#define DEFAULT_ZIMAGE "/boot/zImage"
#define DEFAULT_ZIMAGE_SECURE "/boot/zImage-ivt_signed"
#define DEFAULT_INITRD "/boot/initrd"
#define DEFAULT_INITRD_SECURE "/boot/initrd-ivt_signed"
#define DEFAULT_FDT "/boot/datarespons-vdt6010-revA.dtb"

#define BOOTSCRIPT_NOSECURE \
	"run setargs; run loadfdt;" \
	"if run loadimage; then " \
		"bootz ${loadaddr} - ${fdt_addr};" \
	"else " \
		"echo ERROR: Could not load prescribed config;" \
	"fi;"

#define BOOTSCRIPT_SECURE \
	"run setargs; " \
	"if run load_ivt_info; then " \
		"echo IVT starts at ${ivt_offset}; " \
		"if run loadimage; then " \
			"if hab_auth_img ${loadaddr} ${filesize} ${ivt_offset}; then " \
				"echo Authenticated kernel; " \
				"run loadfdt; bootz ${loadaddr} - ${fdt_addr}; " \
			"else " \
				"echo Failed to authenticate kernel; " \
			"fi; " \
		"else " \
			"echo Failed to load image ${zimage}; " \
		"fi; " \
	"else " \
		"echo No IVT information; " \
	"fi;"

#define VALIDATE_ZIMAGE \
	"if run load_ivt_info; then " \
		"echo kernel IVT starts at ${ivt_offset}; " \
		"if run loadimage; then " \
			"if hab_auth_img ${loadaddr} ${filesize} ${ivt_offset}; then " \
				"echo Authenticated kernel; " \
			"else " \
				"echo Failed to authenticate kernel && false;" \
			"fi; " \
		"else " \
			"echo Failed to load image ${zimage} && false; " \
		"fi; " \
	"else " \
		"echo No IVT information && false; " \
	"fi;"

#define VALIDATE_INITRD \
	"if run load_initrd_ivt_info; then " \
		"echo INITRD IVT loaded at ${initrd_addr} offset is ${ivt_offset}; " \
		"if run loadinitrd; then " \
			"if hab_auth_img ${initrd_addr} ${filesize} ${ivt_offset}; then " \
				"echo Authenticated initrd; " \
			"else " \
				"echo Failed to authenticate initrd && false; " \
			"fi; " \
		"else " \
			"echo Failed to load image ${initrd_file} && false; " \
		"fi; " \
	"else " \
		"echo No IVT information && false; " \
	"fi;"

#define CONFIG_EXTRA_ENV_SETTINGS \
	"bootretry="xstr(CONFIG_BOOT_RETRY_TIME)"\0" \
	"zimage="DEFAULT_ZIMAGE"\0" \
	"initrd_file="DEFAULT_INITRD"\0" \
	"fdt_file="DEFAULT_FDT"\0" \
	"fdt_addr=0x11000000\0" \
	"ip_dyn=try\0" \
	"console=" CONSOLE_DEV ",115200\0" \
	"fdt_high=0xffffffff\0"	  \
	"loglevel="xstr(CONFIG_LOGLEVEL)"\0" \
	"consoleblank=0\0" \
	"showtty=console=tty1\0" \
	"setargs=setenv bootargs console=${console} root=${rootdev} rootwait ro rootfstype=ext4 consoleblank=${consoleblank} loglevel=${loglevel} ${showtty}\0" \
	"loadbootscript=if ext4load ${bootfrom} ${bootdev}:${bootpart} ${loadaddr} /boot/boot.txt; then env import -t ${loadaddr} ${filesize}; fi; \0" \
	"ivt_offset=0\0" \
	"load_ivt_info=if ext4load ${bootfrom} ${bootdev}:${bootpart} 11F00000 /boot/zImage-padded-size; then env import -t 11F00000 ${filesize}; fi; \0" \
	"load_initrd_ivt_info=if ext4load ${bootfrom} ${bootdev}:${bootpart} 11F00000 /boot/initrd-padded-size; then env import -t 11F00000 ${filesize}; fi; \0" \
	"setmmc=setenv bootfrom mmc; setenv bootdev "DEFAULT_MMC_DEV"; setenv bootpart "DEFAULT_MMC_PART"; setenv rootdev "DEFAULT_MMC_ROOT"; \0 " \
	"setusb=setenv bootfrom usb; setenv bootdev "DEFAULT_USB_DEV"; setenv bootpart "DEFAULT_USB_PART"; setenv rootdev "DEFAULT_USB_ROOT"; \0 " \
	"loadimage=ext4load ${bootfrom} ${bootdev}:${bootpart} ${loadaddr} ${zimage}; \0" \
	"loadinitrd=ext4load ${bootfrom} ${bootdev}:${bootpart} ${initrd_addr} ${initrd_file}; \0" \
	"loadfdt=ext4load ${bootfrom} ${bootdev}:${bootpart} ${fdt_addr} ${fdt_file}; \0" \
	"bootscript_secure=" BOOTSCRIPT_SECURE " \0" \
	"bootscript_nosecure=" BOOTSCRIPT_NOSECURE " \0" \
	"bootscript="BOOTSCRIPT_NOSECURE" \0" \
	"validate_image=" VALIDATE_ZIMAGE " \0" \
	"validate_initrd=" VALIDATE_INITRD " \0" \
	"initrd_addr=0x12C00000\0" \
	"initrd_high=0xffffffff\0" \
	"factory_args=setenv bootargs console=${console} rdinit=/linuxrc enable_wait_mode=off \0" \
	"habtest=run load_ivt_info loadimage; hab_auth_img ${loadaddr} ${filesize} ${ivt_offset}; \0" \
	"install_boot=run install_args loadfdt loadimage loadinitrd; bootz ${loadaddr} ${initrd_addr} ${fdt_addr}; \0"

#define BOOT_PRELOADED \
	"echo trying preloaded boot...;" \
	"run factory_args;" \
	"bootz ${loadaddr} ${initrd_addr} ${fdt_addr};" \
	"echo    no preloaded image;"

#define BOOT_USB \
	"echo trying usb boot...;" \
	"if usb storage; then " \
		"run setusb;" \
		"run loadbootscript;" \
		"run bootscript;" \
		"echo USB boot failed;" \
	"else " \
		"echo no usb device available;" \
	"fi;"

#define BOOT_MMC \
	"echo trying mmc boot...;" \
	"if mmc dev "DEFAULT_MMC_DEV"; then " \
		"run setmmc;" \
		"run loadbootscript;" \
		"run bootscript;" \
		"echo mmc boot failed;" \
	"else " \
		"echo no mmc device available;" \
	"fi;"

#define CONFIG_BOOTCOMMAND \
	"echo starting boot procedure...;" \
	BOOT_PRELOADED  \
	BOOT_USB \
	BOOT_MMC \
	"echo no boot device found;"

/*
	"if test ${bootpart} = 2; then " \
		"setenv bootpart 1; setenv mmc_root /dev/mmcblk0p1;" \
	"else " \
		"setenv bootpart 2; setenv mmc_root /dev/mmcblk0p2;" \
	"fi; " \
*/


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
#define CONFIG_VIDEO_IPUV3
#define CONFIG_VIDEO_BMP_RLE8
#define CONFIG_SPLASH_SCREEN
#define CONFIG_SPLASH_SCREEN_ALIGN
#define CONFIG_BMP_24BPP
#define CONFIG_VIDEO_LOGO
#define CONFIG_VIDEO_BMP_LOGO
#define CONFIG_IMX_HDMI
#define CONFIG_IMX_VIDEO_SKIP

/* USB */
#if defined(CONFIG_USB)
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET
#define CONFIG_MXC_USB_PORTSC		(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_USB_MAX_CONTROLLER_COUNT	2 /* Enabled USB controller number */
#endif

#endif /* __VDT6010_CONFIG_H */
