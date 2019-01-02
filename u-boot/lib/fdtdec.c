/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef USE_HOSTCC
#include <common.h>
#include <errno.h>
#include <serial.h>
#include <libfdt.h>
#include <fdtdec.h>
#include <linux/ctype.h>

#include <asm/gpio.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Here are the type we know about. One day we might allow drivers to
 * register. For now we just put them here. The COMPAT macro allows us to
 * turn this into a sparse list later, and keeps the ID with the name.
 */
#define COMPAT(id, name) name
static const char * const compat_names[COMPAT_COUNT] = {
	COMPAT(UNKNOWN, "<none>"),
	COMPAT(NVIDIA_TEGRA20_USB, "nvidia,tegra20-ehci"),
	COMPAT(NVIDIA_TEGRA30_USB, "nvidia,tegra30-ehci"),
	COMPAT(NVIDIA_TEGRA114_USB, "nvidia,tegra114-ehci"),
	COMPAT(NVIDIA_TEGRA114_I2C, "nvidia,tegra114-i2c"),
	COMPAT(NVIDIA_TEGRA20_I2C, "nvidia,tegra20-i2c"),
	COMPAT(NVIDIA_TEGRA20_DVC, "nvidia,tegra20-i2c-dvc"),
	COMPAT(NVIDIA_TEGRA20_EMC, "nvidia,tegra20-emc"),
	COMPAT(NVIDIA_TEGRA20_EMC_TABLE, "nvidia,tegra20-emc-table"),
	COMPAT(NVIDIA_TEGRA20_KBC, "nvidia,tegra20-kbc"),
	COMPAT(NVIDIA_TEGRA20_NAND, "nvidia,tegra20-nand"),
	COMPAT(NVIDIA_TEGRA20_PWM, "nvidia,tegra20-pwm"),
	COMPAT(NVIDIA_TEGRA20_DC, "nvidia,tegra20-dc"),
	COMPAT(NVIDIA_TEGRA124_SDMMC, "nvidia,tegra124-sdhci"),
	COMPAT(NVIDIA_TEGRA30_SDMMC, "nvidia,tegra30-sdhci"),
	COMPAT(NVIDIA_TEGRA20_SDMMC, "nvidia,tegra20-sdhci"),
	COMPAT(NVIDIA_TEGRA20_SFLASH, "nvidia,tegra20-sflash"),
	COMPAT(NVIDIA_TEGRA20_SLINK, "nvidia,tegra20-slink"),
	COMPAT(NVIDIA_TEGRA114_SPI, "nvidia,tegra114-spi"),
	COMPAT(NVIDIA_TEGRA124_PCIE, "nvidia,tegra124-pcie"),
	COMPAT(NVIDIA_TEGRA30_PCIE, "nvidia,tegra30-pcie"),
	COMPAT(NVIDIA_TEGRA20_PCIE, "nvidia,tegra20-pcie"),
	COMPAT(NVIDIA_TEGRA124_XUSB_PADCTL, "nvidia,tegra124-xusb-padctl"),
	COMPAT(SMSC_LAN9215, "smsc,lan9215"),
	COMPAT(SAMSUNG_EXYNOS5_SROMC, "samsung,exynos-sromc"),
	COMPAT(SAMSUNG_S3C2440_I2C, "samsung,s3c2440-i2c"),
	COMPAT(SAMSUNG_EXYNOS5_SOUND, "samsung,exynos-sound"),
	COMPAT(WOLFSON_WM8994_CODEC, "wolfson,wm8994-codec"),
	COMPAT(SAMSUNG_EXYNOS_SPI, "samsung,exynos-spi"),
	COMPAT(GOOGLE_CROS_EC, "google,cros-ec"),
	COMPAT(GOOGLE_CROS_EC_KEYB, "google,cros-ec-keyb"),
	COMPAT(SAMSUNG_EXYNOS_EHCI, "samsung,exynos-ehci"),
	COMPAT(SAMSUNG_EXYNOS5_XHCI, "samsung,exynos5250-xhci"),
	COMPAT(SAMSUNG_EXYNOS_USB_PHY, "samsung,exynos-usb-phy"),
	COMPAT(SAMSUNG_EXYNOS5_USB3_PHY, "samsung,exynos5250-usb3-phy"),
	COMPAT(SAMSUNG_EXYNOS_TMU, "samsung,exynos-tmu"),
	COMPAT(SAMSUNG_EXYNOS_FIMD, "samsung,exynos-fimd"),
	COMPAT(SAMSUNG_EXYNOS_MIPI_DSI, "samsung,exynos-mipi-dsi"),
	COMPAT(SAMSUNG_EXYNOS5_DP, "samsung,exynos5-dp"),
	COMPAT(SAMSUNG_EXYNOS_DWMMC, "samsung,exynos-dwmmc"),
	COMPAT(SAMSUNG_EXYNOS_MMC, "samsung,exynos-mmc"),
	COMPAT(SAMSUNG_EXYNOS_SERIAL, "samsung,exynos4210-uart"),
	COMPAT(MAXIM_MAX77686_PMIC, "maxim,max77686_pmic"),
	COMPAT(GENERIC_SPI_FLASH, "spi-flash"),
	COMPAT(MAXIM_98095_CODEC, "maxim,max98095-codec"),
	COMPAT(INFINEON_SLB9635_TPM, "infineon,slb9635-tpm"),
	COMPAT(INFINEON_SLB9645_TPM, "infineon,slb9645-tpm"),
	COMPAT(SAMSUNG_EXYNOS5_I2C, "samsung,exynos5-hsi2c"),
	COMPAT(SANDBOX_HOST_EMULATION, "sandbox,host-emulation"),
	COMPAT(SANDBOX_LCD_SDL, "sandbox,lcd-sdl"),
	COMPAT(TI_TPS65090, "ti,tps65090"),
	COMPAT(COMPAT_NXP_PTN3460, "nxp,ptn3460"),
	COMPAT(SAMSUNG_EXYNOS_SYSMMU, "samsung,sysmmu-v3.3"),
	COMPAT(PARADE_PS8625, "parade,ps8625"),
	COMPAT(COMPAT_INTEL_LPC, "intel,lpc"),
	COMPAT(INTEL_MICROCODE, "intel,microcode"),
	COMPAT(MEMORY_SPD, "memory-spd"),
	COMPAT(INTEL_PANTHERPOINT_AHCI, "intel,pantherpoint-ahci"),
	COMPAT(INTEL_MODEL_206AX, "intel,model-206ax"),
	COMPAT(INTEL_GMA, "intel,gma"),
	COMPAT(AMS_AS3722, "ams,as3722"),
};

const char *fdtdec_get_compatible(enum fdt_compat_id id)
{
	/* We allow reading of the 'unknown' ID for testing purposes */
	assert(id >= 0 && id < COMPAT_COUNT);
	return compat_names[id];
}

fdt_addr_t fdtdec_get_addr_size(const void *blob, int node,
		const char *prop_name, fdt_size_t *sizep)
{
	const fdt_addr_t *cell;
	int len;

	debug("%s: %s: ", __func__, prop_name);
	cell = fdt_getprop(blob, node, prop_name, &len);
	if (cell && ((!sizep && len == sizeof(fdt_addr_t)) ||
		     len == sizeof(fdt_addr_t) * 2)) {
		fdt_addr_t addr = fdt_addr_to_cpu(*cell);
		if (sizep) {
			const fdt_size_t *size;

			size = (fdt_size_t *)((char *)cell +
					sizeof(fdt_addr_t));
			*sizep = fdt_size_to_cpu(*size);
			debug("addr=%08lx, size=%08x\n",
			      (ulong)addr, *sizep);
		} else {
			debug("%08lx\n", (ulong)addr);
		}
		return addr;
	}
	debug("(not found)\n");
	return FDT_ADDR_T_NONE;
}

fdt_addr_t fdtdec_get_addr(const void *blob, int node,
		const char *prop_name)
{
	return fdtdec_get_addr_size(blob, node, prop_name, NULL);
}

uint64_t fdtdec_get_uint64(const void *blob, int node, const char *prop_name,
		uint64_t default_val)
{
	const uint64_t *cell64;
	int length;

	cell64 = fdt_getprop(blob, node, prop_name, &length);
	if (!cell64 || length < sizeof(*cell64))
		return default_val;

	return fdt64_to_cpu(*cell64);
}

int fdtdec_get_is_enabled(const void *blob, int node)
{
	const char *cell;

	/*
	 * It should say "okay", so only allow that. Some fdts use "ok" but
	 * this is a bug. Please fix your device tree source file. See here
	 * for discussion:
	 *
	 * http://www.mail-archive.com/u-boot@lists.denx.de/msg71598.html
	 */
	cell = fdt_getprop(blob, node, "status", NULL);
	if (cell)
		return 0 == strcmp(cell, "okay");
	return 1;
}

enum fdt_compat_id fdtdec_lookup(const void *blob, int node)
{
	enum fdt_compat_id id;

	/* Search our drivers */
	for (id = COMPAT_UNKNOWN; id < COMPAT_COUNT; id++)
		if (0 == fdt_node_check_compatible(blob, node,
				compat_names[id]))
			return id;
	return COMPAT_UNKNOWN;
}

int fdtdec_next_compatible(const void *blob, int node,
		enum fdt_compat_id id)
{
	return fdt_node_offset_by_compatible(blob, node, compat_names[id]);
}

int fdtdec_next_compatible_subnode(const void *blob, int node,
		enum fdt_compat_id id, int *depthp)
{
	do {
		node = fdt_next_node(blob, node, depthp);
	} while (*depthp > 1);

	/* If this is a direct subnode, and compatible, return it */
	if (*depthp == 1 && 0 == fdt_node_check_compatible(
						blob, node, compat_names[id]))
		return node;

	return -FDT_ERR_NOTFOUND;
}

int fdtdec_next_alias(const void *blob, const char *name,
		enum fdt_compat_id id, int *upto)
{
#define MAX_STR_LEN 20
	char str[MAX_STR_LEN + 20];
	int node, err;

	/* snprintf() is not available */
	assert(strlen(name) < MAX_STR_LEN);
	sprintf(str, "%.*s%d", MAX_STR_LEN, name, *upto);
	node = fdt_path_offset(blob, str);
	if (node < 0)
		return node;
	err = fdt_node_check_compatible(blob, node, compat_names[id]);
	if (err < 0)
		return err;
	if (err)
		return -FDT_ERR_NOTFOUND;
	(*upto)++;
	return node;
}

int fdtdec_find_aliases_for_id(const void *blob, const char *name,
			enum fdt_compat_id id, int *node_list, int maxcount)
{
	memset(node_list, '\0', sizeof(*node_list) * maxcount);

	return fdtdec_add_aliases_for_id(blob, name, id, node_list, maxcount);
}

/* TODO: Can we tighten this code up a little? */
int fdtdec_add_aliases_for_id(const void *blob, const char *name,
			enum fdt_compat_id id, int *node_list, int maxcount)
{
	int name_len = strlen(name);
	int nodes[maxcount];
	int num_found = 0;
	int offset, node;
	int alias_node;
	int count;
	int i, j;

	/* find the alias node if present */
	alias_node = fdt_path_offset(blob, "/aliases");

	/*
	 * start with nothing, and we can assume that the root node can't
	 * match
	 */
	memset(nodes, '\0', sizeof(nodes));

	/* First find all the compatible nodes */
	for (node = count = 0; node >= 0 && count < maxcount;) {
		node = fdtdec_next_compatible(blob, node, id);
		if (node >= 0)
			nodes[count++] = node;
	}
	if (node >= 0)
		debug("%s: warning: maxcount exceeded with alias '%s'\n",
		       __func__, name);

	/* Now find all the aliases */
	for (offset = fdt_first_property_offset(blob, alias_node);
			offset > 0;
			offset = fdt_next_property_offset(blob, offset)) {
		const struct fdt_property *prop;
		const char *path;
		int number;
		int found;

		node = 0;
		prop = fdt_get_property_by_offset(blob, offset, NULL);
		path = fdt_string(blob, fdt32_to_cpu(prop->nameoff));
		if (prop->len && 0 == strncmp(path, name, name_len))
			node = fdt_path_offset(blob, prop->data);
		if (node <= 0)
			continue;

		/* Get the alias number */
		number = simple_strtoul(path + name_len, NULL, 10);
		if (number < 0 || number >= maxcount) {
			debug("%s: warning: alias '%s' is out of range\n",
			       __func__, path);
			continue;
		}

		/* Make sure the node we found is actually in our list! */
		found = -1;
		for (j = 0; j < count; j++)
			if (nodes[j] == node) {
				found = j;
				break;
			}

		if (found == -1) {
			debug("%s: warning: alias '%s' points to a node "
				"'%s' that is missing or is not compatible "
				" with '%s'\n", __func__, path,
				fdt_get_name(blob, node, NULL),
			       compat_names[id]);
			continue;
		}

		/*
		 * Add this node to our list in the right place, and mark
		 * it as done.
		 */
		if (fdtdec_get_is_enabled(blob, node)) {
			if (node_list[number]) {
				debug("%s: warning: alias '%s' requires that "
				      "a node be placed in the list in a "
				      "position which is already filled by "
				      "node '%s'\n", __func__, path,
				      fdt_get_name(blob, node, NULL));
				continue;
			}
			node_list[number] = node;
			if (number >= num_found)
				num_found = number + 1;
		}
		nodes[found] = 0;
	}

	/* Add any nodes not mentioned by an alias */
	for (i = j = 0; i < maxcount; i++) {
		if (!node_list[i]) {
			for (; j < maxcount; j++)
				if (nodes[j] &&
					fdtdec_get_is_enabled(blob, nodes[j]))
					break;

			/* Have we run out of nodes to add? */
			if (j == maxcount)
				break;

			assert(!node_list[i]);
			node_list[i] = nodes[j++];
			if (i >= num_found)
				num_found = i + 1;
		}
	}

	return num_found;
}

int fdtdec_get_alias_seq(const void *blob, const char *base, int offset,
			 int *seqp)
{
	int base_len = strlen(base);
	const char *find_name;
	int find_namelen;
	int prop_offset;
	int aliases;

	find_name = fdt_get_name(blob, offset, &find_namelen);
	debug("Looking for '%s' at %d, name %s\n", base, offset, find_name);

	aliases = fdt_path_offset(blob, "/aliases");
	for (prop_offset = fdt_first_property_offset(blob, aliases);
	     prop_offset > 0;
	     prop_offset = fdt_next_property_offset(blob, prop_offset)) {
		const char *prop;
		const char *name;
		const char *slash;
		const char *p;
		int len;

		prop = fdt_getprop_by_offset(blob, prop_offset, &name, &len);
		debug("   - %s, %s\n", name, prop);
		if (len < find_namelen || *prop != '/' || prop[len - 1] ||
		    strncmp(name, base, base_len))
			continue;

		slash = strrchr(prop, '/');
		if (strcmp(slash + 1, find_name))
			continue;
		for (p = name + strlen(name) - 1; p > name; p--) {
			if (!isdigit(*p)) {
				*seqp = simple_strtoul(p + 1, NULL, 10);
				debug("Found seq %d\n", *seqp);
				return 0;
			}
		}
	}

	debug("Not found\n");
	return -ENOENT;
}

int fdtdec_get_chosen_node(const void *blob, const char *name)
{
	const char *prop;
	int chosen_node;
	int len;

	if (!blob)
		return -FDT_ERR_NOTFOUND;
	chosen_node = fdt_path_offset(blob, "/chosen");
	prop = fdt_getprop(blob, chosen_node, name, &len);
	if (!prop)
		return -FDT_ERR_NOTFOUND;
	return fdt_path_offset(blob, prop);
}

int fdtdec_check_fdt(void)
{
	/*
	 * We must have an FDT, but we cannot panic() yet since the console
	 * is not ready. So for now, just assert(). Boards which need an early
	 * FDT (prior to console ready) will need to make their own
	 * arrangements and do their own checks.
	 */
	assert(!fdtdec_prepare_fdt());
	return 0;
}

/*
 * This function is a little odd in that it accesses global data. At some
 * point if the architecture board.c files merge this will make more sense.
 * Even now, it is common code.
 */
int fdtdec_prepare_fdt(void)
{
	if (!gd->fdt_blob || ((uintptr_t)gd->fdt_blob & 3) ||
	    fdt_check_header(gd->fdt_blob)) {
		printf("No valid FDT found - please append one to U-Boot "
			"binary, use u-boot-dtb.bin or define "
			"CONFIG_OF_EMBED. For sandbox, use -d <file.dtb>\n");
		return -1;
	}
	return 0;
}

int fdtdec_lookup_phandle(const void *blob, int node, const char *prop_name)
{
	const u32 *phandle;
	int lookup;

	debug("%s: %s\n", __func__, prop_name);
	phandle = fdt_getprop(blob, node, prop_name, NULL);
	if (!phandle)
		return -FDT_ERR_NOTFOUND;

	lookup = fdt_node_offset_by_phandle(blob, fdt32_to_cpu(*phandle));
	return lookup;
}

/**
 * Look up a property in a node and check that it has a minimum length.
 *
 * @param blob		FDT blob
 * @param node		node to examine
 * @param prop_name	name of property to find
 * @param min_len	minimum property length in bytes
 * @param err		0 if ok, or -FDT_ERR_NOTFOUND if the property is not
			found, or -FDT_ERR_BADLAYOUT if not enough data
 * @return pointer to cell, which is only valid if err == 0
 */
static const void *get_prop_check_min_len(const void *blob, int node,
		const char *prop_name, int min_len, int *err)
{
	const void *cell;
	int len;

	debug("%s: %s\n", __func__, prop_name);
	cell = fdt_getprop(blob, node, prop_name, &len);
	if (!cell)
		*err = -FDT_ERR_NOTFOUND;
	else if (len < min_len)
		*err = -FDT_ERR_BADLAYOUT;
	else
		*err = 0;
	return cell;
}

int fdtdec_get_int_array(const void *blob, int node, const char *prop_name,
		u32 *array, int count)
{
	const u32 *cell;
	int i, err = 0;

	debug("%s: %s\n", __func__, prop_name);
	cell = get_prop_check_min_len(blob, node, prop_name,
				      sizeof(u32) * count, &err);
	if (!err) {
		for (i = 0; i < count; i++)
			array[i] = fdt32_to_cpu(cell[i]);
	}
	return err;
}

int fdtdec_get_int_array_count(const void *blob, int node,
			       const char *prop_name, u32 *array, int count)
{
	const u32 *cell;
	int len, elems;
	int i;

	debug("%s: %s\n", __func__, prop_name);
	cell = fdt_getprop(blob, node, prop_name, &len);
	if (!cell)
		return -FDT_ERR_NOTFOUND;
	elems = len / sizeof(u32);
	if (count > elems)
		count = elems;
	for (i = 0; i < count; i++)
		array[i] = fdt32_to_cpu(cell[i]);

	return count;
}

const u32 *fdtdec_locate_array(const void *blob, int node,
			       const char *prop_name, int count)
{
	const u32 *cell;
	int err;

	cell = get_prop_check_min_len(blob, node, prop_name,
				      sizeof(u32) * count, &err);
	return err ? NULL : cell;
}

int fdtdec_get_bool(const void *blob, int node, const char *prop_name)
{
	const s32 *cell;
	int len;

	debug("%s: %s\n", __func__, prop_name);
	cell = fdt_getprop(blob, node, prop_name, &len);
	return cell != NULL;
}

/**
 * Decode a list of GPIOs from an FDT. This creates a list of GPIOs with no
 * terminating item.
 *
 * @param blob		FDT blob to use
 * @param node		Node to look at
 * @param prop_name	Node property name
 * @param gpio		Array of gpio elements to fill from FDT. This will be
 *			untouched if either 0 or an error is returned
 * @param max_count	Maximum number of elements allowed
 * @return number of GPIOs read if ok, -FDT_ERR_BADLAYOUT if max_count would
 * be exceeded, or -FDT_ERR_NOTFOUND if the property is missing.
 */
int fdtdec_decode_gpios(const void *blob, int node, const char *prop_name,
		struct fdt_gpio_state *gpio, int max_count)
{
	const struct fdt_property *prop;
	const u32 *cell;
	const char *name;
	int len, i;

	debug("%s: %s\n", __func__, prop_name);
	assert(max_count > 0);
	prop = fdt_get_property(blob, node, prop_name, &len);
	if (!prop) {
		debug("%s: property '%s' missing\n", __func__, prop_name);
		return -FDT_ERR_NOTFOUND;
	}

	/* We will use the name to tag the GPIO */
	name = fdt_string(blob, fdt32_to_cpu(prop->nameoff));
	cell = (u32 *)prop->data;
	len /= sizeof(u32) * 3;		/* 3 cells per GPIO record */
	if (len > max_count) {
		debug(" %s: too many GPIOs / cells for "
			"property '%s'\n", __func__, prop_name);
		return -FDT_ERR_BADLAYOUT;
	}

	/* Read out the GPIO data from the cells */
	for (i = 0; i < len; i++, cell += 3) {
		gpio[i].gpio = fdt32_to_cpu(cell[1]);
		gpio[i].flags = fdt32_to_cpu(cell[2]);
		gpio[i].name = name;
	}

	return len;
}

int fdtdec_decode_gpio(const void *blob, int node, const char *prop_name,
		struct fdt_gpio_state *gpio)
{
	int err;

	debug("%s: %s\n", __func__, prop_name);
	gpio->gpio = FDT_GPIO_NONE;
	gpio->name = NULL;
	err = fdtdec_decode_gpios(blob, node, prop_name, gpio, 1);
	return err == 1 ? 0 : err;
}

int fdtdec_get_gpio(struct fdt_gpio_state *gpio)
{
	int val;

	if (!fdt_gpio_isvalid(gpio))
		return -1;

	val = gpio_get_value(gpio->gpio);
	return gpio->flags & FDT_GPIO_ACTIVE_LOW ? val ^ 1 : val;
}

int fdtdec_set_gpio(struct fdt_gpio_state *gpio, int val)
{
	if (!fdt_gpio_isvalid(gpio))
		return -1;

	val = gpio->flags & FDT_GPIO_ACTIVE_LOW ? val ^ 1 : val;
	return gpio_set_value(gpio->gpio, val);
}

int fdtdec_setup_gpio(struct fdt_gpio_state *gpio)
{
	/*
	 * Return success if there is no GPIO defined. This is used for
	 * optional GPIOs)
	 */
	if (!fdt_gpio_isvalid(gpio))
		return 0;

	if (gpio_request(gpio->gpio, gpio->name))
		return -1;
	return 0;
}

int fdtdec_get_byte_array(const void *blob, int node, const char *prop_name,
		u8 *array, int count)
{
	const u8 *cell;
	int err;

	cell = get_prop_check_min_len(blob, node, prop_name, count, &err);
	if (!err)
		memcpy(array, cell, count);
	return err;
}

const u8 *fdtdec_locate_byte_array(const void *blob, int node,
			     const char *prop_name, int count)
{
	const u8 *cell;
	int err;

	cell = get_prop_check_min_len(blob, node, prop_name, count, &err);
	if (err)
		return NULL;
	return cell;
}

int fdtdec_get_config_int(const void *blob, const char *prop_name,
		int default_val)
{
	int config_node;

	debug("%s: %s\n", __func__, prop_name);
	config_node = fdt_path_offset(blob, "/config");
	if (config_node < 0)
		return default_val;
	return fdtdec_get_int(blob, config_node, prop_name, default_val);
}

int fdtdec_get_config_bool(const void *blob, const char *prop_name)
{
	int config_node;
	const void *prop;

	debug("%s: %s\n", __func__, prop_name);
	config_node = fdt_path_offset(blob, "/config");
	if (config_node < 0)
		return 0;
	prop = fdt_get_property(blob, config_node, prop_name, NULL);

	return prop != NULL;
}

char *fdtdec_get_config_string(const void *blob, const char *prop_name)
{
	const char *nodep;
	int nodeoffset;
	int len;

	debug("%s: %s\n", __func__, prop_name);
	nodeoffset = fdt_path_offset(blob, "/config");
	if (nodeoffset < 0)
		return NULL;

	nodep = fdt_getprop(blob, nodeoffset, prop_name, &len);
	if (!nodep)
		return NULL;

	return (char *)nodep;
}

int fdtdec_decode_region(const void *blob, int node, const char *prop_name,
			 fdt_addr_t *basep, fdt_size_t *sizep)
{
	const fdt_addr_t *cell;
	int len;

	debug("%s: %s: %s\n", __func__, fdt_get_name(blob, node, NULL),
	      prop_name);
	cell = fdt_getprop(blob, node, prop_name, &len);
	if (!cell || (len < sizeof(fdt_addr_t) * 2)) {
		debug("cell=%p, len=%d\n", cell, len);
		return -1;
	}

	*basep = fdt_addr_to_cpu(*cell);
	*sizep = fdt_size_to_cpu(cell[1]);
	debug("%s: base=%08lx, size=%lx\n", __func__, (ulong)*basep,
	      (ulong)*sizep);

	return 0;
}

/**
 * Read a flash entry from the fdt
 *
 * @param blob		FDT blob
 * @param node		Offset of node to read
 * @param name		Name of node being read
 * @param entry		Place to put offset and size of this node
 * @return 0 if ok, -ve on error
 */
int fdtdec_read_fmap_entry(const void *blob, int node, const char *name,
			   struct fmap_entry *entry)
{
	const char *prop;
	u32 reg[2];

	if (fdtdec_get_int_array(blob, node, "reg", reg, 2)) {
		debug("Node '%s' has bad/missing 'reg' property\n", name);
		return -FDT_ERR_NOTFOUND;
	}
	entry->offset = reg[0];
	entry->length = reg[1];
	entry->used = fdtdec_get_int(blob, node, "used", entry->length);
	prop = fdt_getprop(blob, node, "compress", NULL);
	entry->compress_algo = prop && !strcmp(prop, "lzo") ?
		FMAP_COMPRESS_LZO : FMAP_COMPRESS_NONE;
	prop = fdt_getprop(blob, node, "hash", &entry->hash_size);
	entry->hash_algo = prop ? FMAP_HASH_SHA256 : FMAP_HASH_NONE;
	entry->hash = (uint8_t *)prop;

	return 0;
}

static u64 fdtdec_get_number(const fdt32_t *ptr, unsigned int cells)
{
	u64 number = 0;

	while (cells--)
		number = (number << 32) | fdt32_to_cpu(*ptr++);

	return number;
}

int fdt_get_resource(const void *fdt, int node, const char *property,
		     unsigned int index, struct fdt_resource *res)
{
	const fdt32_t *ptr, *end;
	int na, ns, len, parent;
	unsigned int i = 0;

	parent = fdt_parent_offset(fdt, node);
	if (parent < 0)
		return parent;

	na = fdt_address_cells(fdt, parent);
	ns = fdt_size_cells(fdt, parent);

	ptr = fdt_getprop(fdt, node, property, &len);
	if (!ptr)
		return len;

	end = ptr + len / sizeof(*ptr);

	while (ptr + na + ns <= end) {
		if (i == index) {
			res->start = res->end = fdtdec_get_number(ptr, na);
			res->end += fdtdec_get_number(&ptr[na], ns) - 1;
			return 0;
		}

		ptr += na + ns;
		i++;
	}

	return -FDT_ERR_NOTFOUND;
}

int fdt_get_named_resource(const void *fdt, int node, const char *property,
			   const char *prop_names, const char *name,
			   struct fdt_resource *res)
{
	int index;

	index = fdt_find_string(fdt, node, prop_names, name);
	if (index < 0)
		return index;

	return fdt_get_resource(fdt, node, property, index, res);
}

int fdtdec_pci_get_bdf(const void *fdt, int node, int *bdf)
{
	const fdt32_t *prop;
	int len;

	prop = fdt_getprop(fdt, node, "reg", &len);
	if (!prop)
		return len;

	*bdf = fdt32_to_cpu(*prop) & 0xffffff;

	return 0;
}

int fdtdec_decode_memory_region(const void *blob, int config_node,
				const char *mem_type, const char *suffix,
				fdt_addr_t *basep, fdt_size_t *sizep)
{
	char prop_name[50];
	const char *mem;
	fdt_size_t size, offset_size;
	fdt_addr_t base, offset;
	int node;

	if (config_node == -1) {
		config_node = fdt_path_offset(blob, "/config");
		if (config_node < 0) {
			debug("%s: Cannot find /config node\n", __func__);
			return -ENOENT;
		}
	}
	if (!suffix)
		suffix = "";

	snprintf(prop_name, sizeof(prop_name), "%s-memory%s", mem_type,
		 suffix);
	mem = fdt_getprop(blob, config_node, prop_name, NULL);
	if (!mem) {
		debug("%s: No memory type for '%s', using /memory\n", __func__,
		      prop_name);
		mem = "/memory";
	}

	node = fdt_path_offset(blob, mem);
	if (node < 0) {
		debug("%s: Failed to find node '%s': %s\n", __func__, mem,
		      fdt_strerror(node));
		return -ENOENT;
	}

	/*
	 * Not strictly correct - the memory may have multiple banks. We just
	 * use the first
	 */
	if (fdtdec_decode_region(blob, node, "reg", &base, &size)) {
		debug("%s: Failed to decode memory region %s\n", __func__,
		      mem);
		return -EINVAL;
	}

	snprintf(prop_name, sizeof(prop_name), "%s-offset%s", mem_type,
		 suffix);
	if (fdtdec_decode_region(blob, config_node, prop_name, &offset,
				 &offset_size)) {
		debug("%s: Failed to decode memory region '%s'\n", __func__,
		      prop_name);
		return -EINVAL;
	}

	*basep = base + offset;
	*sizep = offset_size;

	return 0;
}
#endif
