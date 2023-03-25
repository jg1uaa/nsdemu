// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 SASANO Takayoshi <uaa@uaa.org.uk>

#include <stdint.h>

void encode_hex(char *out, int outsize, const uint8_t *in, int insize);
int decode_hex(uint8_t *buf, int buflen, const char *str);
void command_loop(void);
