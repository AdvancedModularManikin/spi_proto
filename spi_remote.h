struct host_adc {
	struct binary_semaphore sem;
	uint32_t last_read; // TODO some potential issues with stale data
};

struct host_gpio {
	struct binary_semaphore sem;
};

struct host_solenoid {
	struct binary_semaphore sem;
	//TODO possibly put some other state here
};

struct host_remote {
	struct host_adc adc[ADC_NUM];
	struct host_gpio gpio[GPIO_NUM];
	struct host_solenoid solenoid[SOLENOID_NUM];
};

//TODO move to header
void
adc_handle_master(struct host_adc *adc, size_t n, struct adc_command *a);
void
valve_handle_master(struct host_solenoid *r, size_t n, struct valve_command *a);
void
gpio_handle_master(struct host_gpio *r, size_t n, struct gpio_command *a);
