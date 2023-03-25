// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 SASANO Takayoshi <uaa@uaa.org.uk>

#include <stdint.h>

extern uint8_t seckey[32], pubkey[32], uid[16];

int secure_make_signature(uint8_t *sig, uint8_t *msg);
int secure_make_shared_secret(uint8_t *sec, uint8_t *pub, int publen);
int secure_engine_initialize(void);
