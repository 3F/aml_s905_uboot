/*
 * Copyright (C) 2014 Google, Inc
 *
 * From coreboot, originally based on the Linux kernel (drivers/pci/pci.c).
 *
 * Modifications are:
 * Copyright (C) 2003-2004 Linux Networx
 * (Written by Eric Biederman <ebiederman@lnxi.com> for Linux Networx)
 * Copyright (C) 2003-2006 Ronald G. Minnich <rminnich@gmail.com>
 * Copyright (C) 2004-2005 Li-Ta Lo <ollie@lanl.gov>
 * Copyright (C) 2005-2006 Tyan
 * (Written by Yinghai Lu <yhlu@tyan.com> for Tyan)
 * Copyright (C) 2005-2009 coresystems GmbH
 * (Written by Stefan Reinauer <stepan@coresystems.de> for coresystems GmbH)
 *
 * PCI Bus Services, see include/linux/pci.h for further explanation.
 *
 * Copyright 1993 -- 1997 Drew Eckhardt, Frederic Potter,
 * David Mosberger-Tang
 *
 * Copyright 1997 -- 1999 Martin Mares <mj@atrey.karlin.mff.cuni.cz>

 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <bios_emul.h>
#include <errno.h>
#include <malloc.h>
#include <pci.h>
#include <pci_rom.h>
#include <vbe.h>
#include <video_fb.h>

#ifdef CONFIG_HAVE_ACPI_RESUME
#include <asm/acpi.h>
#endif

__weak bool board_should_run_oprom(pci_dev_t dev)
{
	return true;
}

static bool should_load_oprom(pci_dev_t dev)
{
#ifdef CONFIG_HAVE_ACPI_RESUME
	if (acpi_get_slp_type() == 3)
		return false;
#endif
	if (IS_ENABLED(CONFIG_ALWAYS_LOAD_OPROM))
		return 1;
	if (board_should_run_oprom(dev))
		return 1;

	return 0;
}

__weak uint32_t board_map_oprom_vendev(uint32_t vendev)
{
	return vendev;
}

static int pci_rom_probe(pci_dev_t dev, uint class,
			 struct pci_rom_header **hdrp)
{
	struct pci_rom_header *rom_header;
	struct pci_rom_data *rom_data;
	u16 vendor, device;
	u32 vendev;
	u32 mapped_vendev;
	u32 rom_address;

	pci_read_config_word(dev, PCI_VENDOR_ID, &vendor);
	pci_read_config_word(dev, PCI_DEVICE_ID, &device);
	vendev = vendor << 16 | device;
	mapped_vendev = board_map_oprom_vendev(vendev);
	if (vendev != mapped_vendev)
		debug("Device ID mapped to %#08x\n", mapped_vendev);

#ifdef CONFIG_X86_OPTION_ROM_ADDR
	rom_address = CONFIG_X86_OPTION_ROM_ADDR;
#else
	pci_write_config_dword(dev, PCI_ROM_ADDRESS, (u32)PCI_ROM_ADDRESS_MASK);
	pci_read_config_dword(dev, PCI_ROM_ADDRESS, &rom_address);
	if (rom_address == 0x00000000 || rom_address == 0xffffffff) {
		debug("%s: rom_address=%x\n", __func__, rom_address);
		return -ENOENT;
	}

	/* Enable expansion ROM address decoding. */
	pci_write_config_dword(dev, PCI_ROM_ADDRESS,
			       rom_address | PCI_ROM_ADDRESS_ENABLE);
#endif
	debug("Option ROM address %x\n", rom_address);
	rom_header = (struct pci_rom_header *)rom_address;

	debug("PCI expansion ROM, signature %#04x, INIT size %#04x, data ptr %#04x\n",
	      le32_to_cpu(rom_header->signature),
	      rom_header->size * 512, le32_to_cpu(rom_header->data));

	if (le32_to_cpu(rom_header->signature) != PCI_ROM_HDR) {
		printf("Incorrect expansion ROM header signature %04x\n",
		       le32_to_cpu(rom_header->signature));
		return -EINVAL;
	}

	rom_data = (((void *)rom_header) + le32_to_cpu(rom_header->data));

	debug("PCI ROM image, vendor ID %04x, device ID %04x,\n",
	      rom_data->vendor, rom_data->device);

	/* If the device id is mapped, a mismatch is expected */
	if ((vendor != rom_data->vendor || device != rom_data->device) &&
	    (vendev == mapped_vendev)) {
		printf("ID mismatch: vendor ID %04x, device ID %04x\n",
		       rom_data->vendor, rom_data->device);
		return -EPERM;
	}

	debug("PCI ROM image, Class Code %04x%02x, Code Type %02x\n",
	      rom_data->class_hi, rom_data->class_lo, rom_data->type);

	if (class != ((rom_data->class_hi << 8) | rom_data->class_lo)) {
		debug("Class Code mismatch ROM %08x, dev %08x\n",
		      (rom_data->class_hi << 8) | rom_data->class_lo,
		      class);
	}
	*hdrp = rom_header;

	return 0;
}

int pci_rom_load(uint16_t class, struct pci_rom_header *rom_header,
		 struct pci_rom_header **ram_headerp)
{
	struct pci_rom_data *rom_data;
	unsigned int rom_size;
	unsigned int image_size = 0;
	void *target;

	do {
		/* Get next image, until we see an x86 version */
		rom_header = (struct pci_rom_header *)((void *)rom_header +
							    image_size);

		rom_data = (struct pci_rom_data *)((void *)rom_header +
				le32_to_cpu(rom_header->data));

		image_size = le32_to_cpu(rom_data->ilen) * 512;
	} while ((rom_data->type != 0) && (rom_data->indicator != 0));

	if (rom_data->type != 0)
		return -EACCES;

	rom_size = rom_header->size * 512;

	target = (void *)PCI_VGA_RAM_IMAGE_START;
	if (target != rom_header) {
		debug("Copying VGA ROM Image from %p to %p, 0x%x bytes\n",
		      rom_header, target, rom_size);
		memcpy(target, rom_header, rom_size);
		if (memcmp(target, rom_header, rom_size)) {
			printf("VGA ROM copy failed\n");
			return -EFAULT;
		}
	}
	*ram_headerp = target;

	return 0;
}

static struct vbe_mode_info mode_info;

int vbe_get_video_info(struct graphic_device *gdev)
{
#ifdef CONFIG_FRAMEBUFFER_SET_VESA_MODE
	struct vesa_mode_info *vesa = &mode_info.vesa;

	gdev->winSizeX = vesa->x_resolution;
	gdev->winSizeY = vesa->y_resolution;

	gdev->plnSizeX = vesa->x_resolution;
	gdev->plnSizeY = vesa->y_resolution;

	gdev->gdfBytesPP = vesa->bits_per_pixel / 8;

	switch (vesa->bits_per_pixel) {
	case 24:
		gdev->gdfIndex = GDF_32BIT_X888RGB;
		break;
	case 16:
		gdev->gdfIndex = GDF_16BIT_565RGB;
		break;
	default:
		gdev->gdfIndex = GDF__8BIT_INDEX;
		break;
	}

	gdev->isaBase = CONFIG_SYS_ISA_IO_BASE_ADDRESS;
	gdev->pciBase = vesa->phys_base_ptr;

	gdev->frameAdrs = vesa->phys_base_ptr;
	gdev->memSize = vesa->bytes_per_scanline * vesa->y_resolution;

	gdev->vprBase = vesa->phys_base_ptr;
	gdev->cprBase = vesa->phys_base_ptr;

	return 0;
#else
	return -ENOSYS;
#endif
}

int pci_run_vga_bios(pci_dev_t dev, int (*int15_handler)(void), bool emulate)
{
	struct pci_rom_header *rom, *ram;
	int vesa_mode = -1;
	uint16_t class;
	int ret;

	/* Only execute VGA ROMs */
	pci_read_config_word(dev, PCI_CLASS_DEVICE, &class);
	if ((class ^ PCI_CLASS_DISPLAY_VGA) & 0xff00) {
		debug("%s: Class %#x, should be %#x\n", __func__, class,
		      PCI_CLASS_DISPLAY_VGA);
		return -ENODEV;
	}

	if (!should_load_oprom(dev))
		return -ENXIO;

	ret = pci_rom_probe(dev, class, &rom);
	if (ret)
		return ret;

	ret = pci_rom_load(class, rom, &ram);
	if (ret)
		return ret;

	if (!board_should_run_oprom(dev))
		return -ENXIO;

#if defined(CONFIG_FRAMEBUFFER_SET_VESA_MODE) && \
		defined(CONFIG_FRAMEBUFFER_VESA_MODE)
	vesa_mode = CONFIG_FRAMEBUFFER_VESA_MODE;
#endif
	debug("Selected vesa mode %d\b", vesa_mode);
	if (emulate) {
#ifdef CONFIG_BIOSEMU
		BE_VGAInfo *info;

		ret = biosemu_setup(dev, &info);
		if (ret)
			return ret;
		biosemu_set_interrupt_handler(0x15, int15_handler);
		ret = biosemu_run(dev, (uchar *)ram, 1 << 16, info, true,
				  vesa_mode, &mode_info);
		if (ret)
			return ret;
#else
		printf("BIOS emulation not available - see CONFIG_BIOSEMU\n");
		return -ENOSYS;
#endif
	} else {
#ifdef CONFIG_X86
		bios_set_interrupt_handler(0x15, int15_handler);

		bios_run_on_x86(dev, (unsigned long)ram, vesa_mode,
				&mode_info);
#else
		printf("BIOS native execution is only available on x86\n");
		return -ENOSYS;
#endif
	}
	debug("Final vesa mode %d\n", mode_info.video_mode);

	return 0;
}
