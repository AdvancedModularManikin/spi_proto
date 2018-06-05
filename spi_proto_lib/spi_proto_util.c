#include <string.h>
#include <stdint.h>
#include <stdio.h>

#ifdef CPP
extern "C" {
#endif
#include "config.h"
#include "misc/crc16.h"
#ifdef CPP
}
#endif

#include "spi_proto.h"
#include "spi_proto_util.h"

void
print_spi_state(struct spi_state *s)
{
#define PRINTIT(x) printf( #x ": %d\n", s-> x)
	if (s) {
		printf("our_seq: %d\n", s->our_seq);
		printf("our_next_preack: %d\n", s->our_next_preack);
	
		PRINTIT(we_sent_seq);
		PRINTIT(we_sent_preack);
		
		PRINTIT(last_round_rcvd_seq);
		PRINTIT(last_round_rcvd_preack);
		PRINTIT(num_sent_successfully);
	
		PRINTIT(first_unconfirmed_seq);
		PRINTIT(first_unsent_seq);
		PRINTIT(first_avail_seq);
		PRINTIT(num_unsent);
		PRINTIT(num_sent_but_unconfirmed);
		PRINTIT(num_avail);
		
	} else {
		puts("spi_state NULL!");
	}
#undef PRINTIT
}

void
print_spi_occs(struct spi_state *s)
{
#define PRINTIT(x) printf( #x ": %d\n", s-> x)
	PRINTIT(num_unsent);
	PRINTIT(num_sent_but_unconfirmed);
	PRINTIT(num_avail);
#undef PRINTIT
}

void
print_spi_state_full(struct spi_state *s)
{
	return;
	print_spi_state(s);
	if (s) {
		//TODO print out spi queue
		for (unsigned int j = 0; j < SPI_MSG_QUEUE_SIZE; j++) {
			print_spi_packet(&s->queue[j]);
		}
	}
}

void
print_spi_packet(struct spi_packet *p)
{
	return;
	unsigned char *pp = (unsigned char *) p;
	for (unsigned int i = 0; i < SPI_PACKET_LEN;i++) {
		printf("0x%02x ",pp[i]);
	}
	printf("\n");
}
