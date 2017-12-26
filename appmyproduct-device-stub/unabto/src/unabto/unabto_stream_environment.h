#ifndef _UNABTO_STREAM_ENVIRONMENT_H_
#define _UNABTO_STREAM_ENVIRONMENT_H_

#include <unabto/unabto_stream_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Build and send a packet.
 * @param stream       the stream
 * @param type         the stream window type
 * @param seq          the xmit sequence number
 * @param winInfoData  the window payload additional data (in network order)
 * @param winInfoSize  size of the window payload additional data
 * @param data         the data to be encrypted
 * @param size         size of the data to be encrypted
 * @return             true if packet is sent (and then stream->ackSent is updated)
 */

bool unabto_stream_send_rst_packet(struct nabto_stream_s* stream, struct nabto_win_info* win);
bool build_and_send_packet(struct nabto_stream_s* stream, uint8_t type, uint32_t seq, const uint8_t* winInfoData, size_t winInfoSize, uint8_t* data, uint16_t size, struct nabto_stream_sack_data* sackData);

void stream_reset(struct nabto_stream_s* stream);
void unabto_stream_init_buffers(struct nabto_stream_s* stream);
bool unabto_stream_is_connection_reliable(struct nabto_stream_s* stream);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
