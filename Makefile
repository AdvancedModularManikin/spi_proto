AMM_SOURCE=../amm-tiny/source

CFLAGS=-Wall -Wextra -Wno-missing-braces -Wno-unused-variable -Wno-unused-parameter

all : spi_test msg_test chunk_test resync_test spi_proto.o spi_chunks.o

spi_test: test/spi_proto_tests.c libspiproto.a test/test_util.o
	gcc $(CFLAGS) -o spi_test test/spi_proto_tests.c test/test_util.o crc16.o -L. -lspiproto -I.

msg_test: test/test_longer_random_messages.c libspiproto.a test/test_util.o
	gcc $(CFLAGS) -o msg_test test/test_longer_random_messages.c test/test_util.o -L. -lspiproto -I.

chunk_test: test/test_chunks.c libspiproto.a spi_chunks.o test/test_util.o
	gcc $(CFLAGS) -o chunk_test test/test_chunks.c spi_chunks.o test/test_util.o -L. -lspiproto -I.

resync_test: test/test_resync.c libspiproto.a spi_chunks.o test/test_util.o
	gcc $(CFLAGS) -o resync_test test/test_resync.c spi_chunks.o test/test_util.o -L. -lspiproto -I.

#spi_proto.o: $(AMM_SOURCE)/spi_proto.cpp $(AMM_SOURCE)/spi_proto.h
#	gcc $(CFLAGS) -c -x c $(AMM_SOURCE)/spi_proto.cpp -std=c99

spi_proto.o: spi_proto_lib/spi_proto.c spi_proto.h
	gcc $(CFLAGS) -c spi_proto_lib/spi_proto.c -std=c99 -I.


spi_chunks.o: $(AMM_SOURCE)/spi_chunks.cpp $(AMM_SOURCE)/spi_chunks.h
	gcc $(CFLAGS) -c -x c $(AMM_SOURCE)/spi_chunks.cpp -std=c99

crc16.o: crc16.c
	gcc $(CFLAGS) -c crc16.c

test/test_util.o: test/test_util.c
	gcc $(CFLAGS) -c test/test_util.c -o test/test_util.o

clean:
	rm -f spi_test msg_test chunk_test resync_test spi_proto.o spi_chunks.o crc16.o test/test_util.o libspiproto.a

libspiproto.a: spi_proto.o crc16.o
	ar rc libspiproto.a spi_proto.o crc16.o
	ranlib libspiproto.a
