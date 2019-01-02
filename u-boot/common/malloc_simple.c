/*
 * Simple malloc implementation
 *
 * Copyright (c) 2014 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <malloc.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

void *malloc_simple(size_t bytes)
{
	ulong new_ptr;
	void *ptr;

	new_ptr = gd->malloc_ptr + bytes;
	if (new_ptr > gd->malloc_limit)
		panic("Out of pre-reloc memory");
	ptr = map_sysmem(gd->malloc_base + gd->malloc_ptr, bytes);
	gd->malloc_ptr = ALIGN(new_ptr, sizeof(new_ptr));
	return ptr;
}

#ifdef CONFIG_SYS_MALLOC_SIMPLE
void *calloc(size_t nmemb, size_t elem_size)
{
	size_t size = nmemb * elem_size;
	void *ptr;

	ptr = malloc(size);
	memset(ptr, '\0', size);

	return ptr;
}
#endif
