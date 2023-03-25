// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 SASANO Takayoshi <uaa@uaa.org.uk>

#include <stdlib.h>
#include "random.h"

void random_engine_initialize(void) {
	/* do nothing */
}

void random_fill_buf(void *buf, int len) {
	arc4random_buf(buf, len);
}
