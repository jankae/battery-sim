#include "Supply.h"

#include "pushpull.h"
#include "gui.h"
#include "file.h"

static const uint16_t imagedata[1024] = {
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
0x0000,0x0000,0x0000,0x0000,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0x0000,0x0000,0x0000,0x0000,
0x0000,0x0000,0xffff,0xffff,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xffff,0xffff,0x0000,0x0000,
0x0000,0x0000,0xffff,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0x0320,0x0320,0x0320,0x0320,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0x9532,0x1362,0x1362,0x9532,0xbdf7,0x9532,0x1362,0x1362,0x9532,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xffff,0x0000,0x0000,
0x0000,0xffff,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0x0320,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0x2ba5,0x7cef,0x7cef,0x2ba5,0xbdf7,0x2ba5,0x7cef,0x7cef,0x2ba5,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xffff,0x0000,
0x0000,0xffff,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0x0320,0x0b41,0x1b63,0x8510,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0x0b21,0xb5d6,0xb5d6,0x0b21,0xbdf7,0x0b21,0xb5d6,0xb5d6,0x0b21,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xffff,0x0000,
0x0000,0xffff,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdd7,0x84f0,0x1342,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0x0b21,0xb5d6,0xb5d6,0x0b21,0xbdf7,0x0b21,0xb5d6,0xb5d6,0x0b21,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xffff,0x0000,
0x0000,0xffff,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdd7,0x8510,0x1342,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0x2ba5,0x7cef,0x7cef,0x2ba5,0xbdf7,0x2ba5,0x7cef,0x7cef,0x2ba5,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xffff,0x0000,
0x0000,0xffff,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0x0320,0x0300,0x1b63,0x8510,0xbdf7,0x0320,0xbdf7,0xbdf7,0x9532,0x1362,0x1362,0x9532,0xbdf7,0x9532,0x1362,0x1362,0x9532,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xffff,0x0000,
0x0000,0xffff,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xffff,0x0000,
0x0000,0xffff,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xf800,0xf800,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xcc92,0xf0a2,0xf0a2,0xcc92,0xbdf7,0xcc92,0xf0a2,0xf0a2,0xcc92,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xffff,0x0000,
0x0000,0xffff,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xf800,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xe945,0xd3ef,0xd3ef,0xe945,0xbdf7,0xe945,0xd3ef,0xd3ef,0xe945,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xffff,0x0000,
0x0000,0xffff,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xf800,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xf841,0xbdb6,0xbdb6,0xf841,0xbdf7,0xf841,0xbdb6,0xbdb6,0xf841,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xffff,0x0000,
0x0000,0xffff,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xf800,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xf841,0xbdb6,0xbdb6,0xf841,0xbdf7,0xf841,0xbdb6,0xbdb6,0xf841,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xffff,0x0000,
0x0000,0xffff,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xf800,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xe945,0xd3ef,0xd3ef,0xe945,0xbdf7,0xe945,0xd3ef,0xd3ef,0xe945,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xffff,0x0000,
0x0000,0xffff,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xf800,0xf800,0xf800,0xbdf7,0xbdf7,0xf800,0xbdf7,0xbdf7,0xcc92,0xf0a2,0xf0a2,0xcc92,0xbdf7,0xcc92,0xf0a2,0xf0a2,0xcc92,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xffff,0x0000,
0x0000,0xffff,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xffff,0x0000,
0x0000,0xffff,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xffff,0x0000,
0x0000,0xffff,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xf800,0xf800,0xf800,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0x0320,0x0320,0x0320,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xffff,0x0000,
0x0000,0xffff,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0x0320,0x0320,0x0320,0x0320,0x0320,0x0320,0x0320,0xbdf7,0xbdf7,0xbdf7,0xffff,0x0000,
0x0000,0xffff,0xbdf7,0xbdf7,0xbdf7,0xf800,0xf800,0xf800,0xf800,0x0000,0xf800,0xf800,0xf800,0xf800,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0x0320,0x0320,0x0320,0x0320,0x0320,0x0320,0x0320,0x0320,0x0320,0xbdf7,0xbdf7,0xffff,0x0000,
0x0000,0xffff,0xbdf7,0xbdf7,0xbdf7,0xf800,0xf800,0xf800,0xf800,0x0000,0xf800,0xf800,0xf800,0xf800,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0x0320,0x0320,0x0320,0x0320,0x0320,0x0320,0x0320,0x0320,0x0320,0xbdf7,0xbdf7,0xffff,0x0000,
0x0000,0xffff,0xbdf7,0xbdf7,0xf800,0xf800,0xf800,0xf800,0xf800,0x0000,0xf800,0xf800,0xf800,0xf800,0xf800,0xbdf7,0xbdf7,0xbdf7,0x0320,0x0320,0x0320,0x0320,0x0320,0x0320,0x0320,0x0320,0x0320,0x0320,0x0320,0xbdf7,0xffff,0x0000,
0x0000,0xffff,0xbdf7,0xbdf7,0xf800,0xf800,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0xf800,0xf800,0xbdf7,0xbdf7,0xbdf7,0x0320,0x0320,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0320,0x0320,0xbdf7,0xffff,0x0000,
0x0000,0xffff,0xbdf7,0xbdf7,0xf800,0xf800,0xf800,0xf800,0xf800,0x0000,0xf800,0xf800,0xf800,0xf800,0xf800,0xbdf7,0xbdf7,0xbdf7,0x0320,0x0320,0x0320,0x0320,0x0320,0x0320,0x0320,0x0320,0x0320,0x0320,0x0320,0xbdf7,0xffff,0x0000,
0x0000,0xffff,0xbdf7,0xbdf7,0xbdf7,0xf800,0xf800,0xf800,0xf800,0x0000,0xf800,0xf800,0xf800,0xf800,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0x0320,0x0320,0x0320,0x0320,0x0320,0x0320,0x0320,0x0320,0x0320,0xbdf7,0xbdf7,0xffff,0x0000,
0x0000,0xffff,0xbdf7,0xbdf7,0xbdf7,0xf800,0xf800,0xf800,0xf800,0x0000,0xf800,0xf800,0xf800,0xf800,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0x0320,0x0320,0x0320,0x0320,0x0320,0x0320,0x0320,0x0320,0x0320,0xbdf7,0xbdf7,0xffff,0x0000,
0x0000,0xffff,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0x0320,0x0320,0x0320,0x0320,0x0320,0x0320,0x0320,0xbdf7,0xbdf7,0xbdf7,0xffff,0x0000,
0x0000,0x0000,0xffff,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xf800,0xf800,0xf800,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0x0320,0x0320,0x0320,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xffff,0x0000,0x0000,
0x0000,0x0000,0xffff,0xffff,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xbdf7,0xffff,0xffff,0x0000,0x0000,
0x0000,0x0000,0x0000,0x0000,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0x0000,0x0000,0x0000,0x0000,
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
};

static Image_t icon = { .width = 32, .height = 32, .data = imagedata };

static TaskHandle_t handle;

int32_t vol, source, sink;
static const fileEntry_t SupplyConfig[3] = {
		{ "voltage",&vol, PTR_INT32 },
		{ "source", &source, PTR_INT32 },
		{ "sink", &sink, PTR_INT32 },
};

static void Supply(void *unused);

static void Supply_Start(){
	xTaskCreate(Supply, "Supply", 600, NULL, 3, NULL);
}

void Supply_Init() {
	App_Register("Bench Supply", Supply_Start, icon);
}

static uint8_t loadDialog = 0;
static uint8_t saveDialog = 0;

static void Supply(void *unused) {
	handle = xTaskGetCurrentTaskHandle();

	int32_t voltage, current, power;

	int32_t setVoltage = 0, setMaxCurrent = 0, setMinCurrent = 0;
	int32_t null = 0;

	uint8_t on = 0;

	/* create GUI */
	/* Read back of current/voltage */
	Entry *eSetVoltage = new Entry(&setVoltage, &Limits.maxVoltage, &Limits.minVoltage, Font_Big, 7, &Unit_Voltage);
	Entry *eMaxCurrent = new Entry(&setMaxCurrent, &Limits.maxCurrent, &null, Font_Big, 7, &Unit_Current);
	Entry *eMinCurrent = new Entry(&setMinCurrent, &Limits.minCurrent, &null, Font_Big, 7, &Unit_Current);

	Label *lVol = new Label("Voltage set:", Font_Big);
	Label *lMax = new Label("Source I set:", Font_Big);
	Label *lMin = new Label("Sink I set:", Font_Big);

	Button *bLoad = new Button("Load", Font_Big, [](Widget &w) {
		loadDialog = 1;
		xTaskNotify(handle, SIGNAL_WAKEUP, eSetBits);
	});
	Button *bSave = new Button("Save", Font_Big, [](Widget &w) {
		saveDialog = 1;
		xTaskNotify(handle, SIGNAL_WAKEUP, eSetBits);
	});

	SevenSegment *sVol = new SevenSegment(&voltage, 20, 7, 5, 2, COLOR_DARKGREEN);
	SevenSegment *sCur = new SevenSegment(&current, 20, 7, 5, 3, COLOR_RED);
	SevenSegment *sPow = new SevenSegment(&power, 20, 7, 5, 2, COLOR_BLUE);

	Label *lV = new Label("V", Font_Big);
	lV->setColor(COLOR_DARKGREEN);
	Label *lA = new Label("A", Font_Big);
	lA->setColor(COLOR_RED);
	Label *lW = new Label("W", Font_Big);
	lW->setColor(COLOR_BLUE);

	Label *lOutput = new Label("Output", Font_Big);
	Label *lOn = new Label(6, Font_Big, Label::Orientation::CENTER);
	lOn->setText("OFF");
	lOn->setColor(COLOR_GRAY);

	Container *c= new Container(COORDS(280, 240));

	c->attach(sVol, COORDS(100, 0));
	c->attach(sCur, COORDS(100, 55));
	c->attach(sPow, COORDS(100, 110));

	c->attach(lV, COORDS(267, 3));
	c->attach(lA, COORDS(267, 58));
	c->attach(lW, COORDS(267, 113));

	c->attach(lOutput, COORDS(5, 9));
	c->attach(lOn, COORDS(5, 27));

	c->attach(bLoad, COORDS(5, 120));
	c->attach(bSave, COORDS(5, 150));

	c->attach(lVol, COORDS(0, 182));
	c->attach(eSetVoltage, COORDS(190, 180));
	c->attach(lMax, COORDS(0, 202));
	c->attach(eMaxCurrent, COORDS(190, 200));
	c->attach(lMin, COORDS(0, 222));
	c->attach(eMinCurrent, COORDS(190, 220));

	c->setPosition(COORDS(40, 0));

	desktop_AppStarted(Supply_Start, c);
	uint32_t signal;

	pushpull_AcquireControl();
	pushpull_SetAveraging(300);
	pushpull_SetDriveCurrent(200);
	pushpull_SetEnabled(0);
	pushpull_SetInternalResistance(0);

	while (1) {
		/* Update values */
		voltage = pushpull_GetBatteryVoltage() / 10000;
		current = pushpull_GetCurrent() / 1000;
		power = voltage * current / 1000;
		sVol->requestRedraw();
		sCur->requestRedraw();
		sPow->requestRedraw();

		pushpull_SetVoltage(setVoltage);
		pushpull_SetSourceCurrent(setMaxCurrent);
		pushpull_SetSinkCurrent(setMinCurrent);

		if (on != pushpull_GetEnabled()) {
			pushpull_SetEnabled(on);
			if (pushpull_GetEnabled()) {
				lOn->setText("ON");
				lOn->setColor(COLOR_RED);
			} else {
				lOn->setText("OFF");
				lOn->setColor(COLOR_GRAY);
			}
			on = pushpull_GetEnabled();
		}

		if (App_Handler(&signal, 300)) {
			/* received a notification */
			if (signal & SIGNAL_ONOFF_BUTTON) {
				on = !on;
			}
			if (signal & SIGNAL_PUSHPULL_UPDATE) {
				/* already handled in while(1) loop, nothing to do here */
			}
			if (loadDialog) {
				char filename[_MAX_LFN + 1];
				if (Dialog::FileChooser("Select Preset:", filename, "0:/",
						"SUP") == Dialog::Result::OK) {
					if (file_open(filename, FA_OPEN_EXISTING | FA_READ) == FR_OK
							&& file_ReadParameters(SupplyConfig, 3)
									== FILE_OK) {
						/* got all new parameters */
						on = 0;
						setVoltage = vol;
						setMaxCurrent = source;
						setMinCurrent = sink;
						file_close();
					} else {
						Dialog::MessageBox("Error", Font_Big,
								"Failed to read file", Dialog::MsgBox::OK, NULL,
								true);
					}
				}
				loadDialog = 0;
			}
			if (saveDialog) {
				char filename[_MAX_LFN + 1];
				if (Dialog::StringInput("Preset name:", filename, _MAX_LFN - 4)
						== Dialog::Result::OK) {
					/* add file extension */
					strcat(filename, ".SUP");
					vol = setVoltage;
					source = setMaxCurrent;
					sink = setMinCurrent;
					if (file_open(filename, FA_CREATE_ALWAYS | FA_WRITE)
							!= FR_OK) {
						Dialog::MessageBox("Error", Font_Big,
								"Failed to write file", Dialog::MsgBox::OK,
								NULL, true);
					} else {
						file_WriteParameters(SupplyConfig, 3);
						file_close();
					}
				}
				saveDialog = 0;
			}
		}
	}
}

