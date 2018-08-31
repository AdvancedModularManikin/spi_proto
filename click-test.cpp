//g++ click-test.cpp spi_proto_master.cpp spi_proto_lib/spi_chunks.cpp -std=c++14 -pthread -x c binary_semaphore.c -x c spi_proto_lib/spi_proto.c -I. -Ispi_proto_lib/ -x c crc16.c -x c spi_remote_host.c -o click -g
//causes solenoids to click in strobing sequence. Remote API test
#include <thread>

extern "C" {
#include "spi_proto.h"
#include "binary_semaphore.h"
#include "spi_remote.h"
#include "spi_remote_host.h"
}
#include "spi_proto_master.h"

using namespace spi_proto;

void
click_task(void);

int main(int argc, char *argv[]) {
	host_remote_init(&remote);
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

void
click_task(void)
{
	//enable 24V rail
	puts("enabling 24V!");
	remote_set_gpio(15, 1); //15 is 24V rail
	
	
	//module logic:
	//wait for start message
	//begin pressurizing
	//when pressurized, stop pressurizing and send the "I'm sealed" message to SoM code
	int wait = 200;
	uint8_t solenoid_0 = 7;
	puts("starting clicking!");
	for (;;) {
		for (int i = 0; i < SOLENOID_NUM; i++) {
			remote_set_gpio(solenoid_0 + i, 1);
			delay_ms(wait);
		}
		delay_ms(wait);
		puts("clicked");
		for (int i = 0; i < SOLENOID_NUM; i++) {
			remote_set_gpio(solenoid_0 + i, 0);
			delay_ms(wait);
		}
		delay_ms(wait);
		puts("unclicked");
	}
}
