# Datagram API

Unlike the remote api with its many functions, this has only two, send and receive a message.
You must write a custom program running on the AMMDK k66f to make any practical use of this functionality.

[`test/blip-test.cpp`](test/blip-test.cpp) is an example of a program that uses this API.
It requires `heartrateLED.elf` from the `amm-tiny` repo to be on the k66f.

## Functions

#### `send_message(uint8_t *buf, size_t len)`
`send_message` sends the data in `buf`. `len` contains the length of the buffer. `send_message` returns:

* 0 if message was sent successfully
* -1 if `len` > `SPI_MSG_PAYLOAD_LEN`
* -2 if no space available to send message

#### `*spi_callback`
The `spi_callback` variable is a callback for received messages.

    extern void (*spi_callback)(struct spi_packet*);

This function receives the protocol information as well as payload, which is in field `msg` of the argument.
`msg` is declared `uint8_t msg[SPI_MSG_PAYLOAD_LEN];`.
If you do not need to receive messages, `spi_callback` may be `NULL` but it must be present.

#### `void datagram_task(void)`
This task should be called as a thread, e.g. `std::thread thread(datagram_task);`, and must be running before `send_message` is called.
