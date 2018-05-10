#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/un.h>
#include <stdint.h>
#include <time.h>

#include "../spi_proto.h"

#define TRANSFER_SIZE 36
#define SPI_TRANSFER_LEN TRANSFER_SIZE

void
print_bytes(char *c, int n, int breaklen)
{
	if (!c) return;
	for (int i = 0; i < n; i++) {
		printf("%02x", c[i]);
		if (i % breaklen == (breaklen-1)) {
			printf("\n");
		} else {
			printf(" ");
		}
	}
	puts("");
}

void
linux_test_callback(struct spi_packet *p)
{
	print_bytes((char *) p, sizeof(struct spi_packet), 12);
}
int
main(int argc, char *argv[])
{
	int sock;
	int conn;

	struct sockaddr_un saddr = {AF_UNIX, "spi_forwarding_server"};

	sock = socket(AF_UNIX, SOCK_STREAM, 0);

	conn = connect(sock, (struct sockaddr *)&saddr, sizeof(saddr));

	if (conn) {
		perror("connect didn't work");
		abort();
	}
	struct timespec time_1ms;
	time_1ms.tv_sec = 0;
	time_1ms.tv_nsec = 1*1000000;//1 * 1ms
	
	char recvbuf[TRANSFER_SIZE];
	char sendbuf[TRANSFER_SIZE] = {};
	struct spi_state s;
	spi_proto_initialize(&s);
	int send_on_or_off = 0;
	char on_msg[3] = {0x01, 0x01, 0x01};
	int on_msg_len = 3;
	char off_msg[3] = {0x01, 0x01, 0x00};
	int off_msg_len = 3;
	while (1) {
		if (send_on_or_off) {
			spi_proto_send_msg(&s, on_msg, on_msg_len);
		} else {
			spi_proto_send_msg(&s, off_msg, off_msg_len);
		}
		send_on_or_off ^= 1;
		
		//message sending
		int ret = spi_proto_prep_msg(&s, sendbuf, SPI_TRANSFER_LEN);
		printf("spi_proto_prep_msg ret: %d\n", ret);
	
		//edbug output
		puts("sending");
		print_bytes(sendbuf, SPI_TRANSFER_LEN,12);
	
		//do transaction
		//transfer(spi_fd);
		write(sock, sendbuf, TRANSFER_SIZE);
		nanosleep(&time_1ms, NULL);
		int amt_read = read(sock, recvbuf, TRANSFER_SIZE);
		printf("amt_read = %d\n", amt_read);
	
		//process buffer into struct
		struct spi_packet pack;
		memcpy(&pack, recvbuf, SPI_TRANSFER_LEN);
		//TODO maybe fixup the CRC byte order?
	
		//process received message
		spi_proto_rcv_msg(&s, &pack, linux_test_callback);
	
		print_spi_state(&s);
		puts("received the following over the socket:");	
		print_bytes(recvbuf, TRANSFER_SIZE, 16);
	}

	return 0;
}
