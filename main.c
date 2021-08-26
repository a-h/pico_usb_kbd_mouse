#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bsp/board.h"
#include "tusb.h"

void blink_led_task(void)
{
	const uint32_t interval_ms = 1000;
	static uint32_t start_ms = 0;
	static bool led_state = false;
	if(board_millis() - start_ms < interval_ms) {
		return;
	}
	start_ms += interval_ms;

	board_led_write(led_state);
	led_state = !led_state;
}

int main(void)
{
	board_init();

	printf("TinyUSB HID Example\r\n");

	tusb_init();
	while(1) {
		tuh_task();
		blink_led_task();
	}
}
