//g++ test/click-test.cpp src/spi_proto_master.cpp src/spi_chunks.cpp -std=c++14 -pthread -x c src/binary_semaphore.c -x c src/spi_proto.c -I. -Isrc/ -x c src/crc16.c -x c src/spi_remote_host.c -o click -g
//causes solenoids to click in strobing sequence. Remote API test
#include <thread>
#include "spi_remote.h"

using namespace spi_proto;

void
click_task(void);

int main(int argc, char *argv[]) {
  /* Boilerplate. Start the SPI communication thread after initializing the
   * shared data structure. Then start the task thread.
   */
	host_remote_init(&remote);
	std::thread remote_thread(remote_task);
	std::thread click_thread(click_task);

	while (1) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	return 0;
}

//just a helper function
void
delay_ms(unsigned int ms)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void
click_task(void)
{
	puts("enabling 24V!");
  /* 15 is the 24V rail on the AMMDK V1. We know this by comparing the table in
   * amm-tiny/source/ammdk-carrier/carrier_gpio.cpp to the AMMDK V1 spreadsheet.
   */
	remote_set_gpio(15, 1);

	int wait = 200;
  /* Again, this number arrived at by comparing the carrier_gpio.cpp table with
   * the spreadsheet.
   */
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
