#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <time.h>

#include "spi_proto.h"
#include "spi_proto_util.h"
#include "crc16.h"
#include "test_util.h"

//SPI protocol testing
//test resynchronization
#define PUTS if(VERBOSE) puts
#define PRINTF if(VERBOSE) printf
#define PRINT_BYTES if(VERBOSE) print_bytes
#define PRINT_SPI_STATE if(VERBOSE) print_spi_state
#define VERBOSE 0

#define BIG_ROUNDS 10
unsigned char master_result_msg[BIG_ROUNDS*SPI_MSG_PAYLOAD_LEN];
unsigned char slave_result_msg[BIG_ROUNDS*SPI_MSG_PAYLOAD_LEN];
void
reconstruct_slave_msg(struct spi_packet *p)
{
	static int ix = 0;
	int msg_real = p->magic == SPI_PROTO_MAGIC_REAL;
	//PRINTF("reconstructing slave: %d\n", ix);
	//PRINT_BYTES(p, sizeof(struct spi_packet));
	if (ix < BIG_ROUNDS)
		memcpy(master_result_msg + ix*SPI_MSG_PAYLOAD_LEN, &p->msg, SPI_MSG_PAYLOAD_LEN);
	ix++;
}
void
reconstruct_master_msg(struct spi_packet *p)
{
	static int ix = 0;
	int msg_real = p->magic == SPI_PROTO_MAGIC_REAL;
	//PRINTF("reconstructing master: %d real? %d\n", ix, msg_real);
	//PRINT_BYTES(p, sizeof(struct spi_packet), -1);
	if (ix < BIG_ROUNDS)
		memcpy(slave_result_msg + ix*SPI_MSG_PAYLOAD_LEN, &p->msg, SPI_MSG_PAYLOAD_LEN);
	//PRINT_BYTES(slave_result_msg, BIG_ROUNDS*SPI_MSG_PAYLOAD_LEN, SPI_MSG_PAYLOAD_LEN);
	ix++;
}
int
test_big_transmit_noise(struct spi_state *m, struct spi_state *s, int rounds, float noise_chance)
{
	/*TODO
		initialize
		begin loop
			feed messages into queue until queue full message received
			do a step
			handle results and do output
	*/
	
	struct spi_state master = *m, slave = *s;
	
	//srand(time(NULL));
	const int fnum = 15;
	//working
	//int fail_m[10] = {1,0,0,0,0, 0,0,0,0,0};
	//int fail_s[10] = {1,0,0,0,0, 0,0,0,0,0};
	
	int fail_m[15] = {1,1,1,0,0, 0,0,0,0,0, 0,0,0,0,0};
	int fail_s[15] = {1,0,1,0,1, 0,0,0,0,0, 0,0,0,0,0};
	//need function because of some invariants
	//spi_proto_initialize(&master);
	//spi_proto_initialize(&slave);
	//slave.our_seq = 8;
	//slave.our_next_preack = 8;
	
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
		PRINTF("r1 %d r2 %d\n", ret, ret2);
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
		
		//*
		int rand_cap = ((float)RAND_MAX) * noise_chance;
		if (rand() < rand_cap) {
			master_to_slave.msg[5] ^= 0xff;
		}
		if (rand() < rand_cap) {
			slave_to_master.msg[5] ^= 0xff;
		}
		//*/
		/*
		if (rix < fnum && fail_m[rix]) {
			master_to_slave.msg[5] ^= 0xff;
		}
		if (rix < fnum && fail_s[rix]) {
			slave_to_master.msg[5] ^= 0xff;
		}
		*/
		rix++;
		
		//PRINT_SPI_STATE(&slave);
		int slave_invar = spi_proto_check_invariants(&slave);
		PRINTF("slave invariants: %d\n", slave_invar);
		if (slave_invar) PRINT_SPI_STATE(&slave);
		assert(spi_proto_check_invariants(&slave)==0);
		//receive messages
		PUTS("slave state pre rcv:");
		PRINT_SPI_STATE(&slave);
		PUTS("slave receiving");
		spi_proto_rcv_msg(&slave, &master_to_slave, reconstruct_master_msg);
		PUTS("s;ave state post rcv:");
		PRINT_SPI_STATE(&slave);
		PUTS("master receiving");
		spi_proto_rcv_msg(&master, &slave_to_master, reconstruct_slave_msg);
		
		//TODO do output
		/*
		PUTS("master state");
		PRINT_SPI_STATE(&master);
		*/
		//*
		PUTS("master received:");
		PRINT_BYTES(&slave_to_master, sizeof(slave_to_master), -1);
		PUTS("slave received:");
		PRINT_BYTES(&master_to_slave, sizeof(master_to_slave), -1);
		PUTS("------------------");
		//*/
		
		PRINTF("master rcvd_seq_repeat_count: %d\n", master.rcvd_seq_repeat_count);

		//printf("\tmaster->our_seq>\t%d\t%d\t<slave->our_next_preack\n", master.our_seq, slave.our_next_preack);
		//printf("\tslave->our_seq>  \t%d\t%d\t<master->our_next_preack\n", slave.our_seq, master.our_next_preack);
		
	}
	PUTS("total transmit m->s:");
	PRINT_BYTES(master_source_msg, BIG_ROUNDS*SPI_MSG_PAYLOAD_LEN,SPI_MSG_PAYLOAD_LEN);
	uint16_t m_s_crc = crc16_block(0, master_source_msg, BIG_ROUNDS*SPI_MSG_PAYLOAD_LEN);
	PUTS("...");
	PRINT_BYTES(slave_result_msg, BIG_ROUNDS*SPI_MSG_PAYLOAD_LEN,SPI_MSG_PAYLOAD_LEN);
	uint16_t s_r_crc = crc16_block(0, slave_result_msg, BIG_ROUNDS*SPI_MSG_PAYLOAD_LEN);
	PRINTF("%04x should = %04x\n", m_s_crc, s_r_crc);
	
	PUTS("total transmit s->m:");
	PRINT_BYTES(slave_source_msg, BIG_ROUNDS*SPI_MSG_PAYLOAD_LEN,SPI_MSG_PAYLOAD_LEN);
	uint16_t s_s_crc = crc16_block(0, slave_source_msg, BIG_ROUNDS*SPI_MSG_PAYLOAD_LEN);
	PUTS("...");
	PRINT_BYTES(master_result_msg, BIG_ROUNDS*SPI_MSG_PAYLOAD_LEN,SPI_MSG_PAYLOAD_LEN);
	uint16_t m_r_crc = crc16_block(0, master_result_msg, BIG_ROUNDS*SPI_MSG_PAYLOAD_LEN);
	PRINTF("%04x should = %04x\n", s_s_crc, m_r_crc);

	printf("master->our_seq>\t%d\t%d\t<slave->our_next_preack\n", master.our_seq, slave.our_next_preack);
	printf("slave->our_seq>  \t%d\t%d\t<master->our_next_preack\n", slave.our_seq, master.our_next_preack);
	
	return master.our_seq == slave.our_next_preack && slave.our_seq == master.our_next_preack;
}

int
main(int argc, char *argv[])
{
	//run a test to monitor behavior of message position (seq, ack) after payload is delivered
	//this test sends a large amount of arbitrary memory over the link with some noise and reconstructs it
	//15 ok
	//16 not ok
	//20 not ok
	
	//int ret = test_big_transmit_noise(32, 0.1);
	srand(time(NULL));
#define RETNUM 50
#define RETEST_COUNT 10
	int rets[RETNUM];
	memset(rets, 0, RETNUM*sizeof(int));
	for (int i = 0; i < RETNUM; i++) {
		for (int j = 0; j < RETEST_COUNT; j++) {
			struct spi_state m, s;
			spi_proto_initialize(&m);
			spi_proto_initialize(&s);
			s.our_seq = rand() % 16;
			s.our_next_preack = rand() % 16;
			s.we_sent_seq = (s.our_seq - 1) % 16;
			s.our_prev_sent_seq = (s.our_seq - 2) % 16;
			s.we_sent_preack = (s.our_next_preack - 1) % 16;
			s.our_prev_sent_preack = (s.our_next_preack - 2) % 16;
			//m.our_seq = 15 - s.our_seq;
			rets[i] += test_big_transmit_noise(&m, &s, i, 0.05);
		}
	}

	for(int i = 0; i < RETNUM; i++)
		printf("%03d ", i);
	puts("");
	for (int i = 0; i < RETNUM; i++)
		printf("%1.1f ", ((float)rets[i])/((float) RETEST_COUNT));
	puts("");

	
	//printf("return value was %d\n", ret);
}
