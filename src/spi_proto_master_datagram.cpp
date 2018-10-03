//#include <pthread.h>
#include <thread>
#include <stdint.h>
#include <stdio.h>
#include "string.h"

extern "C" {
#include "spi_proto.h"
//TODO delete these two
#include "spi_remote.h"
}
#include "spi_proto_master_datagram.h"

#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#include <fcntl.h> /* For O_RDWR */

namespace spi_proto {
	struct master_spi_proto p;
}

//TODO centralize
#define TRANSFER_SIZE 36
#define SPI_PAYLOAD_LEN 32

static const char *device = "/dev/spidev0.0";
static uint8_t mode;
static uint8_t bits = 8;
static uint32_t speed = 1 << 23;
static uint16_t delay;

int spi_transfer(int fd, const unsigned char *tx_buf, unsigned char *rx_buf, __u32 buflen) {
	int ret;
	struct spi_ioc_transfer tr = {
		tx_buf : (unsigned long) tx_buf,
		rx_buf : (unsigned long) rx_buf,
		len : TRANSFER_SIZE, speed_hz : speed,
		delay_usecs : delay, bits_per_word : bits,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)
		perror("can't send spi message");
	return ret;
}

void
datagram_task(void)
{
	int spi_fd = open(device, O_RDWR);
	unsigned char recvbuf[TRANSFER_SIZE];
	unsigned char sendbuf[TRANSFER_SIZE] = {};
	spi_proto_master_initialize(&spi_proto::p);
	
	int count = 0;
	bool closed = 0;
	puts("datagram task started!");
	while (!closed) {
		//puts("datagram preparing message");
		int ret = spi_proto_prep_msg(&spi_proto::p.proto, sendbuf, TRANSFER_SIZE);
		/*
		printf("ret was %d\n", ret);	
		printf("OUT\t");
		for (int i = 0; i < 16; i++) printf("%02x ", sendbuf[i]);
		puts("");
		*/
		//do SPI communication
		//puts("datagram performing transfer");
		int spi_tr_res = spi_transfer(spi_fd, sendbuf, recvbuf, TRANSFER_SIZE);
		//printf("transfer returned %d\n", spi_tr_res);
		/*
		printf("IN\t");
		for (int i = 0; i < 16; i++) printf("%02x ", recvbuf[i]);
		puts("");
		*/
		struct spi_packet pack;
		memcpy(&pack, recvbuf, TRANSFER_SIZE);
		//puts("datagram protocol processing");
		spi_proto_rcv_msg(&spi_proto::p.proto, &pack, spi_callback);
		
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		++count;
	}
}
namespace spi_proto {
void
spi_proto_master_initialize(struct master_spi_proto *s)
{
	spi_proto_initialize(&s->proto);
}
int
master_send_message(struct master_spi_proto &p, unsigned char *buf, unsigned int len)
{
	return spi_proto_send_msg(&p.proto, buf, len);
}
}

int
send_message(uint8_t *buf, size_t len)
{
	int ret = spi_proto::master_send_message(spi_proto::p, buf, len);
	//TODO check return value
	//puts("SENDING A MESSAGE FAILED!");
	return -1;
}
