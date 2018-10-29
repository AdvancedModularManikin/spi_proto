CFLAGS=-Wall -Wextra -Wno-missing-braces -Wno-unused-variable -Wno-unused-parameter

all : spi_test msg_test chunk_test resync_test spi_proto.o spi_chunks.o click-test blip-test

spi_test: test/spi_proto_tests.c libspiproto.a test/test_util.o spi_proto_util.o
	gcc $(CFLAGS) -o spi_test test/spi_proto_tests.c test/test_util.o crc16.o spi_proto_util.o -L. -lspiproto -I. -Isrc

msg_test: test/test_longer_random_messages.c libspiproto.a test/test_util.o spi_proto_util.o
	gcc $(CFLAGS) -o msg_test test/test_longer_random_messages.c test/test_util.o spi_proto_util.o -L. -lspiproto -I. -Isrc

chunk_test: test/test_chunks.c libspiproto.a spi_chunks.o test/test_util.o spi_proto_util.o
	gcc $(CFLAGS) -o chunk_test test/test_chunks.c spi_chunks.o test/test_util.o spi_proto_util.o -L. -lspiproto -I. -Isrc

resync_test: test/test_resync.c libspiproto.a spi_chunks.o test/test_util.o spi_proto_util.o
	gcc $(CFLAGS) -o resync_test test/test_resync.c spi_chunks.o test/test_util.o spi_proto_util.o -L. -lspiproto -I. -Isrc

spi_proto.o: src/spi_proto.c src/spi_proto.h
	gcc $(CFLAGS) -c src/spi_proto.c -std=c99 -Isrc

spi_proto_util.o: src/spi_proto_util.c src/spi_proto_util.h
	gcc $(CFLAGS) -c src/spi_proto_util.c -std=c99 -Isrc

spi_chunks.o: src/spi_chunks.cpp src/spi_chunks.h
	gcc $(CFLAGS) -c -x c src/spi_chunks.cpp -std=c99

crc16.o: src/crc16.c
	gcc $(CFLAGS) -c src/crc16.c

test/test_util.o: test/test_util.c
	gcc $(CFLAGS) -c test/test_util.c -o test/test_util.o

clean:
	rm -f spi_test msg_test chunk_test resync_test spi_proto.o spi_chunks.o crc16.o test/test_util.o libspiproto.a binary_semaphore.o spi_proto_util.o spi_remote.o
	rm -f click-test blip-test

libspiproto.a: spi_proto.o crc16.o
	ar rc libspiproto.a spi_proto.o crc16.o
	ranlib libspiproto.a

click-test: test/click-test.cpp libspiproto.a spi_chunks.o test/test_util.o spi_proto_util.o
	#g++ $(CFLAGS) -o click-test test/click-test.cpp -std=c++14 -pthread spi_chunks.o test/test_util.o spi_proto_util.o -L. -lspiproto -I. -Isrc
	g++ test/click-test.cpp src/spi_proto_master.cpp src/spi_chunks.cpp -std=c++14 -pthread -x c src/binary_semaphore.c -x c src/spi_proto.c -I. -Isrc/ -x c src/crc16.c -x c src/spi_remote_host.c -o click-test -g


blip-test: test/blip-test.cpp libspiproto.a spi_chunks.o test/test_util.o spi_proto_util.o
	#g++ $(CFLAGS) -o blip-test test/blip-test.cpp -std=c++14 -pthread spi_chunks.o test/test_util.o spi_proto_util.o -L. -lspiproto -I. -Isrc
	g++ test/blip-test.cpp src/spi_proto_master_datagram.cpp src/spi_chunks.cpp -std=c++14 -pthread -x c src/binary_semaphore.c -x c src/spi_proto.c -I. -Isrc/ -x c src/crc16.c -x c src/spi_remote_host.c -o blip-test -g

