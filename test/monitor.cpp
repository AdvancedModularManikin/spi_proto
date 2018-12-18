#include <stdio.h>
#include <thread>
#include <iostream>
#include "spi_datagram.h"

void delay_ms(unsigned int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

#define NUM_VALUES 16
int values[NUM_VALUES] = {0};
int trigger[2] = {0};
bool trigger_set = false;
uint idx = 0;

struct spi_packet spi_recv_msg; // Local variable holding the latest received spi message
bool spi_recv_fresh = false;    // Local flag indicating there is fresh data in spi_recv_msg

void spi_message_handler_callback(struct spi_packet *p /* this pointer is to memory managed by the SPI library */) {
	int any = 0; // there's still an issue where empty packets are passed. There's code to prevent this, it's not working apparently
	for (int i = 0; i < sizeof(SPI_MSG_PAYLOAD_LEN); i++)
		any |= p->msg[i];
	if (any) {
		/*
		for (int i = 0; i < sizeof(struct spi_packet); i++) {
			printf("%02x ", ((unsigned char *) p)[i]);
		}
		puts("");
		*/
//		printf("ADC%s_SE%d%s:\t %04d\n", p->msg[0] ? "1" : "0", p->msg[1],
//			       p->msg[2] ? "b" : "a", p->msg[3] << 8 | p->msg[4]);
		unsigned char *pchars = (unsigned char *) p;
		//for (int j = 0; j < 8; j++) printf("%02x ", pchars[j]); puts("");
		values[p->msg[5]] = p->msg[3] << 8 | p->msg[4];
		if (!(p->msg[5])) {
				for (int i=0; i<NUM_VALUES; i++) {
					printf("%04d ", values[i]);
				}
				printf("\n");
				idx = 0;
		} else {
			trigger_set = true;
			trigger[0] = p->msg[0]; trigger[1] = p->msg[1];
		}
	}
    //memcpy(&spi_recv_msg, p, sizeof(struct spi_packet));
    //spi_recv_fresh = true;
}

void (*spi_callback)(struct spi_packet *p) = spi_message_handler_callback;

// This is mostly boilerplate
int main(int argc, char *argv[]) {
	using namespace std;
    std::thread datagram_thread(datagram_task);     // Required for SPI datagram library
    //std::thread heart_rate_thread(heartrate_led_task);  // Required to run module business logic

    // Boilerplate to handle command-line arguments
    cout << "monitor running" << endl;
    int closed = 0;
    while (!closed)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    cout << "monitor closing" << endl;

    return 0;
}

