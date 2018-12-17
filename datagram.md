# Datagram API

Unlike the remote api with its many functions, this has only two, send and receive a message.
You must write a custom program running on the AMMDK k66f to make any practical use of this functionality.

`test/blip-test.cpp` is an example of a program that uses this API.

## Functions

`send_message(uint8_t *buf, size_t len)` sends the data in `buf`. `len` contains the length of the buffer. Returns:

* 0 if message was sent successfully
* -1 if `len` > `SPI_MSG_PAYLOAD_LEN`
* -2 if no space available to send message

`TODO` is a callback for received messages.

`void datagram_task(void)` should be called as a thread, like `std::thread thread(datagram_task);` and must be running before `send_message` is called.
