// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 SASANO Takayoshi <uaa@uaa.org.uk>

#include <stdio.h>
#include <string.h>
#include "test.h"
#include "secure.h"
#include "platform.h"

int main(int argc, char *argv[])
{
	random_engine_initialize();

	if (platform_setup(argc, argv) < 0)
		goto fin0;

	random_fill_buf(seckey, sizeof(seckey));
	if (secure_engine_initialize() < 0) {
		printf("cannot create secp256k1 context\n");
		goto fin0;
	}

	if (serial_open(serdev) < 0) {
		printf("cannot open serial port\n");
		goto fin0;
	}

	test_loop();
	/*NOTREACHED*/
fin0:
	return platform_finish();
}
