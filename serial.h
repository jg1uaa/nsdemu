// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 SASANO Takayoshi <uaa@uaa.org.uk>

char serial_read_char(void);
void serial_send_buffer(const void *buf, int len);
void serial_send_string(const char *str);
void serial_close(void);
int serial_open(const char *arg);
