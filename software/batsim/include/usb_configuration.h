/*
 * usb_configuration.h
 *
 *  Created on: Mar 14, 2017
 *      Author: jan
 */

#ifndef USER_USB_CONFIGURATION_H_
#define USER_USB_CONFIGURATION_H_

#include "generic_usb_driver.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define STM32_UUID ((uint32_t *)0x1FFF7A10)

#define CONF_USB_VID 0x0483
#define CONF_USB_PID 0x1DF8

uint8_t* GetStringDescriptor(uint8_t index);

static volatile GUD_GenericUsbDriverTypeDef gud;
static GUD_CallbacksTypeDef callbacks = { GetStringDescriptor };
static char serial[25];

extern PCD_HandleTypeDef hpcd_USB_OTG_FS;

void usb_configuration_init(void);
void usb_send(uint8_t *data, uint8_t length);
GUD_StatusTypeDef usb_getState(void);

#endif /* USER_USB_CONFIGURATION_H_ */
