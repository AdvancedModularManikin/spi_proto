all : spi_proto_tests test_longer_random_messages

spi_proto_tests: test/spi_proto_tests.c crc16.c spi_proto_lib/spi_proto.c
	gcc -o spi_test test/spi_proto_tests.c crc16.c spi_proto_lib/spi_proto.c -I. # -DDEBUG_SPI_PROTO

test_longer_random_messages: test/test_longer_random_messages.c crc16.c spi_proto_lib/spi_proto.c
	gcc -o msg_test test/test_longer_random_messages.c crc16.c spi_proto_lib/spi_proto.c -I.

clean:
	rm -f spi_test msg_test
