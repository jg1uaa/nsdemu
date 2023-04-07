// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 SASANO Takayoshi <uaa@uaa.org.uk>

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "test.h"
#include "command.h"
#include "secure.h"
#include "platform.h"
#include "secp256k1_extrakeys.h"

static char cmd[32], arg[160];
extern secp256k1_xonly_pubkey xpubkey; // from secure.c

static int wait_for_device(void)
{
	int have_arg;

	wait_for_character('/');

	if ((have_arg = receive_token(cmd, sizeof(cmd), 0)) < 0)
		goto fin;

	if (have_arg && (have_arg = receive_token(arg, sizeof(arg), 1)) < 0)
		goto fin;

fin:
	return have_arg;
}

static int send_and_receive(const char *command, const char *argument, uint8_t *buf, int bufsize)
{
	send_response(command, argument == NULL ? NULL : " ", argument);

	if (wait_for_device() < 1) {
		printf("%s: rx error\n", command);
		goto error;
	}		

	if (strcmp(command, cmd)) {
		printf("%s: invalid command (%s)\n", command, cmd);
		goto error;
	}

	if (bufsize && decode_hex(buf, bufsize, arg) < 0) {
		printf("%s: hex decode error (%s)\n", command, arg);
		goto error;
	}

	return 0;
error:
	return -1;
}

static int test_ping1(void)
{
	/* simply send ping */
	send_response("ping", NULL, NULL);

	/* no response; nothing to check */

	return 0;
}

static int test_ping2(void)
{
	const static char cmdstr[] = "ping";
	char buf[hexstr_len(sizeof(uid))];
	uint8_t tmp[sizeof(uid)];

	/* send ping with argument (random hex for test) */
	random_fill_buf(tmp, sizeof(tmp));
	encode_hex(buf, sizeof(buf), tmp, sizeof(tmp));
	if (send_and_receive(cmdstr, buf, NULL, 0) < 0)
		goto error;

	// XXX no content check

	return 0;
error:
	return -1;
}

static int test_pubkey(void)
{
	const static char cmdstr[] = "public-key";
	uint8_t tmp[sizeof(pubkey)];

	/* simply send public-key */
	if (send_and_receive(cmdstr, NULL, tmp, sizeof(tmp)) < 0)
		goto error;

	// XXX no content check

	return 0;
error:
	return -1;
}

static int test_sign(void)
{
	const static char cmdstr[] = "sign-message";
	uint8_t id[32], sig[64];
	char buf[hexstr_len(sizeof(sig))];

	/* send sign-message with 256bit random value (as hash) */
	random_fill_buf(id, sizeof(id));
	encode_hex(buf, sizeof(buf), id, sizeof(id));
	if (send_and_receive(cmdstr, buf, sig, sizeof(sig)) < 0)
		goto error;

	// XXX no content check

	return 0;
error:
	return -1;
}

static int test_shared_secret(void)
{
	const static char cmdstr[] = "shared-secret";
	uint8_t key[64], sec[32];
	char buf[hexstr_len(sizeof(key))];

	/* send shared-secret with 512bit data */
	encode_hex(buf, sizeof(buf), (uint8_t *)&xpubkey, sizeof(xpubkey));
	if (send_and_receive(cmdstr, buf, sec, sizeof(sec)) < 0)
		goto error;

	// XXX no content check

	return 0;
error:
	return -1;
}

void test_loop(void)
{
	while(1) {
		test_ping1();
		test_ping2();
		test_pubkey();
		test_sign();
		test_shared_secret();
		putchar('.'); fflush(stdout);
	}
}
