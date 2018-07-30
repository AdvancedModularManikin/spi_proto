#define CHUNK_TYPE_VALVE 1
#define CHUNK_TYPE_GPIO 2
#define CHUNK_TYPE_DAC 3
#define CHUNK_TYPE_MOTOR 4
#define CHUNK_TYPE_ADC 5
#define CHUNK_TYPE_ECHO 7
#define CHUNK_TYPE_ECHO_RETURN 8

#define CLASS_GPIO CHUNK_TYPE_GPIO
//TODO currently gpio actually works as 0->off 1->on 2->get bring both ends docs into sync one way or another
#define OP_SET 1
#define OP_GET 2

//TODO clean up
#define CMD_ADC_READ 0
