SET(CFLAGS -Wall -Wextra -Wno-missing-braces -Wno-unused-variable -Wno-unused-parameter)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(spi_proto)

add_library(spiproto STATIC src/spi_proto.c src/crc16.c src/spi_chunks.cpp)

add_executable(resync_test EXCLUDE_FROM_ALL test/test_resync.c test/test_util.c)
target_link_libraries(resync_test spiproto)
target_include_directories(resync_test PUBLIC src/ PUBLIC test/)

add_executable(spi_test EXCLUDE_FROM_ALL test/spi_proto_tests.c test/test_util.c src/spi_proto_util.c)
target_link_libraries(spi_test spiproto)
target_include_directories(spi_test PUBLIC src/ PUBLIC test/)

add_executable(test_longer_random_messages EXCLUDE_FROM_ALL test/test_longer_random_messages.c test/test_util.c)
target_link_libraries(test_longer_random_messages spiproto)
target_include_directories(test_longer_random_messages PUBLIC src/ PUBLIC test/)

add_executable(chunk_test EXCLUDE_FROM_ALL test/test_chunks.c test/test_util.c src/spi_proto_util.c)
target_link_libraries(chunk_test spiproto)
target_include_directories(chunk_test PUBLIC src/ PUBLIC test/)

add_executable(gpio_meta_test EXCLUDE_FROM_ALL test/gpio-meta-test.cpp test/test_util.c src/spi_proto_util.c src/spi_remote_host.c src/binary_semaphore.c src/spi_proto_master.cpp src/spi_chunks.cpp)
target_link_libraries(gpio_meta_test spiproto pthread)
target_include_directories(gpio_meta_test PUBLIC src/ PUBLIC test/)

add_executable(monitor EXCLUDE_FROM_ALL test/monitor.cpp test/test_util.c src/spi_proto_util.c src/spi_remote_host.c src/binary_semaphore.c src/spi_chunks.cpp src/spi_proto_master_datagram.cpp)
target_link_libraries(monitor spiproto pthread)
target_include_directories(monitor PUBLIC src/ PUBLIC test/)

#these two can only be built on linux
add_executable(click-test EXCLUDE_FROM_ALL test/click-test.cpp src/spi_proto_master.cpp src/spi_chunks.cpp src/binary_semaphore.c src/spi_proto.c src/crc16.c src/spi_remote_host.c)
target_link_libraries(click-test spiproto pthread)
target_include_directories(click-test PUBLIC src/ PUBLIC test/)

add_executable(blip-test test/blip-test.cpp src/spi_proto_master_datagram.cpp src/spi_chunks.cpp src/binary_semaphore.c src/spi_proto.c src/crc16.c src/spi_remote_host.c)
target_link_libraries(blip-test spiproto pthread)
target_include_directories(blip-test PUBLIC src/ PUBLIC test/)
