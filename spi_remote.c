//TODO temp for compilation
#define GPIO_NUM 1
#define ADC_NUM 1
#define DAC_NUM 2
#define CHUNK_LEN_ADC 0
#define CHUNK_LEN_DAC 0
#define CHUNK_LEN_GPIO 4


//TODO temp for compilation
extern int unknown_chunk_type_msg_count;
extern int out_of_range_chunks;

#include <stdint.h>
#include <pthread.h>
#include "binary_semaphore.h"
#include "spi_proto.h"
#include "spi_remote.h"
#include "spi_proto_lib/spi_chunk_defines.h"


//TODO temp for compilation
uint16_t bad_chunk_counter;


//TODO put semaphores on each endpoint to allow waiting on them, give on the semaphore for each value that is received. the give should be nonblocking
//TODO possible issue, when the thread takes on the semaphore it could end up getting the value of a previous read that some other thread triggered
//a sempahore with a queue where only one taker is released per give would solve this issue


void
host_remote_init(struct host_remote *r)
{
	//currently only initialize the semaphores

	for (int i = 0; i < ADC_NUM; i++) {
		bisem_init(&r->adc[i].sem);
	}
	for (int i = 0; i < GPIO_NUM; i++) {
		bisem_init(&r->gpio[i].sem);
	}
}

int
remote_chunk_handler(struct host_remote *r, uint8_t *buf, size_t len)
{
	//TODO go through chunk types and bounce semaphores
	if (len < 2) return -1; // length zero isn't a real chunk, length 1 can't carry data
	switch(buf[1]) {
	case CHUNK_TYPE_GPIO:
		if (len < CHUNK_LEN_GPIO) {bad_chunk_counter++;break;}
		// [LEN|TYPE|ID|CMD]
		//TODO what about val
		struct gpio_response gpiocmd;
		gpiocmd.gpio_id = buf[2];
		gpiocmd.cmd = buf[3];
		gpio_handle_master(r->gpio, GPIO_NUM, &gpiocmd);
		break;
	case CHUNK_TYPE_ADC:
		if (len < CHUNK_LEN_ADC) {bad_chunk_counter++;break;}
		// [LEN|TYPE|ID|CMD]
		//TODO what about val
		struct adc_response adccmd;
		adccmd.adc_id = buf[2];
		adccmd.cmd = buf[3];
		adc_handle_master(r->adc, ADC_NUM, &adccmd);
		break;
	case CHUNK_TYPE_DAC:
		if (len < CHUNK_LEN_DAC) {bad_chunk_counter++;break;}
		// [LEN|TYPE|ID|CMD]
		struct dac_response daccmd;
		daccmd.dac_id = buf[2];
		daccmd.cmd = buf[3];
		dac_handle_master(r->dac, DAC_NUM, &daccmd);
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
adc_handle_master(struct host_adc *adc, size_t n, struct adc_response *a)
{
	if (a->adc_id > ADC_NUM) {out_of_range_chunks++;return;}
	if (a->cmd == CMD_ADC_READ) {
		adc[a->adc_id].last_read = a->val;
		bisem_post(&adc[a->adc_id].sem);
	} else {
		//TODO other ops
	}
}

void
gpio_handle_master(struct host_gpio *gpio, size_t n, struct gpio_response *c)
{
	if (c->gpio_id > GPIO_NUM) {out_of_range_chunks++;return;}
	if (c->cmd == OP_GET) {
		gpio[c->gpio_id].last_read = c->val;
		bisem_post(&gpio[c->gpio_id].sem);
	} else {
		//TODO other ops
	}
}

void
dac_handle_master(struct host_dac *dac, size_t n, struct dac_response *c)
{
	if (c->dac_id > DAC_NUM) {out_of_range_chunks++;return;}
	if (c->cmd == OP_SET) {
		bisem_post(&dac[c->dac_id].sem);
	} else {
		//TODO other ops
	}
}
