/*
 * dialog.h
 *
 *  Created on: Mar 16, 2017
 *      Author: jan
 */

#ifndef DIALOG_H_
#define DIALOG_H_

#include "gui.h"

typedef enum {
	MSG_ABORT_OK, MSG_OK
} MsgBox_t;

typedef enum {
	DIALOG_RESULT_OK, DIALOG_RESULT_ABORT, DIALOG_RESULT_ERR
} DialogResult_t;

void dialog_InputValue(int32_t * const result,
		const int32_t min, const int32_t max, const unit_t * const unit);

void dialog_MessageBox(const char * const title, font_t font, const char * const msg,
		MsgBox_t type, void (*cb)(DialogResult_t));

DialogResult_t dialog_FileChooser(const char * const title, char *result,
		const char * const dir, const char * const filetype);

DialogResult_t dialog_StringInput(const char * const title, char *result, uint8_t maxLength);


#endif /* DIALOG_H_ */
