//formats for sending and receiving messages. Perhaps should not bother marshalling into structs?

struct adc_cmd {uint8_t id; uint8_t cmd;};
#define CHUNK_LEN_ADC_M2S sizeof(struct adc_cmd)
struct dac_cmd {uint8_t id; uint8_t cmd; uint16_t val;};
#define CHUNK_LEN_DAC_M2S sizeof(struct dac_cmd)
enum gpio_set_vals {
	GPIO_OFF = 0, GPIO_ON = 1, GPIO_TOGGLE = 2
};
struct gpio_cmd {uint8_t id; uint8_t cmd; uint8_t val;};
#define CHUNK_LEN_GPIO_M2S sizeof(struct gpio_cmd)
struct flow_cmd {uint8_t flow_id; uint8_t cmd; uint16_t val;};
#define CHUNK_LEN_FLOW_M2S sizeof(struct flow_cmd)

struct adc_response {uint8_t adc_id; uint8_t cmd; uint8_t val;};
#define CHUNK_LEN_ADC_S2M sizeof(struct dac_cmd)
struct dac_response {uint8_t dac_id; uint8_t cmd;};
#define CHUNK_LEN_DAC_S2M sizeof(struct dac_cmd)
struct gpio_response {uint8_t gpio_id; uint8_t cmd; uint8_t val;};
#define CHUNK_LEN_GPIO_S2M sizeof(struct dac_cmd)
struct flow_response {uint8_t flow_id; uint8_t cmd; uint16_t val;};
#define CHUNK_LEN_FLOW_S2M sizeof(struct dac_cmd)

//IDEA these don't necessarily have to be completely sent. If cmd is SET, this is a response so val is 0 so for e.g. a fpio the response packet can be {4, TYPE_GPIO, ID, SET} and the final portion can be elided. This requires the parsers being very careful.
