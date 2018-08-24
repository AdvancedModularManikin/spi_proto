#Remote API

##Purpose

The ability to control the Tiny peripherals and access DDS messages from a single program greatly simplifies writing such programs.

##How it works

There is another thread, `remote_thread`, which handles sending and receiving all SPI messages. The application can send length-prefixed chunks to this thread, which are then greedily packed into the next outgoing message. On the tiny there is a function which dispatches several kinds of chunks. The flow for using a peripheral is as follows:

[where]
[app] 1. Send a chunk with a command for that peripheral
[app] 2. Wait and block on the sempahore for that peripheral.
[remote] 3. Send the chunk.
[tiny] 4. Receive chunk, perform action, and send confirmation chunk.
[remote] 5. Receive chunk and dispatch, storing any return value and signaling on the appropriate semaphore.
[app] 6. Return from semaphore and take return value, if any.

This sequence is packaged up in the `remote_set_*` and `remote_get_*` functions.

The remote thread sends and receives messages at about 1kHz.

###Internal details

A struct of lists of semaphores is at global scope. This struct has one semaphore for each peripheral. This allows the remote_* api calls to block and appear to the user as though they are directly manipulating peripherals. This abstraction has the flaw that all calls block, while it might be possible to hold semaphores until the chunk has been properly sent (using internal knowledge of the reliability protocol). This idea was not used because it will only save time in a few unusual cases with very specific message retransmit patterns. The confirmation message is usually transmitted in the round immediately after the command message.

##Caveats

The provided functions wait for each command to be confirmed before sending the next. It is be possible to send all commands and then wait for each to complete. This will save some time but is not really necessary, although it might be useful for synchronizing valves more closely.

##Bugs

The semaphores used are not queueing semaphores, so it's possible that if two threads issue read commands at very close times they will read one another's return value. This is not a serious concern: It requires two threads that are using overlapping subsets of the peripherals. It's hard to say that any control system overlapping with another will give good results. Further both threads will usually read the same value.