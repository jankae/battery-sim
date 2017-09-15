#ifndef GUI_ENTRY_H_
#define GUI_ENTRY_H_

#include "widget.h"
#include "display.h"
#include "font.h"
#include "common.h"

class Entry : public Widget {
public:
	Entry(int32_t *value, const int32_t *max, const int32_t *min, font_t font,
			uint8_t length, const unit_t *unit);
	~Entry();

	void setCallback(void (*cb)(Widget&)) {
		changeCallback = cb;
	}
private:
	int32_t constrainValue(int32_t val);
	uint32_t InputStringValue(uint32_t multiplier);

	void draw(coords_t offset) override;
	void input(GUIEvent_t *ev) override;

	static constexpr color_t Foreground = COLOR_FG_DEFAULT;
	static constexpr color_t Background = COLOR_BG_DEFAULT;
	static constexpr color_t Border = COLOR_FG_DEFAULT;

    int32_t *value;
    const int32_t *max;
    const int32_t *min;
    font_t font;
    const unit_t *unit;
    uint8_t length;
	bool editing;
	bool dotSet;
	void (*changeCallback)(Widget&);
	uint8_t editPos;
    char *inputString;
};

#endif
