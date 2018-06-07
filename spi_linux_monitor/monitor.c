#define _DEFAULT_SOURCE
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../spi_proto.h"
#include "spi_chunk_defines.h"

#define SPI_TRANSFER_LEN (SPI_MSG_PAYLOAD_LEN+4)
#define SPI_TRANSFER_SIZE SPI_TRANSFER_LEN

void transfer(int fd);

uint8_t spi_in_buf[SPI_TRANSFER_SIZE], spi_out_buf[SPI_TRANSFER_SIZE];


void
spi_handler(struct spi_packet *p)
{
	//TODO
}


void
print_bytes(unsigned char *b, int n)
{
	for (int i = 0; i < n;i++)
		printf("%02x ", b[i]);
	printf("\n");
}
void
first_loop(void)
{
	//nothing to do
}
void
loop(struct spi_state *s, int spi_fd)
{
	/* loop:
		do whatever testing processing (message sending, for example)
		marshal messages into buffers
		do the transaction
		process the received message
	*/
	
	
	//load messages into queue if any
	static int mstate = 0;
	mstate %= 2;
	int N = 10;
	//two messages of various chunks
	unsigned char m2s1[10] = {2,7,2,7,1,1,1,3,7,1};
	unsigned char m2s2[8] = {4, 7, 3, 4, 4, 7, 3, 1};
	// TODO send other messages
	//while(!spi_proto_send_msg(s, mstate++ ? m2s1:m2s2, N));
	
	//message sending
	spi_proto_prep_msg(s, spi_out_buf, SPI_TRANSFER_LEN);
	
	//debug output
	puts("sending");
	print_bytes(spi_out_buf, SPI_TRANSFER_LEN);
	
	//do transaction
	transfer(spi_fd);
	
	//process buffer into struct
	struct spi_packet pack;
	memcpy(&pack, spi_in_buf, SPI_TRANSFER_LEN);
	//TODO maybe fixup the CRC byte order?
	/*
	puts("Received:");
	print_bytes(spi_in_buf, SPI_TRANSFER_LEN);
	*/
	//process received message
	
	//puts("before rcv_msg");
	spi_proto_rcv_msg(s, &pack, spi_handler);
	//puts("after rcv_msg");	
//	print_spi_state(s);
}

void
parse_cli(struct spi_state *s, char *cmd, size_t n)
{
	//example: "g s 1 1\n" -> GPIO, SET, (GPIO 1), 1 -> 
	int class = 0;
	int op = 0;
	int *param1 = NULL;
	int *param2 = NULL;
	
	char *tokens[4] = {NULL, NULL, NULL, NULL};

	assert(cmd != NULL);
	
	for (int i = 0; i < 4; i++)
		printf("tok %d: '%s'\n", i, tokens[i]);

	for (int i = 0;(tokens[i] = strsep(&cmd, " ")) != NULL && i < 4;i++);
		//printf("%s\n", token);
	
	for (int i = 0; i < 4; i++)
		printf("tok %d: '%s'\n", i, tokens[i]);
	
	if (tokens[0] && !strcmp(tokens[0], "g")) {
		puts("type is GPIO");
		class = CLASS_GPIO;
	} else {
		printf("'%s' is not a recognized type\n", tokens[0]);
	}
	
	if (tokens[1] && !strcmp(tokens[1], "s")) {
		puts("command is SET");
		op = OP_SET;
	} else if (tokens[1] && !strcmp(tokens[1], "g")) {
		puts("command is GET");
		op = OP_GET;
	} else {
		printf("'%s' is not a recognized operation\n", tokens[1]);
	}
	
	int num1fail = 0, num2fail = 0;
	char *res = NULL;
	int num1 = tokens[2] ? strtol(tokens[2], &res, 10) : 0;
	if (num1fail = (res == tokens[2])) {
		puts("num1 parse failed");
	} else param1 = &num1;
	res = NULL;
	int num2 = tokens[3] ? strtol(tokens[3], &res, 10) : 0;
	if (num2fail = (res == tokens[3])) {
		puts("num2 parse failed");
	} else param2 = &num2;
	printf("num params: good? (%d, %d) %d %d\n", !num1fail, !num2fail, num1, num2);
	
	//TODO centralize chunk schema definitions
	unsigned char buf[32];
	unsigned int ix = 0;
	char matched = 0;
	if (class == CLASS_GPIO && op == OP_SET && param1 && param2) {
		printf("set gpio %d to %d\n", *param1, !!*param2);
		buf[ix++] = CLASS_GPIO;
		buf[ix++] = *param1;
		buf[ix++] = !!*param2;
		if (*param2) {
			puts("TODO emit gpio ON"); matched = 1;
		} else {
			puts("TODO emit gpio OFF"); matched = 1;
		}
	}
	if (class == CLASS_GPIO && op == OP_GET && param1) {
		buf[ix++] = CLASS_GPIO;
		buf[ix++] = *param1;
		buf[ix++] = 2;
		puts("TODO emit gpio GET"); matched = 1;
	}
	
	puts("buf is:");
	for (int i = 0;i < ix; i++) printf("%02x ", buf[i]);
	if (matched) spi_proto_send_msg(s, buf, ix);
	if (!matched) puts("no cases matched. syntax error?");
	
}
