// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 SASANO Takayoshi <uaa@uaa.org.uk>

#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include "serial.h"

#define SIGNDEV_SERIAL_SPEED B9600

static int fd = -1;

char serial_read_char(void)
{
	char c = 0;
	ssize_t size;

	/* not good, but enough to test */
	while (fd >= 0 && (size = read(fd, &c, sizeof(c))) < 1)
		usleep(10000);

	return c;
}

void serial_send_buffer(const void *buf, int len)
{
	write(fd, buf, len);
}

void serial_send_string(const char *str)
{
	serial_send_buffer(str, strlen(str));
}

int serial_open(const char *arg)
{
	struct termios t;
	int flags, v = -1;

	/* device is non-blocked during setup */
	if ((fd = open(arg, O_RDWR | O_NOCTTY | O_EXCL | O_NONBLOCK)) < 0)
		goto fin0;

	memset(&t, 0, sizeof(t));
	cfsetospeed(&t, SIGNDEV_SERIAL_SPEED);
	cfsetispeed(&t, SIGNDEV_SERIAL_SPEED);

	t.c_cflag |= CREAD | CLOCAL | CS8;
	// t.c_cflag |= CRTSCTS;
	t.c_iflag = INPCK;
	t.c_oflag = 0;
	t.c_lflag = 0;
	t.c_cc[VTIME] = 0;
	t.c_cc[VMIN] = 1;

	tcflush(fd, TCIOFLUSH);
	tcsetattr(fd, TCSANOW, &t);

	if ((flags = fcntl(fd, F_GETFL)) < 0 ||
	    fcntl(fd, F_SETFL, (flags & ~O_NONBLOCK)) < 0)
		goto fin1;

	v = 0;
	goto fin0;
fin1:
	serial_close();
fin0:
	return v;
}

void serial_close(void)
{
	close(fd);
	fd = -1;
}
