#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <time.h>

#include "spi_proto.h"
#include "config.h"
#include "misc/crc16.h"
#include "test/test_util.h"

//SPI protocol testing

#define BIG_ROUNDS 7
unsigned char master_result_msg[BIG_ROUNDS*SPI_MSG_PAYLOAD_LEN];
unsigned char slave_result_msg[BIG_ROUNDS*SPI_MSG_PAYLOAD_LEN];
void
reconstruct_slave_msg(struct spi_packet *p)
{
	static int ix = 0;
	int msg_real = p->magic == SPI_PROTO_MAGIC_REAL;
	//printf("reconstructing slave: %d\n", ix);
	//print_bytes(p, sizeof(struct spi_packet));
	if (ix < BIG_ROUNDS)
		memcpy(master_result_msg + ix*SPI_MSG_PAYLOAD_LEN, &p->msg, SPI_MSG_PAYLOAD_LEN);
	ix++;
}
void
reconstruct_master_msg(struct spi_packet *p)
{
	static int ix = 0;
	int msg_real = p->magic == SPI_PROTO_MAGIC_REAL;
	printf("reconstructing master: %d real? %d\n", ix, msg_real);
	print_bytes(p, sizeof(struct spi_packet), -1);
	if (ix < BIG_ROUNDS)
		memcpy(slave_result_msg + ix*SPI_MSG_PAYLOAD_LEN, &p->msg, SPI_MSG_PAYLOAD_LEN);
	print_bytes(slave_result_msg, BIG_ROUNDS*SPI_MSG_PAYLOAD_LEN, SPI_MSG_PAYLOAD_LEN);
	ix++;
}
int
test_big_transmit_noise(int rounds, float noise_chance)
{
	/*TODO
		initialize
		begin loop
			feed messages into queue until queue full message received
			do a step
			handle results and do output
	*/
	
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
	
	unsigned char master_source_msg[BIG_ROUNDS*SPI_MSG_PAYLOAD_LEN];
	unsigned char slave_source_msg[BIG_ROUNDS*SPI_MSG_PAYLOAD_LEN];
	for (int i = 0; i < BIG_ROUNDS*SPI_MSG_PAYLOAD_LEN; i++) {
		master_source_msg[i] = rand();
		slave_source_msg[i] = rand();
	} 
	for (int i = 0; i+2 < BIG_ROUNDS; i++) {
		//feed messages into each queue
		int ret = spi_proto_send_msg(&master, master_source_msg + i*SPI_MSG_PAYLOAD_LEN, SPI_MSG_PAYLOAD_LEN);
		int ret2 = spi_proto_send_msg(&slave, slave_source_msg + i*SPI_MSG_PAYLOAD_LEN, SPI_MSG_PAYLOAD_LEN);
		printf("r1 %d r2 %d\n", ret, ret2);
	}
	unsigned int rix = 0;
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
		
		/*
		int rand_cap = ((float)RAND_MAX) * noise_chance;
		if (rand() < rand_cap) {
			master_to_slave.msg[5] ^= 0xff;
		}
		if (rand() < rand_cap) {
			slave_to_master.msg[5] ^= 0xff;
		}
		*/
		
		if (rix < fnum && fail_m[rix]) {
			master_to_slave.msg[5] ^= 0xff;
		}
		if (rix < fnum && fail_s[rix]) {
			slave_to_master.msg[5] ^= 0xff;
		}
		rix++;
		
		//receive messages
		spi_proto_rcv_msg(&slave, &master_to_slave, reconstruct_master_msg);
		spi_proto_rcv_msg(&master, &slave_to_master, reconstruct_slave_msg);
		
		//TODO do output
		/*
		puts("master state");
		print_spi_state(&master);
		*/
		/*
		puts("master received:");
		print_bytes(&slave_to_master, sizeof(slave_to_master), -1);
		puts("slave received:");
		print_bytes(&master_to_slave, sizeof(master_to_slave), -1);
		puts("------------------");
		//*/
		
	}
	puts("total transmit m->s:");
	print_bytes(master_source_msg, BIG_ROUNDS*SPI_MSG_PAYLOAD_LEN,SPI_MSG_PAYLOAD_LEN);
	uint16_t m_s_crc = crc16_block(0, master_source_msg, BIG_ROUNDS*SPI_MSG_PAYLOAD_LEN);
	puts("...");
	print_bytes(slave_result_msg, BIG_ROUNDS*SPI_MSG_PAYLOAD_LEN,SPI_MSG_PAYLOAD_LEN);
	uint16_t s_r_crc = crc16_block(0, slave_result_msg, BIG_ROUNDS*SPI_MSG_PAYLOAD_LEN);
	printf("%04x should = %04x\n", m_s_crc, s_r_crc);
	
	puts("total transmit s->m:");
	print_bytes(slave_source_msg, BIG_ROUNDS*SPI_MSG_PAYLOAD_LEN,SPI_MSG_PAYLOAD_LEN);
	uint16_t s_s_crc = crc16_block(0, slave_source_msg, BIG_ROUNDS*SPI_MSG_PAYLOAD_LEN);
	puts("...");
	print_bytes(master_result_msg, BIG_ROUNDS*SPI_MSG_PAYLOAD_LEN,SPI_MSG_PAYLOAD_LEN);
	uint16_t m_r_crc = crc16_block(0, master_result_msg, BIG_ROUNDS*SPI_MSG_PAYLOAD_LEN);
	printf("%04x should = %04x\n", s_s_crc, m_r_crc);
	
	return 0;
}

int
main(int argc, char *argv[])
{
	//run a test to monitor behavior of message position (seq, ack) after payload is delivered
	//this test sends a large amount of arbitrary memory over the link with some noise and reconstructs it
	//15 ok
	//16 not ok
	//20 not ok
	
	test_big_transmit_noise(15, 0.1);
}
