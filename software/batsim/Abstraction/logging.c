#include "logging.h"
#include "display.h"

#include <stdarg.h>

void Log_CritError(const char *filename, uint16_t line, const char *fmt, ...) {
	/* Something bad happened */
	display_SetForeground(COLOR_RED);
	display_SetBackground(COLOR_BLACK);
	display_Clear();
	display_SetFont(Font_Big);
	display_String(0, 0, "CRITICAL ERROR:");
	display_SetForeground(COLOR_WHITE);
	display_SetFont(Font_Medium);
	display_String(0, 16, filename);
	char buffer[41];
	snprintf(buffer, sizeof(buffer), "Line: %u", line);
	display_String(0, 24, buffer);

	va_list arp;
	va_start(arp, fmt);
	snprintf(buffer, sizeof(buffer), fmt, arp);
	va_end(arp);

	display_String(0, 40, buffer);

	while(1);
}
