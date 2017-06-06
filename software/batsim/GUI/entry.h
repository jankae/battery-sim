#ifndef GUI_ENTRY_H_
#define GUI_ENTRY_H_

#include "widget.h"
#include "display.h"
#include "font.h"
#include "common.h"
#include "dialog.h"

#define ENTRY_INPUT_MAX_LENGTH      12

#define ENTRY_BG_COLOR				COLOR_BG_DEFAULT
#define ENTRY_FG_COLOR				COLOR_FG_DEFAULT
#define ENTRY_BORDER_COLOR			COLOR_FG_DEFAULT

typedef struct {
    widget_t base;
    int32_t *value;
    const int32_t *max;
    const int32_t *min;
    font_t font;
    const unit_t *unit;
    uint8_t length;
//    void (*changeCallback)(void);

    struct {
        uint8_t editing :1;
        uint8_t dotSet :1;
    } flags;
    uint8_t editPos;
    char inputString[ENTRY_INPUT_MAX_LENGTH];
} entry_t;

entry_t* entry_new(int32_t *value, const int32_t *max, const int32_t *min,
		font_t font, uint8_t digits, const unit_t * const unit);

//uint32_t entry_GetInputStringValue(entry_t *e, uint32_t multiplier);
//uint32_t entry_getIncrement(entry_t *e);
int32_t entry_constrainValue(entry_t *e, int32_t value);
void entry_setIncrement(entry_t *e, uint32_t inkrement);
void entry_draw(widget_t *w, coords_t offset);
void entry_input(widget_t *w, GUIEvent_t *ev);

#endif
