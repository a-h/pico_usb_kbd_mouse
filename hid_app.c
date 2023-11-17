#include "bsp/board.h"
#include "tusb.h"

static void process_kbd_report(hid_keyboard_report_t const *report);
static void process_mouse_report(hid_mouse_report_t const * report);

// tuh_hid_mount_cb is executed when a new device is mounted.
void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len)
{
	uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);
	if(itf_protocol == HID_ITF_PROTOCOL_NONE) {
		printf("Device with address %d, instance %d is not a keyboard or mouse.\r\n", dev_addr, instance);
		return;
	}
	const char* protocol_str[] = { "None", "Keyboard", "Mouse" };
	printf("Device with address %d, instance %d is a %s.\r\n", dev_addr, instance, protocol_str[itf_protocol]);

	// request to receive report
  	// tuh_hid_report_received_cb() will be invoked when report is available
  	if ( !tuh_hid_receive_report(dev_addr, instance) )
  	{
    		printf("Error: cannot request to receive report\r\n");
    	}
}

// tuh_hid_report_received_cb is executed when data is received from the keyboard or mouse.
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len)
{
	switch(tuh_hid_interface_protocol(dev_addr, instance)) {
	case HID_ITF_PROTOCOL_KEYBOARD:
		process_kbd_report((hid_keyboard_report_t const*) report);
		break;
	case HID_ITF_PROTOCOL_MOUSE:
		process_mouse_report((hid_mouse_report_t const*) report);
		break;
	}

	// request to receive next report
  	// tuh_hid_report_received_cb() will be invoked when report is available
  	if ( !tuh_hid_receive_report(dev_addr, instance) )
  	{
    		printf("Error: cannot request to receive report\r\n");
  	}
}

// tuh_hid_umount_cb is executed when a device is unmounted.
void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance)
{
	printf("Device with address %d, instance %d was unmounted.\r\n", dev_addr, instance);
}

static inline bool is_key_held(hid_keyboard_report_t const *report, uint8_t keycode)
{
	for(uint8_t i=0; i<6; i++) {
		if(report->keycode[i] == keycode) {
			return true;
		}
	}
	return false;
}

static uint8_t const keycode2ascii[128][2] = { HID_KEYCODE_TO_ASCII };

static void process_kbd_report(hid_keyboard_report_t const *report)
{
	static hid_keyboard_report_t prev_report = { 0, 0, {0} };
	
	for(uint8_t i=0; i<6; i++) {
		if(report->keycode[i]) {
			if(!is_key_held(&prev_report, report->keycode[i])) {
				bool const is_shift_pressed = report->modifier & (KEYBOARD_MODIFIER_LEFTSHIFT | KEYBOARD_MODIFIER_RIGHTSHIFT);
				uint8_t ch = keycode2ascii[report->keycode[i]][is_shift_pressed ? 1 : 0];
				printf("%c", ch);
				if(ch == '\r') {
					printf("\n");
				}
			}
		}
	}

	prev_report = *report;
}

static void process_mouse_report(hid_mouse_report_t const * report)
{
	static hid_mouse_report_t prev_report = { 0 };

	// Mouse position.
	printf("Mouse: (%d %d %d)", report->x, report->y, report->wheel);

	// Button state.
	uint8_t button_changed_mask = report->buttons ^ prev_report.buttons;
	if(button_changed_mask & report->buttons) {
		printf(" %c%c%c",
		       report->buttons & MOUSE_BUTTON_LEFT   ? 'L' : '-',
		       report->buttons & MOUSE_BUTTON_MIDDLE ? 'M' : '-',
		       report->buttons & MOUSE_BUTTON_RIGHT  ? 'R' : '-');
	}

	printf("\n");
}
