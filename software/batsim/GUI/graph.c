#include "graph.h"

graph_t* graph_new(int32_t *values, uint16_t num, uint16_t height, color_t color, unit_t *unit) {
	graph_t* g = pvPortMalloc(sizeof(graph_t));
	if (!g) {
		/* malloc failed */
		return NULL;
	}
	/* initialize common widget values */
	widget_init((widget_t*) g);
	/* set widget functions */
	g->base.func.draw = graph_draw;
	g->base.func.input = graph_input;

	g->values = values;
	g->color = color;
	g->base.size.x = num + 2;
	g->base.size.y = height;
	g->unit = unit;

//	g->base.flags.selectable = 0;
	return g;
}

void graph_draw(widget_t *w, coords_t offset) {
	graph_t* g = (graph_t*) w;
    /* calculate corners */
    coords_t upperLeft = offset;
	coords_t lowerRight = upperLeft;
	lowerRight.x += g->base.size.x - 1;
	lowerRight.y += g->base.size.y - 1;

	/* draw rectangle */
	display_SetForeground(GRAPH_BORDER_COLOR);
	display_Rectangle(upperLeft.x, upperLeft.y, lowerRight.x, lowerRight.y);

	/* find min/max value */
	int32_t min = INT32_MAX;
	int32_t max = INT32_MIN;
	uint16_t num = w->size.x - 2;
	uint16_t i;
	for (i = 0; i < num; i++) {
		if (g->values[i] < min)
			min = g->values[i];
		if (g->values[i] > max)
			max = g->values[i];
	}

	/* draw graph data */
	display_SetForeground(g->color);
	int32_t lastY = common_Map(g->values[0], min, max, lowerRight.y - 1,
			upperLeft.y + 1);
	for (i = 1; i < num; i++) {
		int32_t Y = common_Map(g->values[i], min, max, lowerRight.y - 1,
				upperLeft.y + 1);
		display_Line(upperLeft.x + i, lastY, upperLeft.x + 1 + i, Y);
		lastY = Y;
	}

	/* display min/max data */
	display_SetFont(Font_Medium);
	char buf[11];
	common_StringFromValue(buf, 7, max, g->unit);
	display_String(lowerRight.x - 41, upperLeft.y + 2, buf);
	common_StringFromValue(buf, 7, min, g->unit);
	display_String(lowerRight.x - 41, lowerRight.y - 8, buf);
}

void graph_input(widget_t *w, GUIEvent_t *ev) {
    /* graph doesn't handle any input */
	return;
}

void graph_NewColor(graph_t *g, color_t color) {
	g->color = color;
	widget_RequestRedraw((widget_t*) g);
}

void graph_NewData(graph_t *g, int32_t *data) {
	g->values = data;
	widget_RequestRedrawFull((widget_t*) g);
}
