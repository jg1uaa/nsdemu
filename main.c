// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 SASANO Takayoshi <uaa@uaa.org.uk>

#include <stdio.h>
#include <unistd.h>
#include "command.h"
#include "nostr-key.h"
#include "secure.h"
#include "serial.h"

static char *serdev = NULL;
static char *keystr = NULL;

int main(int argc, char *argv[])
{
	int ch;

	while ((ch = getopt(argc, argv, "l:k:")) != -1) {
		switch (ch) {
		case 'l':
			serdev = optarg;
			break;
		case 'k':
			keystr = optarg;
			break;
		}
	}

	if (serdev == NULL || keystr == NULL) {
		printf("usage: %s -l [serial device] -k [secure key]\n",
		       argv[0]);
		goto fin0;
	}

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
	return 0;
}
