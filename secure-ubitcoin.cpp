// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 SASANO Takayoshi <uaa@uaa.org.uk>

#include <string.h>
#include "Bitcoin.h"

extern "C" {
#include "platform.h"
#include "secure.h"
}

uint8_t seckey[32], pubkey[32], uid[16];

static PrivateKey *private_key;
static PublicKey public_key;

extern "C" uint32_t random32(void)
{
	uint32_t v;

	random_fill_buf(&v, sizeof(v));

	return v;
}

int secure_make_signature(uint8_t *sig, uint8_t *msg)
{
	SchnorrSignature signature = private_key->schnorr_sign(msg);
	signature.serialize(sig, 64);

	return 0;
}

int secure_make_shared_secret(uint8_t *sec, uint8_t *pub, int publen)
{
	/* pub comes with "04" prefix */
	PublicKey peerpubkey(pub + 1, false);
	private_key->ecdh(peerpubkey, sec, false);

	return 0;
}

int secure_engine_initialize(void)
{
	private_key = new PrivateKey(seckey);
	public_key = private_key->publicKey();
	public_key.x(pubkey, sizeof(pubkey));

	/* create uuid from pubkey */
	for (int i = 0; i < sizeof(uid); i++)
		uid[i] = pubkey[i] ^ pubkey[i + sizeof(uid)];

	return 0;
}
