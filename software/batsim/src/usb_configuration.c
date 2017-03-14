/*
 * usb_configuration.c
 *
 *  Created on: Mar 14, 2017
 *      Author: jan
 */

#include "gud_conf.h"
#include "usb_configuration.h"

unsigned char usb_buffer[256];

void usb_configuration_init(void) {

	/// Handle, Vendor ID, Product ID, Revision, Callbacks, Speed
	GUD_Init(&gud, CONF_USB_VID, CONF_USB_PID, 0x0200, &callbacks,
			USB_FULL_SPEED);

	/// Setup CDC interface
	gud_setup_cdc(&gud);
	/// Finally setup everything
	GUD_Setup(&hpcd_USB_OTG_FS, &gud);

	/// Init configuration #1
	GUD_ConfigurationInitTypeDef conf;
	conf.max_power = 50; // = 100mA/2
	conf.self_powered = 1;
	conf.remote_wakeup = 0;
	conf.num_interfaces = NUM_INTERFACES;

	/// Register configuration
	GUD_RegisterConfiguration(&conf);
	/// Start driver
	GUD_Start();

}

void usb_send(uint8_t *data, uint8_t length){
	GUD_SendData(USB_EP_CDC_DATA, data, length);
	while (usb_getState() != GUD_OK)
		;
}

GUD_StatusTypeDef usb_getState(void){
	if (gud.ep_in[USB_EP_CDC_DATA & 0x7F].state != IDLE) {
		return GUD_BUSY;
	} else {
		return GUD_OK;
	}
}

void GUD_DataReceivedCallback(uint8_t ep_num, uint8_t* buffer,
		uint16_t length) {
//	GUD_SendData(ep_num, buffer, length);
}

uint8_t* GetStringDescriptor(uint8_t index) {
	switch(index) {
//	case GUD_MANUFACTURER_STRING_INDEX :
//		return (uint8_t*) "";
	case GUD_PRODUCT_STRING_INDEX :
		return (uint8_t*) "batterySimulator";
	case GUD_SERIAL_STRING_INDEX :
		return (uint8_t*) serial;
	default :
		return (uint8_t*) "Unknown";
	}
}





