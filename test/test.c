// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 SASANO Takayoshi <uaa@uaa.org.uk>

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "test.h"
#include "command.h"
#include "secure.h"
#include "platform.h"
#include "secp256k1_schnorrsig.h"
#include "secp256k1_extrakeys.h"

static char cmd[32], arg[160];
static uint8_t nsd_pubkey[sizeof(pubkey) + 1];
static uint8_t nsd_pubkey_xy[65];

static secp256k1_context *ctx_test;
static secp256k1_xonly_pubkey xpubkey;

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
	size_t outputlen;
	secp256k1_pubkey ec_pubkey;

	/* simply send public-key */
	if (send_and_receive(cmdstr, NULL,
			     nsd_pubkey + 1, sizeof(nsd_pubkey) - 1) < 0)
		goto error;

	/* create pubkey(xy) from obtained public-key */
	nsd_pubkey[0] = 0x02; // compressed form
	outputlen = sizeof(nsd_pubkey_xy);
	if (!secp256k1_xonly_pubkey_parse(ctx_test, &xpubkey, nsd_pubkey + 1) ||
	    !secp256k1_ec_pubkey_parse(ctx_test, &ec_pubkey,
				       nsd_pubkey, sizeof(nsd_pubkey)) ||
	    !secp256k1_ec_pubkey_serialize(ctx_test, nsd_pubkey_xy,
					   &outputlen, &ec_pubkey,
					   SECP256K1_EC_UNCOMPRESSED)) {
		printf("%s: internal error\n", cmdstr);
		goto error;
	}

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

	/* verify signature */
	if (!secp256k1_schnorrsig_verify(ctx_test, sig, id,
					 sizeof(id), &xpubkey))
		goto error;

	return 0;
error:
	return -1;
}

static int test_shared_secret(void)
{
	const static char cmdstr[] = "shared-secret";
	uint8_t sec[32];
	char buf[hexstr_len(sizeof(nsd_pubkey_xy))];

	/* send shared-secret with pubkey(xy) */
	encode_hex(buf, sizeof(buf),
		   nsd_pubkey_xy + 1, sizeof(nsd_pubkey_xy) - 1);
	if (send_and_receive(cmdstr, buf, sec, sizeof(sec)) < 0)
		goto error;

	// XXX no content check

	return 0;
error:
	return -1;
}

void test_loop(void)
{
	if ((ctx_test =
	     secp256k1_context_create(SECP256K1_CONTEXT_NONE)) == NULL) {
		printf("cannot create secp256k1 context\n");
		return;
	}

	if (test_ping1())
		printf("ping1: error\n");
	if (test_ping2())
		printf("ping2: error\n");
	if (test_pubkey())
		printf("pubkey: error\n");

	while(1) {
		if (test_sign())
			printf("sign: error\n");
		if (test_shared_secret())
			printf("shared secret: error\n");
		
		putchar('.'); fflush(stdout);
	}
}
