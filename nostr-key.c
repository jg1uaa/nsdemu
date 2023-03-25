// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 SASANO Takayoshi <uaa@uaa.org.uk>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "nostr-key.h"

#define unpacked_size(x) (((x) * 8 + 4) / 5)	// round-up
#define packed_size(x) (((x) * 5) / 8)		// round-down
#define expanded_hrp_size(x) (((x) * 2) + 1)

#define BECH32_CHECKSUM_LEN 6
#define LOWER_CASE_USED (1 << 0)
#define UPPER_CASE_USED (1 << 1)

static int check_string_case(char *str, int len)
{
	int i, m;

	m = 0;
	for (i = 0; i < len; i++) {
		if (str[i] >= 'a' && str[i] <= 'z')
			m |= LOWER_CASE_USED;
		else if (str[i] >= 'A' && str[i] <= 'Z')
			m |= UPPER_CASE_USED;
	}

	if (m == 0)
		m = LOWER_CASE_USED; // as default

	return m;
}

static uint32_t bech32_polymod(uint8_t *values, int len)
{
	uint32_t b, chk = 1;
	int i;

	for (i = 0; i < len; i++) {
		b = (chk >> 25);
		chk = (chk & 0x01ffffff) << 5 ^ (values[i] & 0x01f);
		chk ^= (b & 0x01) ? 0x3b6a57b2 : 0;
		chk ^= (b & 0x02) ? 0x26508e6d : 0;
		chk ^= (b & 0x04) ? 0x1ea119fa : 0;
		chk ^= (b & 0x08) ? 0x3d4233dd : 0;
		chk ^= (b & 0x10) ? 0x2a1462b3 : 0;
	}

	return chk;
}

static uint8_t bech32_char_decode(int c)
{
	const static uint8_t decode_table[] = {
		0x0f, 0xff, 0x0a, 0x11, 0x15, 0x14, 0x1a, 0x1e, // 01234567
		0x07, 0x05, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // 89:;<=>?
		0xff, 0x1d, 0xff, 0x18, 0x0d, 0x19, 0x09, 0x08, // @ABCDEFG
		0x17, 0xff, 0x12, 0x16, 0x1f, 0x1b, 0x13, 0xff, // HIJKLMNO
		0x01, 0x00, 0x03, 0x10, 0x0b, 0x1c, 0x0c, 0x0e, // PQRSTUVW
		0x06, 0x04, 0x02, 0xff, 0xff, 0xff, 0xff, 0xff, // XYZ[\]^_
		0xff, 0x1d, 0xff, 0x18, 0x0d, 0x19, 0x09, 0x08, // `abcdefg
		0x17, 0xff, 0x12, 0x16, 0x1f, 0x1b, 0x13, 0xff, // hijklmno
		0x01, 0x00, 0x03, 0x10, 0x0b, 0x1c, 0x0c, 0x0e, // pqrstuvw
		0x06, 0x04, 0x02, 0xff, 0xff, 0xff, 0xff, 0xff, // xyz{|}~
	};

	return (c < '0' || c > 'z') ? 0xff : decode_table[c - '0'];
}

static int pack5to8(uint8_t *out, int outlen, uint8_t *in, int inlen)
{
	int inpos, outpos, olen;
	uint64_t v;

	olen = packed_size(inlen);
	outlen = (outlen < olen) ? outlen : olen;

	for (inpos = outpos = 0; inpos < inlen && outpos < outlen; ) {
		v = (uint64_t)(in[inpos++] & 0x1f) << 35;
		if (inpos < inlen) v |= (uint64_t)(in[inpos++] & 0x1f) << 30;
		if (inpos < inlen) v |= (uint64_t)(in[inpos++] & 0x1f) << 25;
		if (inpos < inlen) v |= (uint64_t)(in[inpos++] & 0x1f) << 20;
		if (inpos < inlen) v |= (uint64_t)(in[inpos++] & 0x1f) << 15;
		if (inpos < inlen) v |= (uint64_t)(in[inpos++] & 0x1f) << 10;
		if (inpos < inlen) v |= (uint64_t)(in[inpos++] & 0x1f) << 5;
		if (inpos < inlen) v |= (uint64_t)(in[inpos++] & 0x1f);

		out[outpos++] = v >> 32;
		if (outpos < outlen) out[outpos++] = v >> 24;
		if (outpos < outlen) out[outpos++] = v >> 16;
		if (outpos < outlen) out[outpos++] = v >> 8;
		if (outpos < outlen) out[outpos++] = v;
	}

	return outpos;
}

static int unpack8to5(uint8_t *out, int outlen, uint8_t *in, int inlen)
{
	int inpos, outpos, olen;
	uint64_t v;

	olen = unpacked_size(inlen);
	outlen = (outlen < olen) ? outlen : olen;

	for (inpos = outpos = 0; inpos < inlen && outpos < outlen; ) {
		v = (uint64_t)in[inpos++] << 32;
		if (inpos < inlen) v |= (uint64_t)in[inpos++] << 24;
		if (inpos < inlen) v |= (uint64_t)in[inpos++] << 16;
		if (inpos < inlen) v |= (uint64_t)in[inpos++] << 8;
		if (inpos < inlen) v |= (uint64_t)in[inpos++];

		out[outpos++] = (v >> 35) & 0x1f;
		if (outpos < outlen) out[outpos++] = (v >> 30) & 0x1f;
		if (outpos < outlen) out[outpos++] = (v >> 25) & 0x1f;
		if (outpos < outlen) out[outpos++] = (v >> 20) & 0x1f;
		if (outpos < outlen) out[outpos++] = (v >> 15) & 0x1f;
		if (outpos < outlen) out[outpos++] = (v >> 10) & 0x1f;
		if (outpos < outlen) out[outpos++] = (v >> 5) & 0x1f;
		if (outpos < outlen) out[outpos++] = v & 0x1f;
	}

	return outpos;
}

static int expand_hrp(uint8_t *out, int outlen, char *in, int inlen)
{
// do not use pre/post incremented argument in macro
#define lower_case(c) (((c) >= 'A' && (c) <= 'Z') ? ((c) - 'A' + 'a') : (c))

	int inpos, outpos, olen;

	olen = expanded_hrp_size(inlen);
	outlen = (outlen < olen) ? outlen : olen;

	for (inpos = outpos = 0; inpos < inlen && outpos < outlen;
	     inpos++, outpos++)
		out[outpos] = (lower_case(in[inpos]) >> 5) & 0x07;

	if (outpos < outlen)
		out[outpos++] = 0;

	for (inpos = 0; inpos < inlen && outpos < outlen; inpos++, outpos++)
		out[outpos] = lower_case(in[inpos]) & 0x1f;

	return outpos;
}

int nostr_key_decode(uint8_t *out, int outlen, char *in)
{
	int i, j, hrplen, tmpsize, v = -1;
	char *sep;
	uint8_t *tmp;

	/* get HRP length, check upper/lower case */
	if ((sep = strrchr(in, '1')) == NULL)
		goto fin0;
	hrplen = sep - in;
	if (check_string_case(in, strlen(in)) == (LOWER_CASE_USED |
						  UPPER_CASE_USED))
		goto fin0;

	/* allocate working buffer */
	tmpsize = expanded_hrp_size(hrplen) +
		unpacked_size(outlen) + BECH32_CHECKSUM_LEN;
	tmp = alloca(tmpsize);

	/* expand HRP string to calculate checksum */
	expand_hrp(tmp, tmpsize, in, hrplen);

	/* decode bech32 string with checksum */
	for (i = hrplen + 1, j = expanded_hrp_size(hrplen); in[i]; i++, j++)
		if ((tmp[j] = bech32_char_decode(in[i])) == 0xff)
			goto fin0;

	/* validate checksum */
	if (bech32_polymod(tmp, tmpsize) != 1)
		goto fin0;

	/* output result */
	pack5to8(out, outlen,
		 tmp + expanded_hrp_size(hrplen), unpacked_size(outlen));
	v = 0;
fin0:
	return v;
}

static void bech32_set_checksum(uint8_t *p, uint32_t cs)
{
	int i;

	for (i = 0; i < BECH32_CHECKSUM_LEN; i++)
		p[i] = (cs >> (5 * (5 - i))) & 0x1f;
}

int nostr_key_encode(char *out, int outlen, uint8_t *in, int inlen, char *hrp)
{
	int i, hrplen, tmpsize, v = -1;
	uint8_t *tmp, *p;
	const char *t;
	const static char *encode_table[] = {
		[0] = NULL,
		[LOWER_CASE_USED] = "qpzry9x8gf2tvdw0s3jn54khce6mua7l",
		[UPPER_CASE_USED] = "QPZRY9X8GF2TVDW0S3JN54KHCE6MUA7L",
		[LOWER_CASE_USED | UPPER_CASE_USED] = NULL,
	};

	/* get HRP length, check upper/lower case */
	hrplen = strlen(hrp);
	if (hrplen < 1 || hrplen > 83)
		goto fin0;
	if ((t = encode_table[check_string_case(hrp, hrplen)]) == NULL)
		goto fin0;

	/* allocate working buffer (with terminate character for string) */
	tmpsize = expanded_hrp_size(hrplen) +
		unpacked_size(inlen) + BECH32_CHECKSUM_LEN;
	tmp = alloca(tmpsize + 1);
	tmp[tmpsize] = 0;

	/* expand HRP string to calculate checksum */
	expand_hrp(tmp, tmpsize, hrp, hrplen);

	/* convert hex payload */
	unpack8to5(tmp + expanded_hrp_size(hrplen), unpacked_size(inlen),
		   in, inlen);

	/* calculate checksum (checksum region is 0 filled) */
	bech32_set_checksum(tmp +
			    expanded_hrp_size(hrplen) + unpacked_size(inlen),
			    bech32_polymod(tmp, tmpsize) ^ 1);

	/* output result */
	p = tmp + expanded_hrp_size(hrplen) - hrplen - 1;
	memcpy(p, hrp, hrplen);
	p[hrplen] = '1';
	for (i = expanded_hrp_size(hrplen); i < tmpsize; i++)
		tmp[i] = t[tmp[i]];
	snprintf(out, outlen, "%s", p); // strlcpy() should be better
	v = 0;
fin0:
	return v;
}
