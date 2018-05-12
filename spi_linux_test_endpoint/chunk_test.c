#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../spi_proto.h"
#include "../../amm-tiny/source/spi_chunks.h"
#include "../../amm-tiny/source/spi_chunk_defines.h"

#define SPI_TRANSFER_LEN (SPI_MSG_PAYLOAD_LEN+4)
#define SPI_TRANSFER_SIZE SPI_TRANSFER_LEN

void transfer(int fd);

uint8_t spi_in_buf[SPI_TRANSFER_SIZE], spi_out_buf[SPI_TRANSFER_SIZE];


int
master_chunk_handler(uint8_t *buf, size_t len)
{
	if (len < 2) return -1; // length zero isn't a real chunk, length 1 can't carry data
	switch(buf[1]) {
	case CHUNK_TYPE_VALVE:
		if (len < 4) break; // TODO maybe increment some error counter somewhere
		// [LEN|TYPE|ID|CMD]
		printf("VALVE:\t%02x %02x %02x %02x\n", buf[0], buf[1], buf[2], buf[3]);
		break;
	case CHUNK_TYPE_GPIO:
		if (len < CHUNK_LEN_GPIO) break; // TODO log bad chunk counter
		// [LEN|TYPE|ID|CMD]
		printf("GPIO:\t%02x %02x %02x %02x\n", buf[0], buf[1], buf[2], buf[3]);
		break;
	//case CHUNK_TYPE_STRING
	default:
		puts("Unknown Chunk type.");
		unknown_chunk_type_msg_count++; // TODO think of better name
	}
	return 0;
}

void
unchunk_master(struct spi_packet *p)
{
	if (p)
		spi_msg_chunks(p->msg, SPI_MSG_PAYLOAD_LEN, master_chunk_handler);
	else
		puts("unchunk_master called with bogus arg");
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
	spi_proto_rcv_msg(s, &pack, unchunk_master);
	puts("after rcv_msg");	
//	print_spi_state(s);
}
