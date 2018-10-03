//g++ click-test.cpp spi_proto_master.cpp spi_proto_lib/spi_chunks.cpp -std=c++14 -pthread -x c binary_semaphore.c -x c spi_proto_lib/spi_proto.c -I. -Ispi_proto_lib/ -x c crc16.c -x c spi_remote_host.c -o click -g
//causes solenoids to click in strobing sequence. Remote API test
#include <thread>

extern "C" {
#include "spi_proto.h"
#include "spi_remote.h"
#include "spi_remote_host.h"
}
#include "spi_proto_master_datagram.h"

using namespace spi_proto;

void
blip_task(void);

int main(int argc, char *argv[]) {
	std::thread remote_thread(datagram_task);
	std::thread click_thread(blip_task);
	
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

void
blip_task(void)
{
	//forever:
	//	send heartrate 60
	//	wait 10s
	//	send heartrate 120
	//	wait 10s

	int wait = 10*1000;
	puts("starting blinking!");
	for (;;) {
		puts("blink slow");
		send_message({60},1);
		delay_ms(wait);
		puts("blink fast");
		send_message({120},1);
		delay_ms(wait);
	}
}
