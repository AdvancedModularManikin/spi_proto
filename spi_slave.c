/*
The communication protocol for letting the master use the peripherals of the slave is as follows
data direction is always known and so never marked in the protocol.
typeCode | id | ...

typeCodes are
syntax error 0 TODO use this for info commands instead
GPIO 1
ADC 2

TODO there should be a way to get maximum id of type, as well as a short description of a type-id pair.

*/

struct spi_protocol_state {
	bool waiting_on_deadman;
	struct spi_prosthetic_msg deadman_pending_msg;
	long deadman_deadline;
}

struct spi_prosthetic_msg {
	byte type_code;
	byte id; //an id, in the case of deadman a non-sequential code
	byte payload[0];
}

#define CONTROL_BAD_ID 0
#define CONTROL_BAD_OPCODE 1

#define MTYPE_CONTROL 0
#define MTYPE_GPIO 1
#define MTYPE_ADC 2
#define MTYPE_DEADMAN 3
#define NUM_M_TYPE 4

typedef void (type_handler_function_t)(struct spi_prosthetic_msg *m);

type_handler_function_t *type_handlers[NUM_M_TYPE] = {
	handle_control, handle_gpio, handle_adc, handle_deadman
};

void
process(struct spi_protocol_state *s, struct spi_prosthetic_msg *m)
{
	if (waiting_on_deadman) {
		//TODO store the message in the current deadman handler for later processing
		s->deadman_pending_msg = *m; // TODO copy it
		//TODO note that the waiting period begins now
	}
	if (m->type_code < NUM_M_TYPE) {
		if (&& m->type_code != MTYPE_DEADMAN) {
			type_handlers[m->type_code](m);
		} else {
			//TODO create a deadman handler for this message, or update an existing one
		}
	}
}

struct gpio_spi_box {
	GPIO_Type *base;
	uint8_t pin_ix; // the mask is 1 << pin_ix
	//int clock; //TODO other stuff that's needed
	char *desc; // has a max length, should be something simple like "D6"
};

int
read_gpio(struct gpio_spi_box *g)
{
	return GPIO_ReadPinInput(g->base, g->pin_ix);
}
void
write_gpio(struct gpio_spi_box *g, int level)
{
	GPIO_WritePinOutput(g->base, g->pin_ix, level);
}
void
handle_gpio(struct spi_prosthetic_msg *m)
{
	/*
	if valid id
		case opcode of
			read
				read that gpio, compose a packet reporting it
			write
				write that gpio
			settings change
				apply the settings
			description query
				compose message with short description (e.g. D7)
			other
				compose unreckognized opcode message
	else
		compose invalid id opcode
	*/
	if (m->id < gpios.max_id) {
		switch (m->payload[0]) {
		case OP_GPIO_READ:
			//read GPIO
			unsigned char gpio_read_val = read_gpio(&gpios[m->id]);
			//TODO compose response message
			break;
		case OP_GPIO_WRITE:
			//DONE write the gpio, no response
			write_gpio(&gpios[m->id], m->payload[1]);
			break;
		case OP_GPIO_SETTINGS:
			//TODO apply the settings change, no response
			break;
		case OP_GPIO_DESCRIPTION:
			//TODO compose response with description for this GPIO
			break;
		default:
			//TODO compose BAD OPCODE response, with gpio id and opcode?
			break;
		}
	} else {
		//TODO compose BAD ID (GPIO) message
	}
}

void
handle_adc(struct spi_prosthetic_msg *m)
{
	/*
	if valid id
		case opcode of
			read
				read that adc, compose a packet reporting it
			settings change
				apply the settings
			description query
				compose message with short description (e.g. D7)
			other
				compose unrecognized opcode message
	else
		compose invalid id opcode
	*/
	if (m->id < adc.adc_num) {
		switch (m->payload[0]) {
		case OP_ADC_READ:
			//TODO read the adc, compose a response
			break;
		case OP_ADC_SETTINGS:
			//TODO apply the settings change, no response
			break;
		case OP_ADC_DESCRIPTION:
			//TODO get the description, compose response
			break;
		default:
			//TODO compose BAD OPCODE message with adc id and opcode
		}
	} else {
		//TODO compose BAD ID (ADC) message
	}
}

void
compose_unfailing(struct spi_protocol_state *s, struct spi_prosthetic_msg *m /*TODO ARGS*/)
{
	/*
	the key thing here is that this function is unfailing. This invariant is maintained by disabling receipt of messages if the queue fills. So when the program starts the queue is empty, and if messages are transmitted correctly the queue will never have more than one message in it. If messages are repeatedly dropped in both directions, the queue state won't change. If messages are dropped in one direction, the queue on one edge might fill up. If the side with the full queue stops accepting messages, things will balance unless the other side also suddenly has a full queue.
	
	TODO can this problem can be avoided by taking note of the SEQ/ACK data in each message rather than simply discarding it?
	*/
	
	//TODO enqueue the message or whatever
}