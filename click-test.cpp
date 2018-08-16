//causes solenoids to click in strobing sequence. Remote API test

//TODO centralize in a config file
#define ADC_NUM 1
#define GPIO_NUM 2
#define DAC_NUM 2
#define SOLENOID_NUM 8
#define FLOW_NUM 1

#include <stdint.h>
#include <stddef.h>

#include <thread>

#include <string.h>

extern "C" {
#include "spi_proto.h"
#include "spi_proto_lib/spi_chunks.h"
#include "spi_proto_lib/spi_chunk_defines.h"
#include "binary_semaphore.h"
#include "spi_remote.h"
#include "spi_remote_host.h"
}
#include "master_spi_proto.h"

#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#include <fcntl.h>		/* For O_RDWR */

using namespace spi_proto;


//TODO move these to config - variables that define peripherals on the tiny or are needed for remote
#define NUM_WAIT_CHUNKS 10
struct waiting_chunk wait_chunks[NUM_WAIT_CHUNKS] = {0};
//void send_chunk(void*, int);
int send_chunk(uint8_t*, size_t);

static const char *device = "/dev/spidev0.0";
static uint8_t mode;
static uint8_t bits = 8;
static uint32_t speed = 1 << 23;
static uint16_t delay;

//TODO centralize
#define TRANSFER_SIZE 36
#define SPI_PAYLOAD_LEN 32

void
click_task(void);
void
remote_task(void);

struct host_remote remote;

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

int main(int argc, char *argv[]) {
	std::thread remote_thread(remote_task);
	std::thread click_thread(click_task);
	
	while (1) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	
	return 0;
	
}

//functions implemented for port
void
delay_ms(unsigned int ms)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

uint32_t
remote_read_adc(unsigned int ix)
{
	uint8_t buf[4] = {4, CHUNK_TYPE_ADC, ix, OP_GET};
	send_chunk(buf, 4);
	bisem_wait(&remote.adc[ix].sem);
	return remote.adc[ix].last_read;
}

//TODO this should be extended to all types and it should 
int gpio_A_7;
void
remote_gpio_set(int gpio, int on)
{
	//TODO double check message format here
	uint8_t buf[5] = {5, CHUNK_TYPE_GPIO, gpio, OP_SET, on};
	send_chunk(buf, 5);
	bisem_wait(&remote.gpio[gpio].sem);
	return;
}

void
click_task(void)
{
	//enable 24V rail
	puts("enabling 24V!");
	remote_gpio_set(gpio_A_7, 1); //GPIO_SetPinsOutput(GPIOA, 1U<<7U);
	
	
	//module logic:
	//wait for start message
	//begin pressurizing
	//when pressurized, stop pressurizing and send the "I'm sealed" message to SoM code
	int wait = 200;
	uint8_t solenoid_0 = 8;
	puts("starting clicking!");
	for (;;) {
		for (int i = 0; i < SOLENOID_NUM; i++) {
			remote_gpio_set(solenoid_0 + i, 1);
			delay_ms(wait);
		}
		delay_ms(wait);
		puts("clicked");
		for (int i = 0; i < SOLENOID_NUM; i++) {
			remote_gpio_set(solenoid_0 + i, 0);
			delay_ms(wait);
		}
		delay_ms(wait);
		puts("unclicked");
	}
}

int
click_chunk_handler(uint8_t *b, size_t len)
{
	remote_chunk_handler(&remote, b, len);
}

void
remote_handler(struct host_remote *r, struct spi_packet *p)
{
	spi_msg_chunks(p->msg, SPI_PAYLOAD_LEN, click_chunk_handler);
}

void
click_remote(struct spi_packet *p)
{
	puts("click remote!");
	remote_handler(&remote, p);
}

int
send_chunk(uint8_t *buf, size_t len)
{
	//find an open waiting_chunk in waiting_chunks and copy it in
	for (int i = 0; i < NUM_WAIT_CHUNKS; i++) {
		if (!wait_chunks[i].ready_to_pack) {
			memcpy(wait_chunks[i].buf, buf, len);
			wait_chunks[i].buf[0] = len; // just in case
			wait_chunks[i].ready_to_pack = 1;
			return 0;
		}
	}
	puts("SENDING A CHUNK FAILED!");
	return -1;
}

namespace spi_proto {
int
master_send_message(struct master_spi_proto &p, unsigned char *buf, unsigned int len)
{
	return spi_proto_send_msg(&p.proto, buf, len);
}
}

int
prepare_master_chunks(void)
{
	uint8_t buf[SPI_MSG_PAYLOAD_LEN] = {0};
	int ret2, ret1 = chunk_packer(wait_chunks, NUM_WAIT_CHUNKS, buf, SPI_MSG_PAYLOAD_LEN);
	if (!ret1) puts("chunk_packer didn't pack anything!");
	if (ret1) // don't send empty packets as though they're real
		ret2 = master_send_message(spi_proto::p, buf, SPI_MSG_PAYLOAD_LEN);
	return ret1|ret2;
}

void
remote_task(void)
{
	int spi_fd = open(device, O_RDWR);
	unsigned char recvbuf[TRANSFER_SIZE];
	unsigned char sendbuf[TRANSFER_SIZE] = {};
	//struct spi_state *s = &spi_state;
	spi_proto_master_initialize(&spi_proto::p);
	
	int count = 0;
	bool closed = 0;
	while (!closed) {
		prepare_master_chunks();
		int ret = spi_proto_prep_msg(&spi_proto::p.proto, sendbuf, TRANSFER_SIZE);
		printf("ret was %d\n", ret);	
		printf("OUT\t");
		for (int i = 0; i < 16; i++) printf("%02x ", sendbuf[i]);
		puts("");
		//do SPI communication
		int spi_tr_res = spi_transfer(spi_fd, sendbuf, recvbuf, TRANSFER_SIZE);
		printf("IN\t");
		for (int i = 0; i < 16; i++) printf("%02x ", recvbuf[i]);
		puts("");
		struct spi_packet pack;
		memcpy(&pack, recvbuf, TRANSFER_SIZE);
		spi_proto_rcv_msg(&spi_proto::p.proto, &pack, click_remote);
		
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		++count;
	}
}
namespace spi_proto {
void
spi_proto_master_initialize(struct master_spi_proto *s)
{
	spi_proto_initialize(&s->proto);
}
}
