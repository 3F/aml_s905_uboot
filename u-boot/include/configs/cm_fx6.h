/*
 * Config file for Compulab CM-FX6 board
 *
 * Copyright (C) 2014, Compulab Ltd - http://compulab.co.il/
 *
 * Author: Nikita Kiryanov <nikita@compulab.co.il>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_CM_FX6_H
#define __CONFIG_CM_FX6_H

#include <asm/arch/imx-regs.h>
#include <config_distro_defaults.h>
#include "mx6_common.h"

/* Machine config */
#define CONFIG_MX6
#define CONFIG_SYS_LITTLE_ENDIAN
#define CONFIG_MACH_TYPE		4273

#ifndef CONFIG_SPL_BUILD
#define CONFIG_DM
#define CONFIG_CMD_DM

#define CONFIG_DM_GPIO
#define CONFIG_CMD_GPIO

#define CONFIG_DM_SERIAL
#define CONFIG_SYS_MALLOC_F_LEN		(1 << 10)
#endif

/* Display information on boot */
#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO
#define CONFIG_TIMESTAMP

/* CMD */
#include <config_cmd_default.h>
#define CONFIG_CMD_GREPENV
#undef CONFIG_CMD_FLASH
#undef CONFIG_CMD_LOADB
#undef CONFIG_CMD_LOADS
#undef CONFIG_CMD_XIMG
#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_IMLS

/* MMC */
#define CONFIG_MMC
#define CONFIG_CMD_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_FSL_ESDHC
#define CONFIG_FSL_USDHC
#define CONFIG_SYS_FSL_USDHC_NUM	3
#define CONFIG_SYS_FSL_ESDHC_ADDR	USDHC2_BASE_ADDR

/* RAM */
#define PHYS_SDRAM_1			MMDC0_ARB_BASE_ADDR
#define PHYS_SDRAM_2			MMDC1_ARB_BASE_ADDR
#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define CONFIG_NR_DRAM_BANKS		2
#define CONFIG_SYS_MEMTEST_START	0x10000000
#define CONFIG_SYS_MEMTEST_END		0x10010000
#define CONFIG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	IRAM_SIZE
#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* Serial console */
#define CONFIG_MXC_UART
#define CONFIG_MXC_UART_BASE		UART4_BASE
#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE	{9600, 19200, 38400, 57600, 115200}

/* Shell */
#define CONFIG_SYS_PROMPT	"CM-FX6 # "
#define CONFIG_SYS_CBSIZE	1024
#define CONFIG_SYS_MAXARGS	16
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE
#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)

/* SPI flash */
#define CONFIG_SYS_NO_FLASH
#define CONFIG_CMD_SF
#define CONFIG_SF_DEFAULT_BUS		0
#define CONFIG_SF_DEFAULT_CS		0
#define CONFIG_SF_DEFAULT_SPEED		25000000
#define CONFIG_SF_DEFAULT_MODE		(SPI_MODE_0)

/* Environment */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_SPI_MAX_HZ		CONFIG_SF_DEFAULT_SPEED
#define CONFIG_ENV_SPI_MODE		CONFIG_SF_DEFAULT_MODE
#define CONFIG_ENV_SPI_BUS		CONFIG_SF_DEFAULT_BUS
#define CONFIG_ENV_SPI_CS		CONFIG_SF_DEFAULT_CS
#define CONFIG_ENV_SECT_SIZE		(64 * 1024)
#define CONFIG_ENV_SIZE			(8 * 1024)
#define CONFIG_ENV_OFFSET		(768 * 1024)

#define CONFIG_EXTRA_ENV_SETTINGS \
	"kernel=uImage-cm-fx6\0" \
	"autoload=no\0" \
	"loadaddr=0x10800000\0" \
	"fdtaddr=0x11000000\0" \
	"console=ttymxc3,115200\0" \
	"ethprime=FEC0\0" \
	"bootscr=boot.scr\0" \
	"bootm_low=18000000\0" \
	"video_hdmi=mxcfb0:dev=hdmi,1920x1080M-32@50,if=RGB32\0" \
	"video_dvi=mxcfb0:dev=dvi,1280x800M-32@50,if=RGB32\0" \
	"fdtfile=cm-fx6.dtb\0" \
	"doboot=bootm ${loadaddr}\0" \
	"loadfdt=false\0" \
	"setboottypez=setenv kernel zImage-cm-fx6;" \
		"setenv doboot bootz ${loadaddr} - ${fdtaddr};" \
		"setenv loadfdt true;\0" \
	"setboottypem=setenv kernel uImage-cm-fx6;" \
		"setenv doboot bootm ${loadaddr};" \
		"setenv loadfdt false;\0"\
	"run_eboot=echo Starting EBOOT ...; "\
		"mmc dev ${mmcdev} && " \
		"mmc rescan && mmc read 10042000 a 400 && go 10042000\0" \
	"mmcdev=2\0" \
	"mmcroot=/dev/mmcblk0p2 rw rootwait\0" \
	"loadmmcbootscript=load mmc ${mmcdev} ${loadaddr} ${bootscr}\0" \
	"mmcbootscript=echo Running bootscript from mmc ...; "\
		"source ${loadaddr}\0" \
	"mmcargs=setenv bootargs console=${console} " \
		"root=${mmcroot} " \
		"${video}\0" \
	"mmcloadkernel=load mmc ${mmcdev} ${loadaddr} ${kernel}\0" \
	"mmcloadfdt=load mmc ${mmcdev} ${fdtaddr} ${fdtfile}\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs; " \
		"run doboot\0" \
	"satadev=0\0" \
	"sataroot=/dev/sda2 rw rootwait\0" \
	"sataargs=setenv bootargs console=${console} " \
		"root=${sataroot} " \
		"${video}\0" \
	"loadsatabootscript=load sata ${satadev} ${loadaddr} ${bootscr}\0" \
	"satabootscript=echo Running bootscript from sata ...; " \
		"source ${loadaddr}\0" \
	"sataloadkernel=load sata ${satadev} ${loadaddr} ${kernel}\0" \
	"sataloadfdt=load sata ${satadev} ${fdtaddr} ${fdtfile}\0" \
	"sataboot=echo Booting from sata ...; "\
		"run sataargs; " \
		"run doboot\0" \
	"nandroot=/dev/mtdblock4 rw\0" \
	"nandrootfstype=ubifs\0" \
	"nandargs=setenv bootargs console=${console} " \
		"root=${nandroot} " \
		"rootfstype=${nandrootfstype} " \
		"${video}\0" \
	"nandloadfdt=nand read ${fdtaddr} 780000 80000;\0" \
	"nandboot=echo Booting from nand ...; " \
		"run nandargs; " \
		"nand read ${loadaddr} 0 780000; " \
		"if ${loadfdt}; then " \
			"run nandloadfdt;" \
		"fi; " \
		"run doboot\0" \
	"boot=mmc dev ${mmcdev}; " \
		"if mmc rescan; then " \
			"if run loadmmcbootscript; then " \
				"run mmcbootscript;" \
			"else " \
				"if run mmcloadkernel; then " \
					"if ${loadfdt}; then " \
						"run mmcloadfdt;" \
					"fi;" \
					"run mmcboot;" \
				"fi;" \
			"fi;" \
		"fi;" \
		"if sata init; then " \
			"if run loadsatabootscript; then " \
				"run satabootscript;" \
			"else "\
				"if run sataloadkernel; then " \
					"if ${loadfdt}; then " \
						"run sataloadfdt; " \
					"fi;" \
					"run sataboot;" \
				"fi;" \
			"fi;" \
		"fi;" \
		"run nandboot\0"

#define CONFIG_BOOTCOMMAND \
	"run setboottypem; run boot"

/* SPI */
#define CONFIG_SPI
#define CONFIG_MXC_SPI
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_ATMEL
#define CONFIG_SPI_FLASH_EON
#define CONFIG_SPI_FLASH_GIGADEVICE
#define CONFIG_SPI_FLASH_MACRONIX
#define CONFIG_SPI_FLASH_SPANSION
#define CONFIG_SPI_FLASH_STMICRO
#define CONFIG_SPI_FLASH_SST
#define CONFIG_SPI_FLASH_WINBOND

/* NAND */
#ifndef CONFIG_SPL_BUILD
#define CONFIG_CMD_NAND
#define CONFIG_SYS_NAND_BASE		0x40000000
#define CONFIG_SYS_NAND_MAX_CHIPS	1
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_NAND_MXS
#define CONFIG_SYS_NAND_ONFI_DETECTION
/* APBH DMA is required for NAND support */
#define CONFIG_APBH_DMA
#define CONFIG_APBH_DMA_BURST
#define CONFIG_APBH_DMA_BURST8
#endif

/* Ethernet */
#define CONFIG_FEC_MXC
#define CONFIG_FEC_MXC_PHYADDR		0
#define CONFIG_FEC_XCV_TYPE		RGMII
#define IMX_FEC_BASE			ENET_BASE_ADDR
#define CONFIG_PHYLIB
#define CONFIG_PHY_ATHEROS
#define CONFIG_MII
#define CONFIG_ETHPRIME			"FEC0"
#define CONFIG_ARP_TIMEOUT		200UL
#define CONFIG_NET_RETRY_COUNT		5

/* USB */
#define CONFIG_CMD_USB
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_MX6
#define CONFIG_USB_STORAGE
#define CONFIG_MXC_USB_PORTSC		(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_MXC_USB_FLAGS		0
#define CONFIG_USB_MAX_CONTROLLER_COUNT	2
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET	/* For OTG port */

/* I2C */
#define CONFIG_CMD_I2C
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_SPEED		100000
#define CONFIG_SYS_MXC_I2C3_SPEED	400000

#define CONFIG_SYS_I2C_EEPROM_ADDR	0x50
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	1
#define CONFIG_SYS_I2C_EEPROM_BUS	2

/* SATA */
#define CONFIG_CMD_SATA
#define CONFIG_SYS_SATA_MAX_DEVICE	1
#define CONFIG_LIBATA
#define CONFIG_LBA48
#define CONFIG_DWC_AHSATA
#define CONFIG_DWC_AHSATA_PORT_ID	0
#define CONFIG_DWC_AHSATA_BASE_ADDR	SATA_ARB_BASE_ADDR

/* GPIO */
#define CONFIG_MXC_GPIO

/* Boot */
#define CONFIG_ZERO_BOOTDELAY_CHECK
#define CONFIG_LOADADDR			0x10800000
#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR
#define CONFIG_CMDLINE_TAG		/* enable passing of ATAGs */
#define CONFIG_SYS_BOOTMAPSZ	        (8 << 20)
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_REVISION_TAG
#define CONFIG_SERIAL_TAG

/* misc */
#define CONFIG_SYS_GENERIC_BOARD
#define CONFIG_STACKSIZE			(128 * 1024)
#define CONFIG_SYS_MALLOC_LEN			(2 * 1024 * 1024)
#define CONFIG_SYS_U_BOOT_MAX_SIZE_SECTORS	800 /* 400 KB */
#define CONFIG_OF_BOARD_SETUP

/* SPL */
#include "imx6_spl.h"
#define CONFIG_SPL_BOARD_INIT
#define CONFIG_SPL_MMC_SUPPORT
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR	0x80 /* offset 64 kb */
#define CONFIG_SYS_MONITOR_LEN	(CONFIG_SYS_U_BOOT_MAX_SIZE_SECTORS / 2 * 1024)
#define CONFIG_SPL_SPI_SUPPORT
#define CONFIG_SPL_SPI_FLASH_SUPPORT
#define CONFIG_SYS_SPI_U_BOOT_OFFS	(64 * 1024)
#define CONFIG_SPL_SPI_LOAD

#endif	/* __CONFIG_CM_FX6_H */
