/*
 * usb_configuration.c
 *
 *  Created on: Mar 14, 2017
 *      Author: jan
 */

#include "gud_conf.h"
#include "usb_configuration.h"
#include "gui.h"
#include "common.h"

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

void usb_send(uint8_t *data, uint16_t length){
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
#define COMMAND_ENTRIES			200
uint8_t commandBuffer[3*COMMAND_ENTRIES+1];
uint16_t cmdCnt = 0;
void usb_DisplayCommand(uint8_t command, uint16_t value) {
	commandBuffer[cmdCnt * 3 + 0] = command;
	commandBuffer[cmdCnt * 3 + 1] = value >> 8;
	commandBuffer[cmdCnt * 3 + 2] = value & 0xff;
	cmdCnt++;
	if (cmdCnt >= COMMAND_ENTRIES) {
		/* buffer is full, send now */
		commandBuffer[COMMAND_ENTRIES * 3] = '\n';
		usb_send(commandBuffer, COMMAND_ENTRIES * 3 + 1);
		cmdCnt = 0;
	}
}
void usb_DisplayFlush(void){
	if(cmdCnt>0) {
		/* send partial buffer now */
		commandBuffer[cmdCnt * 3] = '\n';
		usb_send(commandBuffer, cmdCnt * 3 + 1);
		cmdCnt = 0;
	}
}

void GUD_DataReceivedCallback(uint8_t ep_num, uint8_t* buffer,
		uint16_t length) {
	if(length==5) {
		if(buffer[0]==1) {
			/* touch press event */
			GUIEvent_t ev;
			ev.type = GUI_TOUCH_PRESSED;
			ev.pos.x = (buffer[1] << 8) + buffer[2];
			ev.pos.y = (buffer[3] << 8) + buffer[4];
			gui_SendEvent(&ev);
		} else if(buffer[0]==2) {
			/* touch press event */
			GUIEvent_t ev;
			ev.type = GUI_TOUCH_RELEASED;
			ev.pos.x = (buffer[1] << 8) + buffer[2];
			ev.pos.y = (buffer[3] << 8) + buffer[4];
			gui_SendEvent(&ev);
		}
	}
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





