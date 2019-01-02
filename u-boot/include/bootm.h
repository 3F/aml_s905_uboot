/*
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _BOOTM_H
#define _BOOTM_H

#include <command.h>
#include <image.h>

#define BOOTM_ERR_RESET		(-1)
#define BOOTM_ERR_OVERLAP		(-2)
#define BOOTM_ERR_UNIMPLEMENTED	(-3)

/*
 *  Continue booting an OS image; caller already has:
 *  - copied image header to global variable `header'
 *  - checked header magic number, checksums (both header & image),
 *  - verified image architecture (PPC) and type (KERNEL or MULTI),
 *  - loaded (first part of) image to header load address,
 *  - disabled interrupts.
 *
 * @flag: Flags indicating what to do (BOOTM_STATE_...)
 * @argc: Number of arguments. Note that the arguments are shifted down
 *	 so that 0 is the first argument not processed by U-Boot, and
 *	 argc is adjusted accordingly. This avoids confusion as to how
 *	 many arguments are available for the OS.
 * @images: Pointers to os/initrd/fdt
 * @return 1 on error. On success the OS boots so this function does
 * not return.
 */
typedef int boot_os_fn(int flag, int argc, char * const argv[],
			bootm_headers_t *images);

extern boot_os_fn do_bootm_linux;
int do_bootelf(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
void lynxkdi_boot(image_header_t *hdr);

boot_os_fn *bootm_os_get_boot_func(int os);

int bootm_host_load_images(const void *fit, int cfg_noffset);

int boot_selected_os(int argc, char * const argv[], int state,
		     bootm_headers_t *images, boot_os_fn *boot_fn);

ulong bootm_disable_interrupts(void);

/* This is a special function used by booti/bootz */
int bootm_find_ramdisk_fdt(int flag, int argc, char * const argv[]);

int do_bootm_states(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[],
		    int states, bootm_headers_t *images, int boot_progress);

void arch_preboot_os(void);

#endif
