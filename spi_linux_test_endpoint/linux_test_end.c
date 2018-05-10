/*
 * SPI testing utility (using spidev driver)
 *
 * Copyright (c) 2007  MontaVista Software, Inc.
 * Copyright (c) 2007  Anton Vorontsov <avorontsov@ru.mvista.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * Cross-compile with cross-gcc -I/path/to/cross-kernel/include
 */

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <time.h>

#include <string.h>
#include "../spi_proto.h"

#define SPI_TRANSFER_LEN sizeof(struct spi_packet)

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

static void pabort(const char *s)
{
	perror(s);
	abort();
}

static const char *device = "/dev/spidev0.0";
static uint8_t mode;
static uint8_t bits = 8;
static uint32_t speed = 8388608U; // TODO
static uint16_t delay;

#define SPI_TRANSFER_SIZE SPI_PACKET_LEN

//TODO make this less brittle
uint8_t spi_in_buf[SPI_TRANSFER_SIZE], spi_out_buf[SPI_TRANSFER_SIZE];
static void transfer(int fd)
{
	int ret;
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)spi_out_buf,
		.rx_buf = (unsigned long)spi_in_buf,
		.len = ARRAY_SIZE(spi_out_buf),
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)
		pabort("can't send spi message");
	return;
	for (ret = 0; ret < ARRAY_SIZE(spi_out_buf); ret++) {
		if (!(ret % 12))
			puts("");
		printf("%.2X ", spi_in_buf[ret]);
	}
	puts("");
}

static void print_usage(const char *prog)
{
	printf("Usage: %s [-DsbdlHOLC3]\n", prog);
	puts("  -D --device   device to use (default /dev/spidev1.1)\n"
	     "  -s --speed    max speed (Hz)\n"
	     "  -d --delay    delay (usec)\n"
	     "  -b --bpw      bits per word \n"
	     "  -l --loop     loopback\n"
	     "  -H --cpha     clock phase\n"
	     "  -O --cpol     clock polarity\n"
	     "  -L --lsb      least significant bit first\n"
	     "  -C --cs-high  chip select active high\n"
	     "  -3 --3wire    SI/SO signals shared\n");
	exit(1);
}

static void parse_opts(int argc, char *argv[])
{
	while (1) {
		static const struct option lopts[] = {
			{ "device",  1, 0, 'D' },
			{ "speed",   1, 0, 's' },
			{ "delay",   1, 0, 'd' },
			{ "bpw",     1, 0, 'b' },
			{ "loop",    0, 0, 'l' },
			{ "cpha",    0, 0, 'H' },
			{ "cpol",    0, 0, 'O' },
			{ "lsb",     0, 0, 'L' },
			{ "cs-high", 0, 0, 'C' },
			{ "3wire",   0, 0, '3' },
			{ "no-cs",   0, 0, 'N' },
			{ "ready",   0, 0, 'R' },
			{ NULL, 0, 0, 0 },
		};
		int c;

		c = getopt_long(argc, argv, "D:s:d:b:lHOLC3NR", lopts, NULL);

		if (c == -1)
			break;

		switch (c) {
		case 'D':
			device = optarg;
			break;
		case 's':
			speed = atoi(optarg);
			break;
		case 'd':
			delay = atoi(optarg);
			break;
		case 'b':
			bits = atoi(optarg);
			break;
		case 'l':
			mode |= SPI_LOOP;
			break;
		case 'H':
			mode |= SPI_CPHA;
			break;
		case 'O':
			mode |= SPI_CPOL;
			break;
		case 'L':
			mode |= SPI_LSB_FIRST;
			break;
		case 'C':
			mode |= SPI_CS_HIGH;
			break;
		case '3':
			mode |= SPI_3WIRE;
			break;
		case 'N':
			mode |= SPI_NO_CS;
			break;
		case 'R':
			mode |= SPI_READY;
			break;
		default:
			print_usage(argv[0]);
			break;
		}
	}
}

int main(int argc, char *argv[])
{
	int ret = 0;
	int fd;

	parse_opts(argc, argv);

	fd = open(device, O_RDWR);
	if (fd < 0)
		pabort("can't open device");

	/*
	 * spi mode
	 */
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
		pabort("can't set spi mode");

	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
		pabort("can't get spi mode");

	/*
	 * bits per word
	 */
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't set bits per word");

	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't get bits per word");

	/*
	 * max speed hz
	 */
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't set max speed hz");

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't get max speed hz");

	printf("spi mode: %d\n", mode);
	printf("bits per word: %d\n", bits);
	printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);

	//struct timespec time_100ms;
	//time_100ms.tv_sec = 0;
	//time_100ms.tv_nsec = 1*1000000;//10 * 1ms

	//time_100ms.tv_nsec = 1*5;//10 * 1ms
	struct spi_state s;
	spi_proto_initialize(&s);
	first_loop();
	for (;;){//(int i = 0; i < 500; i++) {
		loop(&s, fd);
		//nanosleep(&time_100ms, NULL);
	}
	//transfer(fd);

	close(fd);

	return ret;
}

void
linux_test_callback(struct spi_packet *p)
{
	if (!p->msg[0]) return;
	char strbuf[33];
	memcpy(strbuf, p->msg, 32);
	strbuf[32] = 0;
	printf("got msg:[ %s ]\n", strbuf);
	return;
	//TODO print it out or something
	uint8_t *s = (uint8_t *) p;
	for (int i = 0; i < sizeof(struct spi_packet);i++)
		printf("%02x ", s[i]);
	printf("\n");
}


void
print_bytes(unsigned char *b, int n)
{
	for (int i = 0; i < n;i++)
		printf("%02x ", b[i]);
	printf("\n");
}
void
first_loop(void)
{
	//TODO set up communications and initialize SPI
}
void
loop(struct spi_state *s, int spi_fd)
{
	/* TODO loop:
		do whatever testing processing (message sending, for example)
		marshal messages into buffers
		do the transaction
		process the received message
	*/
	
	
	//load messages into queue if any
	int N = 10;
	unsigned char m2s[10] = {2,3,5,7,11,13,17,19,23, 29};
	// TODO send other messages
	while(!spi_proto_send_msg(s, m2s, N));
	
	//message sending
	spi_proto_prep_msg(s, spi_out_buf, SPI_TRANSFER_LEN);
	
	//edbug output
	//puts("sending");
//	print_bytes(spi_out_buf, SPI_TRANSFER_LEN);
	
	//do transaction
	transfer(spi_fd);
	
	//process buffer into struct
	struct spi_packet pack;
	memcpy(&pack, spi_in_buf, SPI_TRANSFER_LEN);
	//TODO maybe fixup the CRC byte order?
	
	//process received message
	//puts("before rcv_msg");
	spi_proto_rcv_msg(s, &pack, linux_test_callback);
	//puts("after rcv_msg");	
//	print_spi_state(s);
}
