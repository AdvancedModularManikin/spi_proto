all: sock_spi_bridge sock_spi_test_client

sock_spi_bridge: sock_spi_bridge.c
	gcc -Wall -o sock_spi_bridge sock_spi_bridge.c

sock_spi_test_client: sock_spi_test_client.c libspiproto.a ../spi_proto.h
	gcc -Wall -o sock_spi_test_client sock_spi_test_client.c -L. -lspiproto

clean:
	rm -f sock_spi_bridge sock_spi_test_client spi_forwarding_server
	rm -f libspiproto.a
	rm -f spi_proto.o crc16.o

spi_proto.o: spi_proto.c spi_proto.h
	gcc -c spi_proto.c

crc16.o: crc16.c
	gcc -c crc16.c

libspiproto.a: spi_proto.o crc16.o
	ar rc libspiproto.a spi_proto.o crc16.o
	ranlib libspiproto.a

