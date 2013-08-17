#ifndef CHANNEL_H
#define CHANNEL_H

#include <stddef.h>

/** @brief Message channel
 *
 * Message channels are used to exchange messages between two end-points.
 * Channels have four methods: (client,server)_read_bytes to receive a message
 * and (client,server_)write_bytes to send a message.
 *
 * Channels have a very simple semantic: Let's call the two end-points A and B
 * for now. A shall be known as the client and B as the server. With this
 * semantic, A connects to B. A sends a message to B. B receives the message
 * and sends a response back to A. A can send multiple messages to B and B
 * shall receive them in the same order.
 *
 * Not that read_bytes and write_bytes may fail with ERROR_CHANNEL_BUSY. This
 * is an indication that their is currently no data to read or that the channel
 * can not send data right now. The caller should try again later in that case.
 *
 * @file channel.h
 */
typedef struct channel_s
{
  /**
   * Write bytes to the channel and send them to the server.
   *
   * @param[in] channel The channel.
   * @param[in] bytes Bytes to be written.
   * @param[in] size Number of bytes in bytes.
   *
   * @return @ref ERROR_OK on success
   * @return @ref ERROR_CHANNEL_BUSY The channel is busy
   * @return @ref ERROR_INVALID_ARGUMENTS If the @a channel is invalid, @bytes
   *  is NULL or size is 0
   * @return @ref ERROR_UNKNOWN Any other error occured
   */
  int (*client_write_bytes)(struct channel_s* channel, const unsigned char* bytes, size_t size);

  /**
   * Read bytes from the channel received from the server.
   *
   * The caller is responsible to free the memory pointed to by @a bytes.
   *
   * @param[in] channel The channel.
   * @param[out] bytes Pointer to the variable where pointer to the bytes
   *   should be stored.
   * @param[out] size Pointer to the variable where the number of bytes should
   *   be store.
   *
   * @return @ref ERROR_OK on success
   * @return @ref ERROR_CHANNEL_BUSY The channel is busy
   * @return @ref ERROR_INVALID_ARGUMENTS If the @a channel is invalid, @bytes
   *  is NULL or size is NULL
   * @return @ref ERROR_UNKNOWN Any other error occured
   */
  int (*client_read_bytes)(struct channel_s* channel, unsigned char** bytes, size_t* size);

  /**
   * Write bytes to the channel and send them to the client.
   *
   * @param[in] channel The channel.
   * @param[in] bytes Bytes to be written.
   * @param[in] size Number of bytes in bytes.
   *
   * @return @ref ERROR_OK on success
   * @return @ref ERROR_CHANNEL_BUSY The channel is busy
   * @return @ref ERROR_INVALID_ARGUMENTS If the @a channel is invalid, @bytes
   *  is NULL or size is 0
   * @return @ref ERROR_UNKNOWN Any other error occured
   */
  int (*server_write_bytes)(struct channel_s* channel, const unsigned char* bytes, size_t size);

  /**
   * Read bytes from the channel received from the client.
   *
   * The caller is responsible to free the memory pointed to by @a bytes.
   *
   * @param[in] channel The channel.
   * @param[out] bytes Pointer to the variable where pointer to the bytes
   *   should be stored.
   * @param[out] size Pointer to the variable where the number of bytes should
   *   be store.
   *
   * @return @ref ERROR_OK on success
   * @return @ref ERROR_CHANNEL_BUSY The channel is busy
   * @return @ref ERROR_INVALID_ARGUMENTS If the @a channel is invalid, @bytes
   *  is NULL or size is NULL
   * @return @ref ERROR_UNKNOWN Any other error occured
   */
  int (*server_read_bytes)(struct channel_s* channel, unsigned char** bytes, size_t* size);

  /**
   * Frees the channel and all of its allocated ressources.
    *
    * @param[in] channel The channel.
    *
    * @return @ref ERROR_OK on success, an error code otherwise.
    * @return @ref ERROR_INVALID_ARGUMENTS If the channel is invalid
    * @return @ref ERROR_UNKNOWN Any other error occured
    */
  int (*free)(struct channel_s* channel);

  /**
   * Channel specific data.
   */
  void* data;
} channel_t;

/**
 * Wrapper around the channel's client_read_bytes.
 *
 * @see channel_t.client_read_bytes
 */
int channel_client_read_bytes(channel_t* channel, unsigned char** bytes, size_t* size);

/**
 * Wrapper around the channel's client_write_bytes.
 *
 * @see channel_t.clent_write_bytes
 */
int channel_client_write_bytes(channel_t* channel, const unsigned char* bytes, size_t size);

/** \brief Wrapper around the channel's server_read_bytes.
 *
 * @see channel_t.server_read_bytes
 */
int channel_server_read_bytes(channel_t* channel, unsigned char** bytes, size_t* size);

/**
 * Wrapper around the channel's server_write_bytes.
 *
 * @see channel_t.server_write_bytes
 */
int channel_server_write_bytes(channel_t* channel, const unsigned char* bytes, size_t size);

/** Wrapper around the channel's frees.
 *
 * @see channel_t.free
 */
int channel_free(channel_t* channel);

#endif
