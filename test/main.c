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
	platform_setup(argc, argv);

	memset(seckey, 0xee, sizeof(seckey));

	if (serdev != NULL) {
		if (secure_engine_initialize() < 0) {
			printf("invalid key\n");
			goto fin0;
		}

		if (serial_open(serdev) < 0) {
			printf("cannot open serial port\n");
			goto fin0;
		}

		test_loop();
		/*NOTREACHED*/
	}
fin0:
	return platform_finish();
}
