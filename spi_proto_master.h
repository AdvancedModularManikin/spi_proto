struct host_remote remote;

void
remote_task(void);
namespace spi_proto {
struct master_spi_proto {
	struct spi_state proto;
};

struct master_spi_proto p;
void
spi_proto_master_initialize(struct master_spi_proto *s);
int
master_send_message(struct master_spi_proto &p, unsigned char *buf, unsigned int len);
}

uint32_t
remote_read_adc(unsigned int ix);
void
remote_gpio_set(int gpio, int on);

int
send_chunk(uint8_t *buf, size_t len);
