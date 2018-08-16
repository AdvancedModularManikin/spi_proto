//formats for sending and receiving messages. Perhaps should not bother marshalling into structs?
struct adc_response {uint8_t adc_id; uint8_t cmd; uint8_t val;};
struct dac_response {uint8_t dac_id; uint8_t cmd;};
struct gpio_response {uint8_t gpio_id; uint8_t cmd; uint8_t val;};
struct flow_response {uint8_t flow_id; uint8_t cmd; uint16_t val;};
