# Remote API

## Purpose

The ability to control the Tiny peripherals and access DDS messages from a single program greatly simplifies writing such programs.
For an example of direct usage, see the [datagram documentation.](datagram.md)

## Usage

Include the following header:

    #include "spi_remote.h"

You must put the file `99-spidev-open.rules` into `/etc/udev/rules.d` and either reboot or run `udevadm control --reload-rules`.
This will allow non-privileged users (specifically the `amm` user) to access the spi interface.

### Function usage

`void remote_set_gpio(int ix, int val)`

Set gpio number `ix` to `val`.
Blocking.

`bool remote_get_gpio(int ix)`

Get the input of gpio number `ix`.
Blocking.
You should first set the direction of the gpio with `remote_set_gpio_meta`.

`void remote_set_gpio_meta(int ix, int val)`

Set direction of gpio number `ix` to `val`.
Blocking.
`val` is either `OP_SET` or `OP_GET` (defined in [spi_chunk_defines.h](src/spi_chunk_defines.h)).
GPIO default to `OP_SET`.

`void remote_set_dac(int ix, int val)`

Set DAC number `ix` to `val`.
Clamped between 0x000 and 0xfff.
Blocking.

`uint32_t remote_get_adc(int ix)`

Return the current ADC reading. It is only refreshed at 50Hz.
The maximum value is 4095 (12 bits, 0x000 to 0xfff).

`void remote_task(void);`

Create a thread as `std::thread remote_thread(remote_task);` to use this API.
This thread sits in a loop, summarized below:

    forever:
        msg = queued_messages();
        response = spi_send(msg);
        process(response);

#### Internal

`int send_chunk(uint8_t *buf, size_t len);`

This is used by all functions to enqueue messages to be packed to be sent over the SPI connection.
The first byte of `buf` is overwritten with `len`.

## Example

[`test/click-test.cpp`](test/click-test.cpp) is a simple example.

## How it works

There is another thread, `remote_thread`, which handles sending and receiving all SPI messages.
The application can send length-prefixed chunks to this thread, which are then greedily packed into the next outgoing message.
The remote thread signals the controlling thread through semaphores in a struct `remote` linked in in `spi_proto_master.cpp`.
On the k66f there is a function which dispatches several kinds of chunks.
The flow for using a peripheral is as follows:

[where]  

1. [app] Send a chunk with a command for that peripheral
2. [app] Wait and block on the semaphore for that peripheral.
3. [remote] Send the chunk.
4. [k66f] Receive chunk, perform action, and send confirmation chunk.
5. [remote] Receive chunk and dispatch, storing any return value and signaling on the appropriate semaphore.
6. [app] Return from semaphore and take return value, if any.

This sequence is packaged up in the `remote_set_*` and `remote_get_*` functions.

The remote thread sends and receives messages at about 1kHz.

### Internal details

A struct of lists of semaphores is at global scope.
This struct has one semaphore for each peripheral.
This allows the `remote_*` api calls to block and appear to the user as though they are directly manipulating peripherals.
This abstraction has the flaw that all calls block.
It would be possible to hold semaphores only until the chunk has been properly sent (using internal knowledge of the protocol) but this is not a guarantee that the requested action has been taken.
This idea was not used because it will only save time in a few unusual cases with very specific message retransmit patterns.
The confirmation message is usually transmitted in the round immediately after the command message.

## Caveats

The provided functions wait for each command to be confirmed before sending the next.
It is possible to send all commands and then wait for each to complete.
This will save some time but is not really necessary, although it might be useful for synchronizing valves more closely.

## Bugs

The semaphores used are not queueing semaphores, so it's possible that if two threads issue read commands at very close times they will read one another's return value.
This is not a serious concern: It requires two threads that are using overlapping subsets of the peripherals.
It's hard to say that any control system overlapping with another will give good results.
Further both threads will usually read the same value.
