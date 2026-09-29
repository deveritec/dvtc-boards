/*
 * Copyright (c) 2026 Deveritec GmbH
 * SPDX-License-Identifier: Apache-2.0
 *
 * LED and button test for dectnrpi_nds. Operator interaction required:
 * each LED is switched on in turn and confirmed by its paired button:
 *   led0 <-> button0 (SW0), led1 <-> button1 (SW1), led2 <-> button2 (SW2)
 */

#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

#define PROMPT_TIMEOUT_MS 15000
#define DEBOUNCE_MS       50

static const struct gpio_dt_spec leds[] = {
	GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios),
	GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios),
	GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios),
};

static const struct gpio_dt_spec buttons[] = {
	GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios),
	GPIO_DT_SPEC_GET(DT_ALIAS(sw1), gpios),
	GPIO_DT_SPEC_GET(DT_ALIAS(sw2), gpios),
};

static void *leds_buttons_setup(void)
{
	for (size_t i = 0; i < ARRAY_SIZE(leds); i++) {
		zassert_true(gpio_is_ready_dt(&leds[i]), "led%u: GPIO port not ready",
			     (unsigned int)i);
		zassert_equal(0, gpio_pin_configure_dt(&leds[i], GPIO_OUTPUT_INACTIVE),
			      "led%u: configure failed", (unsigned int)i);
	}

	for (size_t i = 0; i < ARRAY_SIZE(buttons); i++) {
		zassert_true(gpio_is_ready_dt(&buttons[i]), "button%u: GPIO port not ready",
			     (unsigned int)i);
		zassert_equal(0, gpio_pin_configure_dt(&buttons[i], GPIO_INPUT),
			      "button%u: configure failed", (unsigned int)i);
	}

	return NULL;
}

/* Returns true if the button is pressed (debounced) within timeout_ms. */
static bool wait_pressed(const struct gpio_dt_spec *btn, int timeout_ms)
{
	int64_t end = k_uptime_get() + timeout_ms;

	while (k_uptime_get() < end) {
		if (gpio_pin_get_dt(btn) > 0) {
			k_msleep(DEBOUNCE_MS);
			if (gpio_pin_get_dt(btn) > 0) {
				return true;
			}
		}
		k_msleep(10);
	}

	return false;
}

/* Returns true if the button is released within timeout_ms. */
static bool wait_released(const struct gpio_dt_spec *btn, int timeout_ms)
{
	int64_t end = k_uptime_get() + timeout_ms;

	while (k_uptime_get() < end) {
		if (gpio_pin_get_dt(btn) == 0) {
			k_msleep(DEBOUNCE_MS);
			return true;
		}
		k_msleep(10);
	}

	return false;
}

ZTEST(led_button_tests, test_each_led_confirmed_by_its_button)
{
	int failures = 0;

	for (size_t i = 0; i < ARRAY_SIZE(leds); i++) {
		zassert_equal(0, gpio_pin_set_dt(&leds[i], 1), "led%u: set failed",
			      (unsigned int)i);
		printk("LED%u ON: press button%u (SW%u) to confirm within %d s\n",
		       (unsigned int)i, (unsigned int)i, (unsigned int)i,
		       PROMPT_TIMEOUT_MS / 1000);

		bool confirmed = wait_pressed(&buttons[i], PROMPT_TIMEOUT_MS);

		(void)gpio_pin_set_dt(&leds[i], 0);

		if (confirmed) {
			(void)wait_released(&buttons[i], PROMPT_TIMEOUT_MS);
			printk("pass LED%u / button%u\n", (unsigned int)i, (unsigned int)i);
		} else {
			printk("FAIL LED%u / button%u: no confirmation within %d ms\n",
			       (unsigned int)i, (unsigned int)i, PROMPT_TIMEOUT_MS);
			failures++;
		}
	}

	zassert_equal(0, failures, "%d LED/button check(s) failed, see output above", failures);
}

ZTEST_SUITE(led_button_tests, NULL, leds_buttons_setup, NULL, NULL, NULL);