// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 SASANO Takayoshi <uaa@uaa.org.uk>

extern char *serdev;
extern char *keystr;

int platform_setup(int argc, char *argv[]);
int platform_finish(void);

void random_engine_initialize(void);
void random_fill_buf(void *buf, int len);

char serial_read_char(void);
void serial_send_buffer(const void *buf, int len);
void serial_send_string(const char *str);
void serial_close(void);
int serial_open(const char *arg);
