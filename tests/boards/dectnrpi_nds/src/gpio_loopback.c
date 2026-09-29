/*
 * Copyright (c) 2026 Deveritec GmbH
 * SPDX-License-Identifier: Apache-2.0
 *
 * GPIO loopback test for the Raspberry Pi header pins of dectnrpi_nds.
 * Pairs are defined in boards/dectnrpi_nds_nrf9151_ns.overlay (zephyr,user loopback-out/in-gpios).
 */

#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/ztest.h>

#define LB_NODE DT_PATH(zephyr_user)

#define LB_PAIRS DT_PROP_LEN(LB_NODE, loopback_out_gpios)

BUILD_ASSERT(LB_PAIRS == DT_PROP_LEN(LB_NODE, loopback_in_gpios),
	     "loopback-out-gpios and loopback-in-gpios must have the same length");

static const struct gpio_dt_spec lb_out[] = {
	DT_FOREACH_PROP_ELEM_SEP(LB_NODE, loopback_out_gpios, GPIO_DT_SPEC_GET_BY_IDX, (,))
};

static const struct gpio_dt_spec lb_in[] = {
	DT_FOREACH_PROP_ELEM_SEP(LB_NODE, loopback_in_gpios, GPIO_DT_SPEC_GET_BY_IDX, (,))
};

static void *gpio_loopback_setup(void)
{
	for (size_t i = 0; i < LB_PAIRS; i++) {
		zassert_true(gpio_is_ready_dt(&lb_out[i]), "out pair %u: GPIO port not ready",
			     (unsigned int)i);
		zassert_true(gpio_is_ready_dt(&lb_in[i]), "in pair %u: GPIO port not ready",
			     (unsigned int)i);
	}
	return NULL;
}

/* Returns the number of failed checks for this pair. */
static int check_pair(const struct gpio_dt_spec *drv, const struct gpio_dt_spec *rd,
		      size_t idx, const char *dir)
{
	int ret;
	int failures = 0;

	ret = gpio_pin_configure_dt(rd, GPIO_INPUT | GPIO_PULL_DOWN);
	if (ret) {
		printk("pair %u (%s): reader configure failed (%d)\n", (unsigned int)idx, dir, ret);
		return 1;
	}

	ret = gpio_pin_configure_dt(drv, GPIO_OUTPUT_LOW);
	if (ret) {
		printk("pair %u (%s): driver configure failed (%d)\n", (unsigned int)idx, dir, ret);
		return 1;
	}

	for (int level = 0; level <= 1; level++) {
		ret = gpio_pin_set_dt(drv, level);
		if (ret) {
			printk("pair %u (%s): set failed (%d)\n", (unsigned int)idx, dir, ret);
			failures++;
			continue;
		}

		k_busy_wait(100);

		int val = gpio_pin_get_dt(rd);

		if (val != level) {
			printk("FAIL pair %u (%s): driven %d, read %d (check wiring)\n",
			       (unsigned int)idx, dir, level, val);
			failures++;
		}
	}

	if (failures == 0) {
		printk("pass pair %u (%s)\n", (unsigned int)idx, dir);
	}

	(void)gpio_pin_configure_dt(drv, GPIO_DISCONNECTED);
	(void)gpio_pin_configure_dt(rd, GPIO_DISCONNECTED);

	return failures;
}

ZTEST(gpio_loopback, test_loopback_out_to_in)
{
	int failures = 0;

	for (size_t i = 0; i < LB_PAIRS; i++) {
		failures += check_pair(&lb_out[i], &lb_in[i], i, "out->in");
	}

	zassert_equal(0, failures, "%d check(s) failed, see pair list above", failures);
}

ZTEST(gpio_loopback, test_loopback_in_to_out)
{
	int failures = 0;

	for (size_t i = 0; i < LB_PAIRS; i++) {
		failures += check_pair(&lb_in[i], &lb_out[i], i, "in->out");
	}

	zassert_equal(0, failures, "%d check(s) failed, see pair list above", failures);
}

ZTEST_SUITE(gpio_loopback, NULL, gpio_loopback_setup, NULL, NULL, NULL);