#include "dialog.h"

#include "logging.h"

#define INPUT_DIALOG_LENGTH		10

extern SemaphoreHandle_t fileAccess;
extern TaskHandle_t GUIHandle;

struct {
	window_t *window;
	union {
		struct {
			void (*cb)(DialogResult_t);
		} msgbox;
		struct {
			SemaphoreHandle_t dialogDone;
			uint8_t OKclicked;
		} fileChooser;
		struct {
			SemaphoreHandle_t dialogDone;
			uint8_t OKclicked;
			char *string;
			label_t *lString;
			uint8_t pos;
			uint8_t maxLength;
		} StringInput;
	};
} dialog;

static void MessageBoxButton(widget_t *source) {
	button_t *b = (button_t*) source;
	DialogResult_t res = DIALOG_RESULT_ERR;
	/* find which button has been pressed */
	if(!strcmp(b->name, "OK")) {
		res = DIALOG_RESULT_OK;
	} else if(!strcmp(b->name, "ABORT")) {
		res = DIALOG_RESULT_ABORT;
	}

	if (dialog.msgbox.cb)
		dialog.msgbox.cb(res);

	// TODO does this always work? (might result in null pointer exception due to deleting
	// widgets while working on them)
	window_destroy((window_t*) dialog.window);

//	/* destroy dialog */
//	GUIEvent_t ev = { .type = EVENT_WINDOW_CLOSE, .w = dialog.window };
//	gui_SendEvent(&ev);
}

void dialog_MessageBox(const char * const title, const char * const msg,
		MsgBox_t type, void (*cb)(DialogResult_t)){
	/* check pointers */
	if (!title || !msg) {
		return;
	}
	/* create dialog window and elements */
	textfield_t *text = textfield_new(msg, Font_Medium, COORDS(300, 180));
	if (!text) {
		return;
	}
	/* calculate window size */
	coords_t windowSize = text->base.size;
	if (windowSize.x < 132) {
		windowSize.x = 136;
	} else {
		windowSize.x += 4;
	}
	windowSize.y += 50;
	window_t *w = window_new(title, Font_Big, windowSize);
	container_t *c = container_new(window_GetAvailableArea(w));
	container_attach(c, (widget_t*) text, COORDS(1, 2));
	switch (type) {
	case MSG_OK: {
		button_t *bOK = button_new("OK", Font_Big, 65, MessageBoxButton);
		container_attach(c, (widget_t*) bOK,
				COORDS((c->base.size.x - bOK->base.size.x) / 2,
						c->base.size.y - bOK->base.size.y - 1));
	}
		break;
	case MSG_ABORT_OK: {
		button_t *bOK = button_new("OK", Font_Big, 65, MessageBoxButton);
		button_t *bAbort = button_new("ABORT", Font_Big, 65, MessageBoxButton);
		container_attach(c, (widget_t*) bAbort,
				COORDS(c->base.size.x / 2 - bAbort->base.size.x - 1,
						c->base.size.y - bAbort->base.size.y - 1));
		container_attach(c, (widget_t*) bOK,
				COORDS(c->base.size.x / 2 + 1,
						c->base.size.y - bOK->base.size.y - 1));

	}
		break;
	}

	dialog.window = w;
	dialog.msgbox.cb = cb;

	widget_Select((widget_t*) w);
	window_SetMainWidget(w, (widget_t*) c);
}

static void FileChooserButton(widget_t *source) {
	button_t *b = (button_t*) source;
	/* find which button has been pressed */
	if(!strcmp(b->name, "OK")) {
		dialog.fileChooser.OKclicked = 1;
	} else if(!strcmp(b->name, "ABORT")) {
		dialog.fileChooser.OKclicked = 0;
	}
	xSemaphoreGive(dialog.fileChooser.dialogDone);
}

DialogResult_t dialog_FileChooser(const char * const title, char *result,
		const char * const dir, const char * const filetype) {
	if(xTaskGetCurrentTaskHandle() == GUIHandle) {
		/* This dialog must never be called by the GUI thread (Deadlock) */
		CRIT_ERROR("Dialog started from GUI thread.");
	}

	/* check pointers */
	if (!title || !dir) {
		return DIALOG_RESULT_ERR;
	}

	dialog.fileChooser.dialogDone = xSemaphoreCreateBinary();
	if(!dialog.fileChooser.dialogDone) {
		/* failed to create semaphore */
		return DIALOG_RESULT_ERR;
	}

	/* Find applicable files */
	if(!xSemaphoreTake(fileAccess, 1000)) {
		/* failed to allocate fileAccess */
		return DIALOG_RESULT_ERR;
	}

#define MAX_NUMBER_OF_FILES		50
	char *filenames[MAX_NUMBER_OF_FILES + 1];
	uint8_t foundFiles = 0;
	FRESULT fr; /* Return value */
	DIR dj; /* Directory search object */
	FILINFO fno; /* File information */

#if _USE_LFN
	static char lfn[_MAX_LFN + 1]; /* Buffer to store the LFN */
	fno.lfname = lfn;
	fno.lfsize = sizeof lfn;
#endif

	fr = f_opendir(&dj, dir);

	if (fr == FR_OK) {
		while (f_readdir(&dj, &fno) == FR_OK) {
			char *fn;
#if _USE_LFN
			fn = *fno.lfname ? fno.lfname : fno.fname;
#else
			fn = fno.fname;
#endif
			if(!fn[0] || foundFiles >= MAX_NUMBER_OF_FILES)
				/* no more files */
				break;
			if (filetype) {
				/* check file for filetype */
				char *typestart = strchr(fn, '.');
				if (!typestart) {
					/* file has not type */
					continue;
				}
				typestart++;
				if (strcmp(typestart, filetype)) {
					/* type doesn't match */
					continue;
				}
			}
			/* allocate memory for filename */
			filenames[foundFiles] = pvPortMalloc(strlen(fn));
			if (!filenames[foundFiles]) {
				/* malloc failed */
				/* free already allocated names and return with error */
				while (foundFiles > 0) {
					foundFiles--;
					vPortFree(filenames[foundFiles]);
				}
				return DIALOG_RESULT_ERR;
			}
			/* copy filename */
			strcpy(filenames[foundFiles], fn);
			/* switch to next filename */
			foundFiles++;
		}
		f_closedir(&dj);
	}
	xSemaphoreGive(fileAccess);
	/* Got all matching filenames */
	/* mark end of filename strings */
	filenames[foundFiles] = 0;

	/* Create window */
	window_t *w = window_new(title, Font_Big, COORDS(280, 200));
	container_t *c = container_new(window_GetAvailableArea(w));

	button_t *bAbort = button_new("ABORT", Font_Big, 80, FileChooserButton);

	uint8_t selectedFile = 0;
	if (foundFiles) {
		itemChooser_t *i = itemChooser_new(filenames, &selectedFile, Font_Big,
				(c->base.size.y - 30) / Font_Big.height, c->base.size.x);
		container_attach(c, (widget_t*) i, COORDS(0, 0));
		button_t *bOK = button_new("OK", Font_Big, 80, FileChooserButton);
		container_attach(c, (widget_t*) bOK,
				COORDS(c->base.size.x - bOK->base.size.x - 5,
						c->base.size.y - bOK->base.size.y - 5));
		widget_Select((widget_t*) i);
	} else {
		/* got no files */
		label_t *lNoFiles = label_newWithText("No files available", Font_Big);
		container_attach(c, (widget_t*) lNoFiles,
				COORDS((c->base.size.x - lNoFiles->base.size.x) / 2, 40));
		widget_Select((widget_t*) bAbort);
	}


	container_attach(c, (widget_t*) bAbort,
			COORDS(5, c->base.size.y - bAbort->base.size.y - 5));

	window_SetMainWidget(w, (widget_t*) c);

	/* Wait for button to be clicked */
	xSemaphoreTake(dialog.fileChooser.dialogDone, portMAX_DELAY);

	if(dialog.fileChooser.OKclicked) {
		strcpy(result, filenames[selectedFile]);
	}

	/* free all allocated filenames */
	while (foundFiles > 0) {
		foundFiles--;
		vPortFree(filenames[foundFiles]);
	}

	/* delete window */
	window_destroy((window_t*) w);

	if(dialog.fileChooser.OKclicked) {
		return DIALOG_RESULT_OK;
	} else {
		return DIALOG_RESULT_ABORT;
	}
}

static void stringInputChar(char c) {
	if(c == 0x08) {
		/* backspace, delete last char */
		if(dialog.StringInput.pos>0) {
			dialog.StringInput.pos--;
			dialog.StringInput.string[dialog.StringInput.pos] = 0;
			label_SetText(dialog.StringInput.lString, dialog.StringInput.string);
		}
	} else {
		/* append character if space is available */
		if(dialog.StringInput.pos<dialog.StringInput.maxLength - 1) {
			dialog.StringInput.string[dialog.StringInput.pos] = c;
			dialog.StringInput.pos++;
			label_SetText(dialog.StringInput.lString, dialog.StringInput.string);
		}
	}
}

static void StringInputButton(widget_t *source) {
	button_t *b = (button_t*) source;
	/* find which button has been pressed */
	if(!strcmp(b->name, "OK")) {
		dialog.StringInput.OKclicked = 1;
	} else if(!strcmp(b->name, "ABORT")) {
		dialog.StringInput.OKclicked = 0;
	}
	xSemaphoreGive(dialog.StringInput.dialogDone);
}

DialogResult_t dialog_StringInput(const char * const title, char *result, uint8_t maxLength) {
	if(xTaskGetCurrentTaskHandle() == GUIHandle) {
		/* This dialog must never be called by the GUI thread (Deadlock) */
		CRIT_ERROR("Dialog started from GUI thread.");
	}

	/* check pointers */
	if (!title || !result) {
		return DIALOG_RESULT_ERR;
	}

	dialog.StringInput.dialogDone = xSemaphoreCreateBinary();
	if(!dialog.StringInput.dialogDone) {
		/* failed to create semaphore */
		return DIALOG_RESULT_ERR;
	}

	dialog.StringInput.string = result;
	dialog.StringInput.maxLength = maxLength;
	dialog.StringInput.pos = 0;

	memset(result, 0, maxLength);

	/* Create window */
	window_t *w = window_new(title, Font_Big, COORDS(313, 233));
	container_t *c = container_new(window_GetAvailableArea(w));

	keyboard_t *k = keyboard_new(stringInputChar);

	dialog.StringInput.lString = label_newWithLength(
			c->base.size.x / Font_Big.width, Font_Big, LABEL_CENTER);

	/* Create buttons */
	button_t *bOK = button_new("OK", Font_Big, 80, StringInputButton);
	button_t *bAbort = button_new("ABORT", Font_Big, 80, StringInputButton);

	container_attach(c, (widget_t*) dialog.StringInput.lString, COORDS(0, 8));
	container_attach(c, (widget_t*) k, COORDS(0, 30));
	container_attach(c, (widget_t*) bOK,
			COORDS(c->base.size.x - bOK->base.size.x - 5,
					c->base.size.y - bOK->base.size.y - 5));
	container_attach(c, (widget_t*) bAbort,
			COORDS(5, c->base.size.y - bAbort->base.size.y - 5));

	widget_Select((widget_t*) k);
	window_SetMainWidget(w, (widget_t*) c);

	/* Wait for button to be clicked */
	xSemaphoreTake(dialog.StringInput.dialogDone, portMAX_DELAY);

	/* delete window */
	window_destroy((window_t*) w);

	if(dialog.StringInput.OKclicked) {
		return DIALOG_RESULT_OK;
	} else {
		return DIALOG_RESULT_ABORT;
	}
}

