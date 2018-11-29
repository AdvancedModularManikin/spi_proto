extern struct host_remote remote;

void
remote_task(void);
namespace spi_proto {
struct master_spi_proto {
	struct spi_state proto;
};

extern struct master_spi_proto p;
void
spi_proto_master_initialize(struct master_spi_proto *s);
int
master_send_message(struct master_spi_proto &p, unsigned char *buf, unsigned int len);
}

uint32_t
remote_get_adc(unsigned int ix);
void remote_set_dac(unsigned int ix, uint16_t val);

void
remote_set_gpio(int gpio, int on);
int
remote_get_gpio(int gpio);

//TODO convert to unsigned char flags
void
remote_set_gpio_meta(int gpio, int in);

int
send_chunk(uint8_t *buf, size_t len);
void remote_task(void);
void click_remote(struct spi_packet *p);
int prepare_master_chunks(void);
