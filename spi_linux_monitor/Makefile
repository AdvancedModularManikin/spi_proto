AMM_SOURCE=../../amm-tiny/source

linux_test_end: linux_test_end.c ../spi_proto.h spi_proto.o crc16.o monitor
	gcc -o linux_test_end linux_test_end.c monitor.o -L. -lspiproto -std=c99

monitor: monitor.c ../spi_proto.h ../spi_proto_lib/spi_proto.c libspiproto.a
	gcc -Wall -c monitor.c -std=c99
#	gcc -Wall chunk_test.o -L. -lspiproto -std=c99

#how to use static lib
#	gcc -Wall -o sock_spi_test_client sock_spi_test_client.c -L. -lspiproto

clean:
	rm -f libspiproto.a
	rm -f spi_proto.o crc16.o
	rm -f monitor.o
	rm -f linux_test_end

spi_proto.o: $(AMM_SOURCE)/spi_proto.cpp $(AMM_SOURCE)/spi_proto.h
	gcc -c -x c $(AMM_SOURCE)/spi_proto.cpp -std=c99

crc16.o: $(AMM_SOURCE)/crc16.cpp
	gcc -x c -c $(AMM_SOURCE)/crc16.cpp -std=c99

libspiproto.a: spi_proto.o crc16.o
	ar rc libspiproto.a spi_proto.o crc16.o
	ranlib libspiproto.a
