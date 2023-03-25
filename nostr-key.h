// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 SASANO Takayoshi <uaa@uaa.org.uk>

#include <stdint.h>

int nostr_key_decode(uint8_t *out, int outlen, char *in);
int nostr_key_encode(char *out, int outlen, uint8_t *in, int inlen, char *hrp);
