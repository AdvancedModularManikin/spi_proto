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
void
test_spi_proto_initialize(void)
{	
	struct spi_state master, slave;
	
	//need function because of some invariants
	spi_proto_initialize(&master);
	spi_proto_initialize(&slave);
	
	printf("%s: post init\n", __func__);
	
	//create buffers
	struct spi_packet master_to_slave, slave_to_master;
	
	//prepare message
	int master_write_ret = spi_proto_prep_msg(&master, &master_to_slave, SPI_PACKET_LEN);
	int slave_write_ret = spi_proto_prep_msg(&slave, &slave_to_master, SPI_PACKET_LEN);
	printf ("%s: post prep\n", __func__);
	
	//receive messages
	spi_proto_rcv_msg(&slave, &master_to_slave, NULL);
	spi_proto_rcv_msg(&master, &slave_to_master, NULL);
	printf ("%s: post rcv\n", __func__);
	
	//TODO print out data structures
	print_spi_state(&master);
	print_spi_state(&slave);
}

void
test_spi_proto_one_round(unsigned char *m2s, unsigned char *s2m, size_t len)
{
	struct spi_state master, slave;
	
	//need function because of some invariants
	spi_proto_initialize(&master);
	spi_proto_initialize(&slave);
	
	
	puts("MASTER after init");
	print_spi_state_full(&master);
	
	//send m2s from master to slave, and vice versa.
	//TODO crash if len > SPI_PACKET_LEN
	assert(len <= SPI_PACKET_LEN);

	//create buffers
	struct spi_packet master_to_slave, slave_to_master;
	
	puts("after creation");
	print_spi_packet(&master_to_slave);
	
	puts("after clear");
	memset(&master_to_slave, 0, sizeof(master_to_slave));
	print_spi_packet(&master_to_slave);
	
	int msend = spi_proto_send_msg(&master, m2s, len);
	int ssend = spi_proto_send_msg(&slave, s2m, len);
	
	puts("MASTER after send");
	print_spi_state_full(&master);
	
	assert(msend != -1);
	assert(ssend != -1);
	
	//prepare message
	int master_write_ret = spi_proto_prep_msg(&master, &master_to_slave, SPI_PACKET_LEN);
	int slave_write_ret = spi_proto_prep_msg(&slave, &slave_to_master, SPI_PACKET_LEN);
	
	puts("after write");
	print_spi_packet(&master_to_slave);
	
	//receive messages
	spi_proto_rcv_msg(&slave, &master_to_slave, NULL);
	spi_proto_rcv_msg(&master, &slave_to_master, NULL);
	
	//TODO extract messages
	//TODO compare with sent messages
	
	puts("after sending");
	print_spi_packet(&master_to_slave);
	print_spi_packet(&slave_to_master);
	
	//print out data structures
	print_spi_state(&master);
	//print_spi_state(&slave);
	
}

int
test_spi_longer(unsigned int rounds)
{
	/*TODO
		initialize
		begin loop
			feed messages into queue until queue full message received
			do a step
			handle results and do output
	*/
	
	struct spi_state master, slave;
	
	//need function because of some invariants
	spi_proto_initialize(&master);
	spi_proto_initialize(&slave);
	
	int N = 10;
	unsigned char m2s[10] = {1,2,3,4,5,6,7,8,9,10};
	unsigned char s2m[10] = {2,3,5,7,11,13,17,19,23, 29};
	while (rounds--) {
		//TODO feed messages into each queue
		while(!spi_proto_send_msg(&master, m2s, N));
		while(!spi_proto_send_msg(&slave, s2m, N));
		
		//CHECK step transmission
		
		//create buffers
		struct spi_packet master_to_slave, slave_to_master;
	
		//prepare message
		int master_write_ret = spi_proto_prep_msg(&master, &master_to_slave, SPI_PACKET_LEN);
		int slave_write_ret = spi_proto_prep_msg(&slave, &slave_to_master, SPI_PACKET_LEN);
	
		//receive messages
		spi_proto_rcv_msg(&slave, &master_to_slave, NULL);
		spi_proto_rcv_msg(&master, &slave_to_master, NULL);
		
		//TODO do output
		print_spi_state(&master);
		puts("------------------");
	}
	return 0;
}

int
test_spi_longer_some_noise(unsigned int rounds, float noise_chance)
{
	/*TODO
		initialize
		begin loop
			feed messages into queue until queue full message received
			do a step
			handle results and do output
	*/
	srand(0x14411441);
	
	struct spi_state master, slave;
	
	//need function because of some invariants
	spi_proto_initialize(&master);
	spi_proto_initialize(&slave);
	
	int N = 10;
	unsigned char m2s[10] = {1,2,3,4,5,6,7,8,9,10};
	unsigned char s2m[10] = {2,3,5,7,11,13,17,19,23, 29};
	while (rounds--) {
		//TODO feed messages into each queue
		while(!spi_proto_send_msg(&master, m2s, N));
		while(!spi_proto_send_msg(&slave, s2m, N));
		
		//CHECK step transmission
		
		//create buffers
		struct spi_packet master_to_slave, slave_to_master;
	
		//prepare message
		int master_write_ret = spi_proto_prep_msg(&master, &master_to_slave, SPI_PACKET_LEN);
		int slave_write_ret = spi_proto_prep_msg(&slave, &slave_to_master, SPI_PACKET_LEN);
	
		int rand_cap = ((float)RAND_MAX) * noise_chance;
		if (rand() < rand_cap) {
			master_to_slave.msg[5] ^= 0xff;
		}
		if (rand() < rand_cap) {
			slave_to_master.msg[5] ^= 0xff;
		}
		//receive messages
		spi_proto_rcv_msg(&slave, &master_to_slave, NULL);
		spi_proto_rcv_msg(&master, &slave_to_master, NULL);
		
		//TODO do output
		print_spi_state(&master);
		puts("------------------");
	}
	return 0;
}


#define BIG_ROUNDS 5
unsigned char master_result_msg[BIG_ROUNDS*SPI_MSG_PAYLOAD_LEN];
unsigned char slave_result_msg[BIG_ROUNDS*SPI_MSG_PAYLOAD_LEN];
void
reconstruct_slave_msg(struct spi_packet *p)
{
	static int ix = 0;
	//printf("reconstructing slave: %d\n", ix);
	//print_bytes(p, sizeof(struct spi_packet));
	if (p->magic == SPI_PROTO_MAGIC_REAL)
		memcpy(master_result_msg + ix*SPI_MSG_PAYLOAD_LEN, &p->msg, SPI_MSG_PAYLOAD_LEN);
	ix++;
}
void
reconstruct_master_msg(struct spi_packet *p)
{
	static int ix = 0;
	//printf("reconstructing master: %d\n", ix);
	//print_bytes(p, sizeof(struct spi_packet));
	if (p->magic == SPI_PROTO_MAGIC_REAL)
		memcpy(slave_result_msg + ix*SPI_MSG_PAYLOAD_LEN, &p->msg, SPI_MSG_PAYLOAD_LEN);
	//print_bytes(slave_result_msg, BIG_ROUNDS*SPI_MSG_PAYLOAD_LEN);
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
	const int fnum = 10;
	int fail_m[10] = {1,0,0,0,0, 0,0,0,0,0};
	int fail_s[10] = {1,0,0,0,0, 0,0,0,0,0};
	//need function because of some invariants
	spi_proto_initialize(&master);
	spi_proto_initialize(&slave);
	
	unsigned char master_source_msg[BIG_ROUNDS*SPI_MSG_PAYLOAD_LEN];
	unsigned char slave_source_msg[BIG_ROUNDS*SPI_MSG_PAYLOAD_LEN];
	for (int i = 0; i < BIG_ROUNDS*SPI_MSG_PAYLOAD_LEN; i++) {
		master_source_msg[i] = rand();
		slave_source_msg[i] = rand();
	} 
	for (int i = 0; i < BIG_ROUNDS; i++) {
		//feed messages into each queue
		int ret = spi_proto_send_msg(&master, master_source_msg + i*SPI_MSG_PAYLOAD_LEN, SPI_MSG_PAYLOAD_LEN);
		int ret2 = spi_proto_send_msg(&slave, slave_source_msg + i*SPI_MSG_PAYLOAD_LEN, SPI_MSG_PAYLOAD_LEN);
		printf("r1 %d r2 %d\n", ret, ret2);
	}
	unsigned int rix = 0;
	while (rounds--) {
		
		
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
		
		if (fail_m[rix]) {
			master_to_slave.msg[5] ^= 0xff;
		}
		if (fail_s[rix]) {
			slave_to_master.msg[5] ^= 0xff;
		}
		rix++;
		rix %= fnum;
		
		//receive messages
		spi_proto_rcv_msg(&slave, &master_to_slave, reconstruct_master_msg);
		spi_proto_rcv_msg(&master, &slave_to_master, reconstruct_slave_msg);
		
		//TODO do output
		
		puts("master state");
		print_spi_state(&master);
		//*
		puts("master received:");
		print_bytes(&slave_to_master, sizeof(slave_to_master), -1);
		puts("slave received:");
		print_bytes(&master_to_slave, sizeof(master_to_slave), -1);
		puts("------------------");
		//*/
		
	}
	puts("total transmit m->s:");
	print_bytes(master_source_msg, BIG_ROUNDS*SPI_MSG_PAYLOAD_LEN, SPI_MSG_PAYLOAD_LEN);
	uint16_t m_s_crc = crc16_block(0, master_source_msg, BIG_ROUNDS*SPI_MSG_PAYLOAD_LEN);
	puts("...");
	print_bytes(slave_result_msg, BIG_ROUNDS*SPI_MSG_PAYLOAD_LEN, SPI_MSG_PAYLOAD_LEN);
	uint16_t s_r_crc = crc16_block(0, slave_result_msg, BIG_ROUNDS*SPI_MSG_PAYLOAD_LEN);
	printf("%04x should = %04x\n", m_s_crc, s_r_crc);
	
	puts("total transmit s->m:");
	print_bytes(slave_source_msg, BIG_ROUNDS*SPI_MSG_PAYLOAD_LEN, SPI_MSG_PAYLOAD_LEN);
	uint16_t s_s_crc = crc16_block(0, slave_source_msg, BIG_ROUNDS*SPI_MSG_PAYLOAD_LEN);
	puts("...");
	print_bytes(master_result_msg, BIG_ROUNDS*SPI_MSG_PAYLOAD_LEN, SPI_MSG_PAYLOAD_LEN);
	uint16_t m_r_crc = crc16_block(0, master_result_msg, BIG_ROUNDS*SPI_MSG_PAYLOAD_LEN);
	printf("%04x should = %04x\n", s_s_crc, m_r_crc);
	
	return 0;
}

int
main(int argc, char *argv[])
{
	//TODO run the tests
	
	//test_spi_proto_initialize();
	puts("test_spi_proto_initialize done");
	unsigned char m2s[10] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a};
	unsigned char s2m[10] = {0x0a, 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01};
	int n = 10;
	
	
	//puts("one round start:");
	//test_spi_proto_one_round(m2s, s2m, n);
	//puts("one round end.");
	
	
	//test_spi_longer(3);
	
	//test_spi_longer_some_noise(90, 0.1);
	
	//this test sends a large amount of arbitrary memory over the link with some noise and reconstructs it
	test_big_transmit_noise(7, 0.1);
	
	//test_spi_confirm(3);
}
