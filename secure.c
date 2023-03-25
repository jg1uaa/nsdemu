// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 SASANO Takayoshi <uaa@uaa.org.uk>

#include <string.h>
#include "secp256k1_schnorrsig.h"
#include "secp256k1_extrakeys.h"
#include "secp256k1_ecdh.h"
#include "secure.h"
#include "random.h"

uint8_t seckey[32], pubkey[32], uid[16];

static secp256k1_context *ctx;
static secp256k1_keypair keypair;
static secp256k1_xonly_pubkey xpubkey;

int secure_make_signature(uint8_t *sig, uint8_t *msg)
{
	uint8_t random_seed[32];

	random_fill_buf(random_seed, sizeof(random_seed));
	return secp256k1_schnorrsig_sign32(ctx, sig, msg,
					   &keypair, random_seed) ? 0 : -1;
}

static int secp256k1_ecdh_no_hash(unsigned char *output, const unsigned char *x32, const unsigned char *y32  __attribute__((unused)), void *data __attribute__((unused)))
{
	/* just copy x */
	memcpy(output, x32, 32);
	return 1;
}

int secure_make_shared_secret(uint8_t *sec, uint8_t *pub, int publen)
{
	secp256k1_pubkey pubkey;

	return (!secp256k1_ec_pubkey_parse(ctx, &pubkey, pub, publen) ||
		!secp256k1_ecdh(ctx, sec, &pubkey, seckey,
				secp256k1_ecdh_no_hash, NULL)) ? -1 : 0;
}

int secure_engine_initialize(void)
{
	uint8_t random_seed[32];
	int i, v = -1;

	random_engine_initialize();
	random_fill_buf(random_seed, sizeof(random_seed));

	/* initialize secp256k1 engine and create pubkey from seckey */
	if ((ctx =
	     secp256k1_context_create(SECP256K1_CONTEXT_SIGN)) == NULL ||
	    !secp256k1_context_randomize(ctx, random_seed) ||
	    !secp256k1_keypair_create(ctx, &keypair, seckey) ||
	    !secp256k1_keypair_xonly_pub(ctx, &xpubkey, NULL, &keypair) ||
	    !secp256k1_xonly_pubkey_serialize(ctx, pubkey, &xpubkey))
		goto fin0;

	/* create uuid from pubkey */
	for (i = 0; i < sizeof(uid); i++)
		uid[i] = pubkey[i] ^ pubkey[i + sizeof(uid)];

	v = 0;
fin0:
	return v;
}
