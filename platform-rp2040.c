// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 SASANO Takayoshi <uaa@uaa.org.uk>

#include <stdio.h>
#include <string.h>
#include "pico/stdio_usb.h"
#include "pico/time.h"
#include "pico/rand.h"

char *serdev = NULL;
#ifdef KEYSTR
char *keystr = KEYSTR;
#else
#error KEYSTR not defined
#endif

int platform_setup(int argc, char *argv[])
{
	stdio_usb_init();
	stdio_set_translate_crlf(&stdio_usb, false);
	while (!stdio_usb_connected())
		sleep_ms(100);

	return 0;
}

int platform_finish(void)
{
	printf("system halted\n");
	stdio_flush();
	sleep_ms(1000); // enough time to flush needed

	asm __volatile__(
		"	cpsid i	\n"
		"0:		\n"
		"	wfi	\n"
		"	b 0b	\n"
		);

	/* NOTREACHED */
	return 0;
}

void random_engine_initialize(void)
{
	/* do nothing */
}

void random_fill_buf(void *buf, int len)
{
	int i;
	uint8_t *p = (uint8_t *)buf;
	uint64_t v;

	for (i = 0; i < len; ) {
		v = get_rand_64();
		if (i < len) p[i++] = v;
		if (i < len) p[i++] = v >> 8;
		if (i < len) p[i++] = v >> 16;
		if (i < len) p[i++] = v >> 24;
		if (i < len) p[i++] = v >> 32;
		if (i < len) p[i++] = v >> 40;
		if (i < len) p[i++] = v >> 48;
		if (i < len) p[i++] = v >> 56;
	}
}

char serial_read_char(void)
{
	int c;

	while ((c = getchar_timeout_us(1000000)) == PICO_ERROR_TIMEOUT);
	
	return c;
}

void serial_send_buffer(const void *buf, int len)
{
	int i;

	for (i = 0; i < len; i++)
		putchar_raw(((char *)buf)[i]);
}

void serial_send_string(const char *str)
{
	serial_send_buffer(str, strlen(str));
}

int serial_open(const char *arg)
{
	return 0;
}

void serial_close(void)
{
	/* do nothing */
}
