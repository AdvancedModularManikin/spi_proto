spi_proto_tests: spi_proto_tests.c crc16.c spi_proto_lib/spi_proto.c
	gcc -o spi_test spi_proto_tests.c crc16.c spi_proto_lib/spi_proto.c -I. -DDEBUG_SPI_PROTO

clean:
	rm -f spi_test
