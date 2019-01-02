
/*
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, daniel@omicron.se
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _PCI_I386_H_
#define _PCI_I386_H_

#define DEFINE_PCI_DEVICE_TABLE(_table) \
	const struct pci_device_id _table[]

struct pci_controller;

void pci_setup_type1(struct pci_controller *hose);

/**
 * board_pci_setup_hose() - Set up the PCI hose
 *
 * This is called by the common x86 PCI code to set up the PCI controller
 * hose. It may be called when no memory/BSS is available so should just
 * store things in 'hose' and not in BSS variables.
 */
void board_pci_setup_hose(struct pci_controller *hose);

/**
 * pci_early_init_hose() - Set up PCI host before relocation
 *
 * This allocates memory for, sets up and returns the PCI hose. It can be
 * called before relocation. The hose will be stored in gd->arch.hose for
 * later use, but will become invalid one DRAM is available.
 */
int pci_early_init_hose(struct pci_controller **hosep);

int board_pci_pre_scan(struct pci_controller *hose);
int board_pci_post_scan(struct pci_controller *hose);

/*
 * Simple PCI access routines - these work from either the early PCI hose
 * or the 'real' one, created after U-Boot has memory available
 */
unsigned int pci_read_config8(pci_dev_t dev, unsigned where);
unsigned int pci_read_config16(pci_dev_t dev, unsigned where);
unsigned int pci_read_config32(pci_dev_t dev, unsigned where);

void pci_write_config8(pci_dev_t dev, unsigned where, unsigned value);
void pci_write_config16(pci_dev_t dev, unsigned where, unsigned value);
void pci_write_config32(pci_dev_t dev, unsigned where, unsigned value);

#endif
