//chunk test
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <time.h>

#include "spi_proto.h"
#include "spi_proto_util.h"
#include "config.h"
#include "misc/crc16.h"
#include "../amm-tiny/source/spi_chunks.h"

#include "test/test_util.h"

#define BIG_ROUNDS 5

//use a test chunk that contains a start location and a number of bytes to copy there and use this to reconstruct the message.
struct test_chunk {
	unsigned char *b;
	size_t len; // less than 32
};

//TODO currently has problem: designed so that send_chunk takes buffers with the length as the first element. instead it should take a buffer B and a length L and write L then L-1 bytes of B.
int chunk_min_len = 0, chunk_max_len = SPI_MSG_PAYLOAD_LEN;

#define NUM_WAIT_CHUNKS 10
struct waiting_chunk master_wait_chunks[NUM_WAIT_CHUNKS] = {0};//{{0},0}};

int
master_send_chunk(uint8_t *buf, size_t len)
{
	//find an open waiting_chunk in waiting_chunks and copy it in
	for (int i = 0; i < NUM_WAIT_CHUNKS; i++) {
		if (!master_wait_chunks[i].ready_to_pack) {
			memcpy(&master_wait_chunks[i].buf[1], buf, len);
			master_wait_chunks[i].buf[0] = len;
			master_wait_chunks[i].ready_to_pack = 1;
			return 0;
		}
	}
	return -1;
}

struct waiting_chunk slave_wait_chunks[NUM_WAIT_CHUNKS] = {0};

int
slave_send_chunk(uint8_t *buf, size_t len)
{
	//find an open waiting_chunk in waiting_chunks and copy it in
	for (int i = 0; i < NUM_WAIT_CHUNKS; i++) {
		if (!slave_wait_chunks[i].ready_to_pack) {
			memcpy(&slave_wait_chunks[i].buf[1], buf, len);
			slave_wait_chunks[i].buf[0] = len;
			slave_wait_chunks[i].ready_to_pack = 1;
			return 0;
		}
	}
	return -1;
}
int
num_free_chunks(struct waiting_chunk *c, size_t n)
{
	int ret = 0;
	for (unsigned int i = 0; i < n; i++) {
		ret += c[i].ready_to_pack == 0;
	}
	return ret;
}

//FIXED somehow the first chunk written is getting its length put into the spi_state num_sent_but_unconfirmed.
//TODO The issue was in the amm spi_proto, so it's basically junk at this point. Ice it
void
prepare_chunks(struct spi_state *s, unsigned char *msg, size_t msg_len, struct waiting_chunk *wait_chunks, size_t wait_len,
	int (*send_chunk)(uint8_t *buf, size_t len))
{
	unsigned int send_ix = 0;
	unsigned int round = 0;
	puts("before while");
	while (send_ix < msg_len) {
		if (round > 5) {
			while (num_free_chunks(wait_chunks, wait_len) < 5) {
				printf("num free: %d\n", num_free_chunks(wait_chunks, wait_len));
				unsigned char buf[SPI_MSG_PAYLOAD_LEN];
				memset(buf, 0, SPI_MSG_PAYLOAD_LEN);
				int written = chunk_packer(wait_chunks, wait_len, buf, SPI_MSG_PAYLOAD_LEN);
				printf("wrote %d bytes\n", written);
				assert(written);
				puts("sending buf:");
				print_bytes(buf, SPI_MSG_PAYLOAD_LEN, -1);
				puts("prepare chunks per occs:");
				print_spi_occs(s);
				puts("prepare_chunks pre:");
				int ret = spi_proto_send_msg(s, buf, SPI_MSG_PAYLOAD_LEN);
				puts("prepare_chunks post:");
	
				printf("and ret was: %d\n", ret);
			}
			round = 0;
		}
		
		int this_chunk_len = (rand() % (chunk_max_len - chunk_min_len)) + chunk_min_len;
		printf("this_chunk_len: %d\n", this_chunk_len);
		if (!this_chunk_len) {
			send_chunk(NULL, 0);
			send_ix++;
		} else {
			send_chunk(&msg[send_ix], this_chunk_len);
			send_ix += this_chunk_len-1;
		}
		round++;
	}
}

void
master_cb(struct spi_packet *p)
{
	printf("[MASTER] got a packet!\n");
	print_bytes(p, sizeof(struct spi_packet), -1);
}
void
slave_cb(struct spi_packet *p)
{
	printf("[SLAVE] got a packet!\n");
	print_bytes(p, sizeof(struct spi_packet), -1);
}

void
test_chunks(int rounds)
{
	//TODO create spi protos
	//TODO push several packets worth of chunks in
	//TODO do several rounds
	
	struct spi_state master, slave;
	
	//srand(time(NULL));
	const int fnum = 15;
	//working
	//int fail_m[10] = {1,0,0,0,0, 0,0,0,0,0};
	//int fail_s[10] = {1,0,0,0,0, 0,0,0,0,0};
	
	int fail_m[15] = {1,1,1,0,0, 0,0,0,0,0, 0,0,0,0,0};
	int fail_s[15] = {1,0,1,0,1, 0,0,0,0,0, 0,0,0,0,0};
	//need function because of some invariants
	spi_proto_initialize(&master);
	spi_proto_initialize(&slave);
	puts("after init");
	printf("master.num_sent_but_unconfirmed %d\n",master.num_sent_but_unconfirmed);
	int master_chunk_sizes[10] = {0};
	int slave_chunk_sizes[10] = {0};
	int slave_chunk_ix = 0;
	
	puts("before making source messages");
	const int BYTE_AMOUNT = BIG_ROUNDS*SPI_MSG_PAYLOAD_LEN;
	unsigned char master_source_msg[BIG_ROUNDS*SPI_MSG_PAYLOAD_LEN];
	unsigned char slave_source_msg[BIG_ROUNDS*SPI_MSG_PAYLOAD_LEN];
	for (int i = 0; i < BIG_ROUNDS*SPI_MSG_PAYLOAD_LEN; i++) {
		master_source_msg[i] = rand();
		slave_source_msg[i] = rand();
	}
	prepare_chunks(&master, master_source_msg, BYTE_AMOUNT, master_wait_chunks, NUM_WAIT_CHUNKS, master_send_chunk);
	prepare_chunks(&slave, slave_source_msg, BYTE_AMOUNT, slave_wait_chunks, NUM_WAIT_CHUNKS, slave_send_chunk);
	puts("post prepare chunks");
	printf("master.num_sent_but_unconfirmed %d\n",master.num_sent_but_unconfirmed);
	
	/*
	for (int i = 0; i+2 < BIG_ROUNDS; i++) {
		//feed messages into each queue
		int ret = spi_proto_send_msg(&master, master_source_msg + i*SPI_MSG_PAYLOAD_LEN, SPI_MSG_PAYLOAD_LEN);
		int ret2 = spi_proto_send_msg(&slave, slave_source_msg + i*SPI_MSG_PAYLOAD_LEN, SPI_MSG_PAYLOAD_LEN);
		printf("r1 %d r2 %d\n", ret, ret2);
	}
	*/
	unsigned int rix = 0;
	puts("before loop");
	printf("master.num_sent_but_unconfirmed %d\n",master.num_sent_but_unconfirmed);
	while (rounds--) {
		
		if (rix == 10) {
			spi_proto_send_msg(&master, master_source_msg + 5*SPI_MSG_PAYLOAD_LEN, SPI_MSG_PAYLOAD_LEN);
			spi_proto_send_msg(&slave, slave_source_msg + 5*SPI_MSG_PAYLOAD_LEN, SPI_MSG_PAYLOAD_LEN);

			spi_proto_send_msg(&master, master_source_msg + 6*SPI_MSG_PAYLOAD_LEN, SPI_MSG_PAYLOAD_LEN);
			spi_proto_send_msg(&slave, slave_source_msg + 6*SPI_MSG_PAYLOAD_LEN, SPI_MSG_PAYLOAD_LEN);
		}
		
		//CHECK step transmission
		
		//create buffers
		struct spi_packet master_to_slave, slave_to_master;
	
		//prepare message
		int master_write_ret = spi_proto_prep_msg(&master, &master_to_slave, SPI_PACKET_LEN);
		int slave_write_ret = spi_proto_prep_msg(&slave, &slave_to_master, SPI_PACKET_LEN);
		
		if (rix < fnum && fail_m[rix]) {
			master_to_slave.msg[5] ^= 0xff;
		}
		if (rix < fnum && fail_s[rix]) {
			slave_to_master.msg[5] ^= 0xff;
		}
		rix++;
		printf("rix: %d\n", rix);
		puts("master to slave:");
		print_bytes((void*) &master_to_slave, sizeof(struct spi_packet), -1);
		
		//receive messages
		spi_proto_rcv_msg(&slave, &master_to_slave, slave_cb);
		spi_proto_rcv_msg(&master, &slave_to_master, master_cb);
		printf("master.num_sent_but_unconfirmed %d\n",master.num_sent_but_unconfirmed);
		//print_spi_state(&master);
		
		
	}
	puts("total transmit m->s:");
	print_bytes(master_source_msg, BIG_ROUNDS*SPI_MSG_PAYLOAD_LEN,SPI_MSG_PAYLOAD_LEN);
	uint16_t m_s_crc = crc16_block(0, master_source_msg, BIG_ROUNDS*SPI_MSG_PAYLOAD_LEN);
	//puts("...");
	//print_bytes(slave_result_msg, BIG_ROUNDS*SPI_MSG_PAYLOAD_LEN,SPI_MSG_PAYLOAD_LEN);
	//uint16_t s_r_crc = crc16_block(0, slave_result_msg, BIG_ROUNDS*SPI_MSG_PAYLOAD_LEN);
	//printf("%04x should = %04x\n", m_s_crc, s_r_crc);
	
	puts("total transmit s->m:");
	print_bytes(slave_source_msg, BIG_ROUNDS*SPI_MSG_PAYLOAD_LEN,SPI_MSG_PAYLOAD_LEN);
	uint16_t s_s_crc = crc16_block(0, slave_source_msg, BIG_ROUNDS*SPI_MSG_PAYLOAD_LEN);
	//puts("...");
	//print_bytes(master_result_msg, BIG_ROUNDS*SPI_MSG_PAYLOAD_LEN,SPI_MSG_PAYLOAD_LEN);
	//uint16_t m_r_crc = crc16_block(0, master_result_msg, BIG_ROUNDS*SPI_MSG_PAYLOAD_LEN);
	//printf("%04x should = %04x\n", s_s_crc, m_r_crc);
}

int
main(int argc, char **argv)
{
	test_chunks(7);
	
	return 0;
}
