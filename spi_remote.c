//TODO temp for compilation
#define GPIO_NUM 1
#define ADC_NUM 1
#define SOLENOID_NUM 1
#define VALVE_NUM 1
#define CHUNK_LEN_ADC 0
#define CHUNK_LEN_GPIO 0


//TODO temp for compilation
struct adc_command {int adc_id; int val; int cmd;};
struct gpio_command {int gpio_id; int cmd;};
struct valve_command {int valve_id; int cmd;};
int unknown_chunk_type_msg_count;


#include <stdint.h>
#include <pthread.h>
#include "binary_semaphore.h"
#include "spi_proto.h"
#include "spi_remote.h"
#include "spi_proto_lib/spi_chunk_defines.h"




//TODO put semaphores on each endpoint to allow waiting on them, give on the semaphore for each value that is received. the give should be nonblocking
//TODO possible issue, when the thread takes on the semaphore it could end up getting the value of a previous read that some other thread triggered
//a sempahore with a queue where only one taker is released per give would solve this issue


void
host_remote_init(struct host_remote *r)
{
	//TODO mostly initialize the semaphores
}

int
remote_chunk_handler(struct host_remote *r, uint8_t *buf, size_t len)
{
	//TODO go through chunk types and bounce semaphores
	if (len < 2) return -1; // length zero isn't a real chunk, length 1 can't carry data
	switch(buf[1]) {
	case CHUNK_TYPE_VALVE:
		if (len < 4) break; // TODO maybe increment some error counter somewhere
		// [LEN|TYPE|ID|CMD]
		struct valve_command valvecmd;
		valvecmd.valve_id = buf[2];
		valvecmd.cmd = buf[3];
		valve_handle_master(r->solenoid, VALVE_NUM, &valvecmd);
		break;
	case CHUNK_TYPE_GPIO:
		if (len < CHUNK_LEN_GPIO) break; // TODO log bad chunk counter
		// [LEN|TYPE|ID|CMD]
		struct gpio_command gpiocmd;
		gpiocmd.gpio_id = buf[2];
		gpiocmd.cmd = buf[3];
		gpio_handle_master(r->gpio, GPIO_NUM, &gpiocmd);
		break;
	case CHUNK_TYPE_ADC:
		if (len < CHUNK_LEN_ADC) break; // TODO log bad chunk counter
		// [LEN|TYPE|ID|CMD]
		struct adc_command adccmd;
		adccmd.adc_id = buf[2];
		adccmd.cmd = buf[3];
		adc_handle_master(r->adc, ADC_NUM, &adccmd);
		break;
	//case CHUNK_TYPE_STRING
	//TODO other chunk types
	default:
		unknown_chunk_type_msg_count++; // TODO think of better name
		return -2;
	}
	return 0;
}

void
adc_handle_master(struct host_adc *adc, size_t n, struct adc_command *a)
{
	//TODO check for adc_id out of range
	if (a->cmd == CMD_ADC_READ) {
		adc[a->adc_id].last_read = a->val;
		bisem_post(&adc[a->adc_id].sem);
	} else {
		//TODO other ops
	}
}
