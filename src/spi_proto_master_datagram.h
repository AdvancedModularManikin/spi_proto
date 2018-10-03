void
datagram_task(void);
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

extern void (*spi_callback)(struct spi_packet*);

int
send_message(uint8_t *buf, size_t len);
void remote_task(void);
