#include "progressbar.h"

progressbar_t* progressbar_new(coords_t size) {
	progressbar_t* p = pvPortMalloc(sizeof(progressbar_t));
	if (!p) {
		/* malloc failed */
		return NULL;
	}
    /* initialize common widget values */
    widget_init((widget_t*) p);
    /* set widget functions */
    p->base.func.draw = progressbar_draw;
    p->base.func.input = progressbar_input;
    /* a progressbar can't have any children */
    p->base.func.drawChildren = NULL;

    p->base.size = size;

    p->state = 0;

    return p;
}

void progressbar_SetState(progressbar_t *p, uint8_t state) {
	if (p->state != state) {
		p->state = state;
		widget_RequestRedraw((widget_t*) p);
	}
}

void progressbar_draw(widget_t *w, coords_t offset) {
	progressbar_t *p = (progressbar_t*) w;
    /* calculate corners */
    coords_t upperLeft = offset;
    coords_t lowerRight = upperLeft;
    lowerRight.x += p->base.size.x - 1;
    lowerRight.y += p->base.size.y - 1;

    /* draw outline */
    display_SetForeground(PROGRESSBAR_BORDER_COLOR);
    display_Rectangle(upperLeft.x, upperLeft.y, lowerRight.x, lowerRight.y);

    /* calculate end of bar */
    uint16_t end = common_Map(p->state, 0, 100, 1, w->size.x - 2);

    /* draw the bar */
    display_SetForeground(PROGRESSBAR_BAR_COLOR);
    display_RectangleFull(upperLeft.x + 1, upperLeft.y + 1, upperLeft.x + end, lowerRight.y - 1);

    /* draw empty space right of the bar (in case of receding progress) */
	if (end < w->size.x - 2) {
		display_SetForeground(PROGRESSBAR_BG_COLOR);
		display_RectangleFull(upperLeft.x + end + 1, upperLeft.y + 1,
				lowerRight.x - 1, lowerRight.y - 1);
	}
}

void progressbar_input(widget_t *w, GUIEvent_t *ev) {
    /* progressbar doesn't handle any input */
	return;
}
