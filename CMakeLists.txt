SET(CFLAGS -Wall -Wextra -Wno-missing-braces -Wno-unused-variable -Wno-unused-parameter)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(spi_proto)

add_library(spiproto STATIC src/spi_proto.c src/crc16.c src/spi_chunks.cpp)

add_executable(resync_test test/test_resync.c test/test_util.c)
target_link_libraries(resync_test spiproto)
target_include_directories(resync_test PUBLIC src/ PUBLIC test/)

add_executable(spi_test test/spi_proto_tests.c test/test_util.c src/spi_proto_util.c)
target_link_libraries(spi_test spiproto)
target_include_directories(spi_test PUBLIC src/ PUBLIC test/)

add_executable(test_longer_random_messages test/test_longer_random_messages.c test/test_util.c)
target_link_libraries(test_longer_random_messages spiproto)
target_include_directories(test_longer_random_messages PUBLIC src/ PUBLIC test/)

add_executable(chunk_test test/test_chunks.c test/test_util.c src/spi_proto_util.c)
target_link_libraries(chunk_test spiproto)
target_include_directories(chunk_test PUBLIC src/ PUBLIC test/)

#TODO can only run on linux
#click-test: test/click-test.cpp libspiproto.a spi_chunks.o test/test_util.o spi_proto_util.o
#	#g++ $(CFLAGS) -o click-test test/click-test.cpp -std=c++14 -pthread spi_chunks.o test/test_util.o spi_proto_util.o -L. -lspiproto -I. -Isrc
#	g++ test/click-test.cpp src/spi_proto_master.cpp src/spi_chunks.cpp -std=c++14 -pthread -x c src/binary_semaphore.c -x c src/spi_proto.c -I. -Isrc/ -x c src/crc16.c -x c src/spi_remote_host.c -o click-test -g

#TODO can only run on linux
#blip-test: test/blip-test.cpp libspiproto.a spi_chunks.o test/test_util.o spi_proto_util.o
#	#g++ $(CFLAGS) -o blip-test test/blip-test.cpp -std=c++14 -pthread spi_chunks.o test/test_util.o spi_proto_util.o -L. -lspiproto -I. -Isrc
#	g++ test/blip-test.cpp src/spi_proto_master_datagram.cpp src/spi_chunks.cpp -std=c++14 -pthread -x c src/binary_semaphore.c -x c src/spi_proto.c -I. -Isrc/ -x c src/crc16.c -x c src/spi_remote_host.c -o blip-test -g
