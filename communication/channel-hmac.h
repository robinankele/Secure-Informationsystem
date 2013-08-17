#ifndef CHANNEL_HMAC_H
#define CHANNEL_HMAC_H

#include <stddef.h>

/** @brief Message channel with HMAC.
 *
 * The HMAC channel is an implementation of a channel as described in @see
 * channel.h. It's designed as a channel wrapper that uses an existing channel
 * for the real data exchange between different endpoints.
 *
 * The characteristics of an HMAC channel are that it calculates an HMAC for any
 * data that should be send and packs the result at the end of the outgoing
 * message. In addition, it checks every incoming data for a correct appended
 * HMAC which will be stripped out before it is passed to the user.
 *
 * If no key has been set for the HMAC channel the data will be passed further
 * to its child channel without any modifications.
 *
 * @file channel-hmac.h
 */

#include "channel.h"

/**
 * Creates a new HMAC channel
 *
 * @param[out] channel The HMAC channel
 * @param[in] child The child channel. The child channel is owned by the newly
 *  created channel and will be freed automatically.
 *
 * @return @ref ERROR_OK if no error occured
 * @return @ref ERROR_MEMORY Out of memory
 * @return @ref ERROR_INVALID_ARGUMENTS If either the channel or the child is a
 *  @a NULL pointer
 * @return @ref ERROR_UNKNOWN Any unspecified error occured
 */
int channel_hmac_new(channel_t** channel, channel_t* child);

/**
 * Sets the key from the HMAC channel. Iff the specified key is NULL the key of
 * the channel is unset and therefore HMAC is disabled.
 *
 * @param channel The HMAC channel
 * @param key The key that shall be used or NULL if HMAC should be turned off
 * @param len Length of the passed key. Irrelevant if @a key is NULL, non-zero
 *  otherwise.
 *
 * @return @ref ERROR_OK if no error occured
 * @return @ref ERROR_MEMORY Out of memory
 * @return @ref ERROR_INVALID_ARGUMENTS If no channel has been passed to the
 *  function
 * @return @ref ERROR_UNKNOWN Any unspecified error occured
 */
int channel_hmac_set_key(channel_t* channel, const unsigned char* key, size_t len);

#endif // CHANNEL_HMAC_H
