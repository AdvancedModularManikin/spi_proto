#define CHUNK_TYPE_GPIO 1
#define CHUNK_TYPE_DAC 2
#define CHUNK_TYPE_ADC 3
#define CHUNK_TYPE_FLOW 4
#define CHUNK_TYPE_ECHO 254
#define CHUNK_TYPE_ECHO_RETURN 255

//TODO currently gpio actually works as 0->off 1->on 2->get bring both ends docs into sync one way or another
#define OP_SET 1
#define OP_GET 2

//TODO command to read version, firmware checksum and git commit hash (like the k20 CMSIS-DAP firmware has)
