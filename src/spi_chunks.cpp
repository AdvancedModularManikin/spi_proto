#include "stdint.h"
//#include "stddef.h"

//for memcpy
#include <string.h>

#include "spi_proto.h"
#include "spi_chunks.h"
//#include "spi_chunks_slave.h"

#ifdef __cplusplus
extern "C" {
#endif

int out_of_range_chunks;

//TODO check that 0 and 1 length packets are correctly handled
/*
assuming the payload of the spi proto is large but most commands are small the command flow is:
	callback for spi_proto (whole message)
	command dispatcher (individual chunks with length)
		component message processors (type_command structs, provided with their type's array of instances)
*/



//returns the number of bytes used
int
chunk_packer(struct waiting_chunk *chunks, size_t numchunk,
	uint8_t *buf, size_t len)
{
	int ret = 0;
	for (unsigned int i = 0; i < numchunk;i++) {
		if (chunks[i].ready_to_pack) {
			if (chunks[i].buf[0] <= len) {
				ret += chunks[i].buf[0];
				memcpy(buf, chunks[i].buf, chunks[i].buf[0]);
				len -= chunks[i].buf[0];
				buf += chunks[i].buf[0];
				chunks[i].ready_to_pack = 0;
				if (len < 3) break; // 2 is the smallest possible chunk: [2|ID]
			}
		}
	}
	return ret;
}

//so we need a function to dispatch on chunks of the message
int
spi_msg_chunks(uint8_t *buf, size_t len, int (*chunk_handler)(uint8_t *b, size_t len))
{
	unsigned int p = 0;
	
	while (p < len) {
		if (buf[p]) {
			if (p + buf[p] < len) { // doesn't overflow at least
				int ret = chunk_handler(&buf[p], buf[p]); // TODO this can access the rest of the array possibly, but we probably trust it anyway
				p += buf[p] ? buf[p] : 1; // TODO could break on zero or one length chunks if we were guaranteed there wouldn't be any after them
				//TODO do something with ret
			} else {
				return -1;
			}
		} else {
			break;
		}
	}
	return 0;
}

#ifdef __cplusplus
}
#endif
