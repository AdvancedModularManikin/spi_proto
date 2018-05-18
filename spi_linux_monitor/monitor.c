#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../spi_proto.h"

#define SPI_TRANSFER_LEN (SPI_MSG_PAYLOAD_LEN+4)
#define SPI_TRANSFER_SIZE SPI_TRANSFER_LEN

void transfer(int fd);

uint8_t spi_in_buf[SPI_TRANSFER_SIZE], spi_out_buf[SPI_TRANSFER_SIZE];


void
spi_handler(struct spi_packet *p)
{
	//TODO
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
	//nothing to do
}
void
loop(struct spi_state *s, int spi_fd)
{
	/* loop:
		do whatever testing processing (message sending, for example)
		marshal messages into buffers
		do the transaction
		process the received message
	*/
	
	
	//load messages into queue if any
	static int mstate = 0;
	mstate %= 2;
	int N = 10;
	//two messages of various chunks
	unsigned char m2s1[10] = {2,7,2,7,1,1,1,3,7,1};
	unsigned char m2s2[8] = {4, 7, 3, 4, 4, 7, 3, 1};
	// TODO send other messages
	while(!spi_proto_send_msg(s, mstate++ ? m2s1:m2s2, N));
	
	//message sending
	spi_proto_prep_msg(s, spi_out_buf, SPI_TRANSFER_LEN);
	
	//debug output
	puts("sending");
	print_bytes(spi_out_buf, SPI_TRANSFER_LEN);
	
	//do transaction
	transfer(spi_fd);
	
	//process buffer into struct
	struct spi_packet pack;
	memcpy(&pack, spi_in_buf, SPI_TRANSFER_LEN);
	//TODO maybe fixup the CRC byte order?
	puts("Received:");
	print_bytes(spi_in_buf, SPI_TRANSFER_LEN);
	//process received message
	puts("before rcv_msg");
	spi_proto_rcv_msg(s, &pack, spi_handler);
	puts("after rcv_msg");	
//	print_spi_state(s);
}
