/*
 * Copyright (C) 2012-2014 Panasonic Corporation
 *   Author: Masahiro Yamada <yamada.m@jp.panasonic.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <linux/compiler.h>
#include <asm/arch/led.h>
#include <asm/arch/board.h>

void __weak bcu_init(void)
{
};
void sbc_init(void);
void sg_init(void);
void pll_init(void);
void pin_init(void);
void clkrst_init(void);

int board_postclk_init(void)
{
#ifdef CONFIG_SOC_INIT
	bcu_init();

	sbc_init();

	sg_init();

	uniphier_board_reset();

	pll_init();

	uniphier_board_init();

	led_write(B, 1, , );

	clkrst_init();

	led_write(B, 2, , );
#endif
	pin_init();

	led_write(B, 3, , );

	return 0;
}
