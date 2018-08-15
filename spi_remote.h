//formats for sending and receiving messages. Perhaps should not bother marshalling into structs?
struct adc_response {int adc_id; int val; int cmd;};
struct dac_response {int dac_id; int cmd;};
struct gpio_response {int gpio_id; int cmd; int val;};

struct host_adc {
	struct binary_semaphore sem;
	uint16_t last_read; // TODO some potential issues with stale data
};

struct host_gpio {
	struct binary_semaphore sem;
	uint8_t last_read;
};

struct host_dac {
	struct binary_semaphore sem;
};

//flow sensors are a separate category because they use interrupts which can't reasonably be exposed remotely (and the other one uses i2c)
struct host_flow {
	struct binary_semaphore sem;
	uint16_t last_read;
};

struct host_remote {
	struct host_adc adc[ADC_NUM];
	struct host_gpio gpio[GPIO_NUM];
	struct host_dac dac[DAC_NUM];
	struct host_flow flow[FLOW_NUM];
};

//TODO rather than creating host_motor just use host_gpio and host_dac
//IDEA monitor which chunks go into which messages and the current last_unconfirmed to signal successful sends of messages that don't need to return. Flaw: this would be complicated and round-trip confirmation is ez, what about other errors? Confirmation is a good thing.
//TODO make it so that bogus messages, whenever possible, still have the appropriate response sent. Set an error flag.

//IDEA features useful in a repl: more detailed error reporting. Associating a name (B19, DAC0, ADC7) with each index in a type. Reporting how many of each peripheral are available.

void
adc_handle_master(struct host_adc *adc, size_t n, struct adc_response *a);
void
dac_handle_master(struct host_dac *dac, size_t n, struct dac_response *a);	
void
gpio_handle_master(struct host_gpio *r, size_t n, struct gpio_response *a);

int
remote_chunk_handler(struct host_remote *r, uint8_t *buf, size_t len);