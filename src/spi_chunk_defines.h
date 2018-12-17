#define CHUNK_TYPE_GPIO 1
#define CHUNK_TYPE_DAC 2
#define CHUNK_TYPE_ADC 3
#define CHUNK_TYPE_FLOW 4
#define CHUNK_TYPE_ECHO 254
#define CHUNK_TYPE_ECHO_RETURN 255

#define OP_SET 1
#define OP_GET 2
#define OP_SET_META 0x11
#define OP_GET_META 0x12

//gpio META options TODO complete
#define GPIO_IS_INPUT (1 << 0)
#define GPIO_IS_PULLUP (1 << 1)


//TODO command to read version, firmware checksum and git commit hash (like the k20 CMSIS-DAP firmware has)
