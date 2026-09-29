#include <zephyr/ztest.h>
#include <zephyr/drivers/gpio.h>

#define LED_BLUE_NODE DT_ALIAS(led0)
#define LED_GREEN_NODE DT_ALIAS(led1)
#define LED_RED_NODE   DT_ALIAS(led2)

static const struct gpio_dt_spec led_blue = GPIO_DT_SPEC_GET(LED_BLUE_NODE, gpios);
static const struct gpio_dt_spec led_green = GPIO_DT_SPEC_GET(LED_GREEN_NODE, gpios);
static const struct gpio_dt_spec led_red   = GPIO_DT_SPEC_GET(LED_RED_NODE, gpios);

extern struct ztest_suite_node _ztest_suite_node_list_start[];
extern struct ztest_suite_node _ztest_suite_node_list_end[];

uint32_t count_fails(void){
	/* Fail-Count for all Suites */
    uint32_t total_fail = 0;

    for (struct ztest_suite_node *ptr = _ztest_suite_node_list_start;
         ptr < _ztest_suite_node_list_end; ++ptr) {
        total_fail += ptr->stats->fail_count;
    }
	return total_fail;
}

void test_main(void)
{
	ztest_run_all(NULL, false, 1, 1);

	/* Check that all the suites in this binary ran at least once. */
	ztest_verify_all_test_suites_ran();

	//TC_SUMMARY_PRINT("FAILS: %d\n", count_fails());

	gpio_pin_configure_dt(&led_green, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&led_red,   GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&led_blue,   GPIO_OUTPUT_INACTIVE);

	gpio_pin_set_dt(&led_blue,   1);
	if (count_fails() > 0) {
        gpio_pin_set_dt(&led_red,   1);
        gpio_pin_set_dt(&led_green, 0);
    } else {
        gpio_pin_set_dt(&led_green, 1);
        gpio_pin_set_dt(&led_red,   0);
    }
}