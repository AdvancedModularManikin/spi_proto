linux_test_end: linux_test_end.c ../spi_proto.h spi_proto.o crc16.o chunk
	gcc -o linux_test_end linux_test_end.c chunk_test.o -L. -lspiproto -std=c99

chunk: chunk_test.c ../spi_proto.h ../spi_proto_lib/spi_proto.c libspiproto.a
	gcc -Wall -c chunk_test.c -std=c99
#	gcc -Wall chunk_test.o -L. -lspiproto -std=c99

#how to use static lib
#	gcc -Wall -o sock_spi_test_client sock_spi_test_client.c -L. -lspiproto

clean:
	rm -f libspiproto.a
	rm -f spi_proto.o crc16.o spi_chunks.o
	rm -f chunk_test.o
	rm -f linux_test_end

spi_proto.o: spi_proto_lib/spi_proto.cpp spi_proto_lib/spi_proto.h
	gcc -c -x c spi_proto_lib/spi_proto.cpp -std=c99

spi_chunks.o: spi_proto_lib/spi_chunks.cpp spi_proto_lib/spi_chunks.h
	gcc -c -x c spi_proto_lib/spi_chunks.cpp -std=c99

crc16.o: crc16.c
	gcc -c crc16.c -std=c99

libspiproto.a: spi_proto.o crc16.o spi_chunks.o
	ar rc libspiproto.a spi_proto.o crc16.o spi_chunks.o
	ranlib libspiproto.a

