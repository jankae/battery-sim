#include "Settings.h"

static const uint16_t imagedata[1024] = {
0x0000,0x0000,0x0000,0x0000,0x4228,0x8cd2,0x94f3,0x7c30,0x39e7,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x18c3,0x7c10,0x73ef,0x0020,0x0000,
0x0000,0x0000,0x0000,0x0000,0x0841,0x8cb2,0xdf5c,0xe7be,0xceba,0x9513,0x4a8a,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x52cb,0x9d34,0xdf5c,0xd71c,0x73ef,0x0041,
0x0000,0x0000,0x0000,0x0000,0x0000,0x0841,0x7c50,0xbe58,0xc6ba,0xdf5d,0xbe58,0x632c,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x52eb,0xbe59,0xd73c,0xd73c,0xd71c,0x9d14,0x10a2,
0x0000,0x634d,0x2945,0x0000,0x0000,0x0000,0x0841,0x8451,0xc679,0xc679,0xcedb,0x9d34,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0861,0x7c30,0xd75c,0xd73c,0xd73c,0xbe59,0x4a8a,0x0000,
0x0000,0x9514,0x9d34,0x2124,0x0000,0x0000,0x0000,0x52ca,0xb618,0xbe79,0xbe79,0xa575,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x1082,0x6b8e,0xe79e,0xe79e,0xd75c,0xd71c,0x73cf,0x0000,0x0000,
0x0000,0x8cb2,0xd73c,0x9d34,0x18c3,0x0000,0x18e3,0x8cd3,0xc6ba,0xbe79,0xbe79,0x9d34,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x1082,0x4a8a,0xe79e,0xe79e,0xe79e,0x9514,0x6bcf,0x1082,0x0000,0x0000,
0x0000,0x7c11,0xc679,0xd73c,0x9d34,0x5b2c,0x94f3,0xe77d,0xc679,0xbe79,0xbe79,0xa5b6,0x0882,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0861,0x5b0c,0xbe59,0xdf7d,0x636d,0x6bae,0x31e7,0x0000,0x0000,0x0000,0x0000,
0x0000,0x4249,0xadf7,0xc679,0xd73c,0xdf7d,0xe79e,0xc699,0xbe79,0xbe59,0xbe59,0xc679,0x52cb,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0841,0x634d,0x9d55,0xe7de,0x636d,0x52ca,0x1082,0x0000,0x0000,0x0000,0x0000,0x0000,
0x0000,0x0020,0x8471,0xbe79,0xbe79,0xc699,0xbe79,0xbe59,0xbe59,0xbe59,0xbe58,0xc6ba,0x8cb2,0x0841,0x0000,0x0000,0x0000,0x0000,0x0000,0x0841,0x636d,0x9d34,0xdf7d,0x636d,0x52aa,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
0x0000,0x0000,0x4228,0x9d34,0xbe79,0xbe59,0xbe59,0xbe59,0xbe58,0xbe58,0xbe38,0xbe38,0xcefb,0x7c30,0x0041,0x0000,0x0000,0x0000,0x0841,0x636d,0x94f3,0xd71b,0x636d,0x4aaa,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
0x0000,0x0000,0x0000,0x4248,0x7c30,0x8cb2,0x8cd2,0xa575,0xb617,0xbe38,0xbe38,0xbe38,0xbe38,0xcefb,0xa2c5,0xa2c5,0x0000,0x0861,0x636d,0x8cb2,0xc699,0x636d,0x4a8a,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
0x0000,0x0000,0x0000,0x0000,0x0000,0x0020,0x0841,0x18e3,0x52eb,0x7c30,0xb617,0xb618,0xb618,0x9aa4,0x92a4,0xdcec,0xc409,0x636d,0x8492,0xb617,0x636d,0x4a8a,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0882,0x6bae,0xb5f7,0xb617,0x7a24,0x79e2,0xdcab,0xe50c,0xc409,0xa596,0x636d,0x4a8a,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0861,0x6b8d,0xadd6,0x8b49,0x7181,0x79e2,0xcc08,0xa2e4,0xa2c5,0x4a69,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0xa2c5,0x7a44,0x8223,0x81e2,0x71a1,0x71a1,0xb304,0x8a02,0x81c1,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0xa2c5,0x9a83,0xf5cf,0xf5ae,0x8a23,0x79c2,0x6981,0x7181,0x8a02,0xb325,0x81c1,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0xa2c5,0xb345,0xe52d,0xf5cf,0xf5cf,0xc3c7,0x8a23,0x79c2,0x6981,0x71a1,0x8a02,0x79c0,0x0020,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0800,0xa2c5,0xb345,0xdcec,0x8a02,0x81c1,0xd46a,0xf5cf,0xab04,0x8202,0x7266,0x7265,0x79c0,0x7c30,0x6b8e,0x0041,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0820,0xa2c5,0xb366,0xdcec,0x8a02,0x81c1,0x8a22,0xbb86,0xab04,0xb324,0x7a43,0x7c30,0x7c30,0x7c30,0x9513,0xc6ba,0x638d,0x0861,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
0x0000,0x0000,0x0000,0x0000,0x0000,0x0820,0xa2c4,0xb366,0xdccb,0x8a02,0x81c1,0x9243,0xc3e7,0xa2a3,0x79a1,0x9a83,0x4920,0x7c30,0x8cd2,0x8cd2,0xa555,0xb5d7,0xc6ba,0x73ef,0x10c3,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
0x0000,0x0000,0x0000,0x0000,0x0820,0xa2a4,0xb366,0xdcab,0x89e2,0x81c1,0x9243,0xc3c7,0xa2a3,0x7981,0x8202,0x79c0,0x0000,0x0000,0x52ca,0x8cd2,0xa555,0xb5d7,0xb5d7,0xc679,0x8491,0x31c6,0x0020,0x0000,0x0000,0x0000,0x0000,0x0000,
0x0000,0x0000,0x0000,0x1040,0x9aa4,0xb365,0xd48a,0x89e2,0x81c1,0x9243,0xbba7,0xa2a3,0x7981,0x8202,0x79c0,0x0000,0x0000,0x0000,0x0000,0x52aa,0x9d34,0xb5d7,0xb5d7,0xb5d7,0xbe38,0xb5f7,0x7c50,0x5b0c,0x2145,0x0000,0x0000,0x0000,
0x0000,0x0000,0x1040,0x9aa4,0xb345,0xd46a,0x81e2,0x81c1,0x9243,0xbb86,0x9a83,0x7981,0x8a02,0x79c0,0x0000,0x0000,0x0000,0x0000,0x0000,0x0020,0x52eb,0xadb6,0xb5d7,0xb5d7,0xb5d7,0xb5d7,0xb617,0xc679,0xadb6,0x6bce,0x18c3,0x0000,
0x0000,0x1860,0x9283,0xb345,0xcc29,0x81e2,0x81c1,0x9243,0xbb66,0x9a83,0x7181,0x8a02,0x79c0,0x0800,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0841,0x5b0c,0xadb6,0xb5d7,0xb5d7,0xb5d7,0x94d3,0x7c0f,0x94f3,0xc679,0x7c50,0x0861,
0x1860,0x9263,0xb345,0xc3c8,0x81e1,0x81c1,0x9243,0xbb65,0x9a83,0x7181,0x8a02,0x79c0,0x0820,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x10a2,0x7c30,0xb5d7,0xb5d7,0x6b8e,0x2124,0x0841,0x2965,0x73ef,0xbe59,0x636d,
0x8a42,0xaae4,0xc3c7,0x81c1,0x81c1,0x9243,0xb345,0x9a83,0x7181,0x8a02,0x79c0,0x0820,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x5aeb,0xb5d7,0xadd6,0x31a6,0x0000,0x0000,0x0000,0x3a28,0xad96,0x7c50,
0x79c0,0xb325,0xab25,0x81c1,0x9243,0xb325,0x9a83,0x7181,0x8a22,0x79c0,0x0820,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x31c7,0xa555,0xa595,0x1904,0x0000,0x0000,0x0000,0x2985,0xa555,0x8451,
0x4100,0x79c0,0xbb86,0xbb66,0xb304,0x9a63,0x7161,0x8a22,0x79c0,0x0820,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0841,0x73ae,0xc699,0x4a8a,0x0000,0x0000,0x0000,0x52eb,0xadd6,0x73cf,
0x0000,0x4100,0x79c0,0xb304,0xaae4,0x7161,0x8a22,0x79c0,0x0820,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x4249,0xa575,0xadd7,0x5b2c,0x4a69,0x5b0c,0xa555,0x94d3,0x4249,
0x0000,0x0000,0x4100,0x79c0,0xaae4,0xa2c3,0x79c0,0x0820,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x4269,0x8491,0xb5d7,0xb5d7,0xadd6,0x8450,0x4a8a,0x0000,
0x0000,0x0000,0x0000,0x30c0,0x79c0,0x79c0,0x0820,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x2104,0x4249,0x52aa,0x4249,0x18e3,0x0000,0x0000,
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
};

static uint8_t calibrate = 0;
static uint8_t fileDialog = 0;
static Image_t icon = { .width = 32, .height = 32, .data = imagedata };

static xTaskHandle hTask;

extern widget_t *topWidget;

static void settings_Start(){
	xTaskCreate(settings, "Settings", 2000, NULL, 3, NULL);
}

void settings_Init() {
	App_Register("Settings", settings_Start, icon);
}

static void calibrateTouch() {
	calibrate = 1;
}

static void fileChooser() {
	fileDialog = 1;
}

void settings(void *unused) {
	hTask = xTaskGetCurrentTaskHandle();

	/* create GUI */
	container_t *c = container_new(COORDS(280, 240));
	button_t *b = button_new("Calibrate", Font_Big, 0, calibrateTouch);
	button_t *file = button_new("File", Font_Big, 0, fileChooser);


	container_attach(c, (widget_t*) b, COORDS(40, 20));
	container_attach(c, (widget_t*) file, COORDS(40, 60));
	c->base.position.x = 40;

	desktop_AppStarted(settings_Start, (widget_t*) c);
	uint32_t signal;

	while (1) {
		if (App_Handler(&signal, 100)) {
			/* no special signals handled in this app */
		}

		if(calibrate) {
			touch_Calibrate();
			widget_RequestRedrawFull(topWidget);
			desktop_Draw();
			calibrate = 0;
		}

		if (fileDialog) {
			char result[15];
			if (dialog_FileChooser("Filename", result, "0:/", NULL)
					== DIALOG_RESULT_OK) {
				dialog_MessageBox("Chosen:", Font_Big, result, MSG_OK, NULL);
			} else {
				dialog_MessageBox("Aborted", Font_Big, "No file chosen", MSG_OK,
						NULL);
			}
			fileDialog = 0;
		}
	}
}
