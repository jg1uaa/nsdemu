// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 SASANO Takayoshi <uaa@uaa.org.uk>

#include <stdint.h>

void encode_hex(char *out, int outsize, const uint8_t *in, int insize);
int decode_hex(uint8_t *buf, int buflen, const char *str);
void send_response(const char *cmd, const char *sep, const char *arg);
void wait_for_character(char d);
int receive_token(char *buf, int len, int arg_mode);
void command_loop(void);

#define hexstr_len(x) ((x) * 2 + 1)
