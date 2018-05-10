//spi forwarder program
//runs as root, opens a unix socket and listens on it for fixed-length messages
//opens one SPI fd, configurable (this is the only configuration, recompile to change length)

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <sys/socket.h>
#include <sys/un.h>

#define TRANSFER_SIZE 36

static const char *device = "/dev/spidev0.0";
static uint8_t mode;
static uint8_t bits = 8;
static uint32_t speed = 8388608U; // TODO
static uint16_t delay;

static void pabort(const char *s)
{
	perror(s);
	abort();
}

static void transfer(int fd, uint8_t *sendbuf, uint8_t *rcvbuf)
{
	int ret;
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)sendbuf,
		.rx_buf = (unsigned long)rcvbuf,
		.len = TRANSFER_SIZE,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	//TODO better error handling i.e. don't just crash
	if (ret < 1)
		pabort("can't send spi message");
	
}

int
setup_spi_params(int fd)
{
	int ret;
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
}

int main(int argc, char **argv)
{
	int spi_fd = open(device, O_RDWR);
	if (spi_fd < 0) {
		pabort("unable to open SPI device"); // TODO maybe print more info like which one
	}
	//TODO acquire the SPI fd and do all IO settings first -- that seems more likely to fail
	setup_spi_params(spi_fd);
	
	int sock_fd = socket(AF_UNIX, SOCK_STREAM, 0); // TODO confirm param choices
	if (sock_fd == -1) {
		//TODO report error message and fail
	}
	
	struct sockaddr_un saddr = { AF_UNIX, "spi_forwarding_server"};
	bind(sock_fd, (struct sockaddr *)&saddr, sizeof(saddr));
	
	listen(sock_fd, 1); //only talk to one other guy. maybe should be 0?
		
	int conn_fd = accept(sock_fd, NULL, NULL);
	
	while (1) {
		uint8_t send_msg[TRANSFER_SIZE];
		uint8_t rcv_msg[TRANSFER_SIZE];
		//TODO continuously read until we have the full packet in case it's not the full size
		ssize_t amt_read = read(conn_fd, send_msg, TRANSFER_SIZE);
		if (amt_read >= 0) {
			printf("got the folling over the unix socket:\n");
			for (int i = 0; i < amt_read; i++) {
				printf("%02x", send_msg[i]);
			}		
			printf("\n");
		}
		transfer(spi_fd, send_msg, rcv_msg);
		
		//TODO do an SPI transaction
		
		write(conn_fd, rcv_msg, TRANSFER_SIZE);
	}
	
	//TODO maybe write some closing logic or something
	close(spi_fd);
	close(conn_fd);
}
