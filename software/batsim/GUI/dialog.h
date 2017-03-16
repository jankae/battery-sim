/*
 * dialog.h
 *
 *  Created on: Mar 16, 2017
 *      Author: jan
 */

#ifndef DIALOG_H_
#define DIALOG_H_

#include "gui.h"

void dialog_InputValue(const char * const title, int32_t *const result,
		const int32_t min, const int32_t max, const unit_t * const unit);

#endif /* DIALOG_H_ */
