#ifndef __GUD_CONF_H__
#define __GUD_CONF_H__

#include "stm32f1xx.h"

#define USB_EP_CONTROL_SET (1 | EP_DIRECTION_OUT)
#define USB_EP_CONTROL_GET (1 | EP_DIRECTION_IN)

#define USB_USE_CDC
#define USB_IF_CDC_CONTROL 0
#define USB_IF_CDC_DATA 1
#define USB_EP_CDC_CONTROL 1
#define USB_EP_CDC_DATA 2
/// For use with CDC, we need 2 interfaces and 3 endpoints
#define NUM_INTERFACES (2)
#define NUM_ENDPOINTS (3)
#define NUM_CONFIGURATIONS (1)

#endif
