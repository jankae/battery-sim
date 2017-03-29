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
	MSG_RESULT_OK, MSG_RESULT_ABORT, MSG_RESULT_ERR
} MsgResult_t;

void dialog_InputValue(int32_t * const result,
		const int32_t min, const int32_t max, const unit_t * const unit);

void dialog_MessageBox(const char * const title, const char * const msg,
		MsgBox_t type, void (*cb)(MsgResult_t));

#endif /* DIALOG_H_ */
