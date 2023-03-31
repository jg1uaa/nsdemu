// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 SASANO Takayoshi <uaa@uaa.org.uk>

#include <stdio.h>
#include "command.h"
#include "nostr-key.h"
#include "secure.h"
#include "platform.h"

int main(int argc, char *argv[])
{
	if (platform_setup(argc, argv) < 0)
		goto fin0;

	if (nostr_key_decode(seckey, sizeof(seckey), keystr) &&
	    decode_hex(seckey, sizeof(seckey), keystr)) {
		printf("invalid key\n");
		goto fin0;
	}

	if (secure_engine_initialize() < 0) {
		printf("cannot create secp256k1 context\n");
		goto fin0;
	}

	if (serial_open(serdev) < 0) {
		printf("cannot open serial port\n");
		goto fin0;
	}

	command_loop();
	/*NOTREACHED*/
fin0:
	return platform_finish();
}
