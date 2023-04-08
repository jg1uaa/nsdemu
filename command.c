// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 SASANO Takayoshi <uaa@uaa.org.uk>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "command.h"
#include "secure.h"
#include "platform.h"

#define CRLF "\x0d\x0a"

static void command_ping(const char *cmd, const char *arg);
static void command_pubkey(const char *cmd, const char *arg);
static void command_sign(const char *cmd, const char *arg);
static void command_shared_secret(const char *cmd, const char *arg);

struct cmdtbl {
	const char *cmd;
	void (*func)(const char *cmd, const char *arg);
};

static struct cmdtbl cmds[] = {
	{"ping", command_ping},
	{"public-key", command_pubkey},
	{"sign-message", command_sign},
	{"shared-secret", command_shared_secret},
};

void encode_hex(char *out, int outsize, const uint8_t *in, int insize)
{
	const static char c[] = "0123456789abcdef";
	int i, j;

	for (i = j = 0; i < outsize - 1 && j < insize; j++) {
		out[i++] = c[(in[j] >> 4) & 0x0f];
		if (i < outsize - 1)
			out[i++] = c[in[j] & 0x0f];
	}
	out[i] = '\0';
}

int decode_hex(uint8_t *buf, int buflen, const char *str)
{
	int i, p, n;
	char *tmp;

	n = buflen * 2;
	tmp = alloca(n);
	memset(tmp, 0, n);
	for (i = 0, p = strlen(str) - 1; i < n; i++, p--) {
		if (str[p] >= '0' && str[p] <= '9')
			tmp[i] = str[p] - '0';
		else if (str[p] >= 'a' && str[p] <= 'f')
			tmp[i] = str[p] - 'a' + 10;
		else if (str[p] >= 'A' && str[p] <= 'F')
			tmp[i] = str[p] - 'A' + 10;
		else return -1;
	}
				
	for (i = 0, p = n - 1; i < buflen; i++, p -= 2)
		buf[i] = (tmp[p] << 4) | tmp[p - 1];

	return 0;
}

void send_response(const char *cmd, const char *sep, const char *arg)
{
	serial_send_string("/");
	serial_send_string(cmd);
	if (sep != NULL) serial_send_string(sep);
	if (arg != NULL) serial_send_string(arg);
	serial_send_string(CRLF);

#ifdef DIAGNOSE
	printf("/%s%s%s\n", cmd, (sep != NULL) ? sep : "(null)",
	       (arg != NULL) ? arg : "(null)");
#endif
}

static void command_ping(const char *cmd, const char *arg)
{
	char buf[hexstr_len(sizeof(uid))];

	if (arg == NULL)
		return;

	encode_hex(buf, sizeof(buf), uid, sizeof(uid));
	send_response(cmd, " 0 ", buf);
}

static void command_pubkey(const char *cmd, const char *arg)
{
	char buf[hexstr_len(sizeof(pubkey))];

	encode_hex(buf, sizeof(buf), pubkey, sizeof(pubkey));
	send_response(cmd, " ", buf);
}

static void command_sign(const char *cmd, const char *arg)
{
	uint8_t id[32], sig[64];
	char buf[hexstr_len(sizeof(sig))];

	if (arg == NULL)
		return;

	if (decode_hex(id, sizeof(id), arg) < 0)
		return;

	if (secure_make_signature(sig, id) < 0)
		return;

	encode_hex(buf, sizeof(buf), sig, sizeof(sig));
	send_response(cmd, " ", buf);
}

static void command_shared_secret(const char *cmd, const char *arg)
{
#define pubkey_len 65

	uint8_t secret[32];
	uint8_t pubkey[pubkey_len];
	char buf[hexstr_len(sizeof(secret))];

	if (arg == NULL)
		return;

	pubkey[0] = 0x04; // uncompressed form
	if (decode_hex(pubkey + 1, pubkey_len - 1, arg) < 0)
		return;

	if (secure_make_shared_secret(secret, pubkey, pubkey_len) < 0)
		return;

	encode_hex(buf, sizeof(buf), secret, sizeof(secret));
	send_response(cmd, " ", buf);
}

static void process_command(const char *cmd, const char *arg)
{
	int i;

#ifdef DIAGNOSE
	printf("cmd: %s arg: %s\n", cmd, (arg != NULL) ? arg : "(null)");
#endif

	for (i = 0; i < sizeof(cmds) / sizeof(struct cmdtbl); i++) {
		if (!strcmp(cmds[i].cmd, cmd)) {
			(*(cmds[i].func))(cmd, arg);
			break;
		}
	}
}

void wait_for_character(char d)
{
	while (serial_read_char() != d);
}

int receive_token(char *buf, int len, int arg_mode)
{
	int i, v;
	char c;

	/*
	 * arg_mode=0: get command token
	 *	<0: error
	 *	0: no argument follows
	 *	>0: argument follows
	 * arg_mode=1: get argument
	 *	(delimiters before argument is removed)
	 *	<0: error
	 *	0: no argument got
	 *	>0: argument got
	 */

	for (i = 0; i < len;) {
		c = serial_read_char();
		if (c == ' ') {
			if (arg_mode && !i) {
				/* ignore delimiter */
				continue;
			} else {
				v = i;
				break;
			}
		} else if (c == '\x0d' || c == '\x0a') {
			v = arg_mode ? i : 0;
			break;
		} else
			buf[i++] = c;
	}

	if (i < len)
		buf[i] = '\0';
	else
		v = -1; /* buffer overflow */

	return v;
}

void command_loop(void)
{
	int have_arg;
	char cmd[32], arg[160];

	while (1) {
		wait_for_character('/');

		if ((have_arg = receive_token(cmd, sizeof(cmd), 0)) < 0)
			continue;

		if (have_arg &&
		    (have_arg = receive_token(arg, sizeof(arg), 1)) < 0)
			continue;
		
		process_command(cmd, have_arg ? arg : NULL);
	}
}
