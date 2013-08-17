#ifndef CHANNEL_ENDPOINT_CONNECT_H
#define CHANNEL_ENDPOINT_CONNECT_H

/** @brief Message channel that connects different endpoints.
 *
 * The endpoint connector channel can be used to link different channels and
 * a channel-with-server like channel together. Let's assume we have two
 * different channels A and B where A is used on the client side and B is used
 * on the server side. B uses a channel-with-server at the bottom to act as a
 * server. Call the channel-with-server S. The following diagram illustrates
 * the situation:
 *
 *  client       server
 * /------\    /--------\
 * |  A   | -- | B -- S |
 * \------/    \--------/
 *
 * The registry uses @a channel_client_read_bytes and @a
 * channel_client_write_bytes on A to send and receive packets from the server.
 *
 * channel-endpoint-connector can be used to connect A and B. This is done in
 * the following way: let's call the channel-endpoint-connect instance C. On the
 * client side A is changed such that C is on the bottom of A, i.e. anything
 * that is sent or received by A goes through C first (similar to the child in
 * channel-hmac). On the server side S is replaced by C and C handles S. S is
 * passed as @a server argument in @ref channel_endpoint_connector_new and B is
 * passed as @a endpoint argument in @ref
 * channel_endpoint_connector_set_endpoint.
 *
 * These channels come in handy if you want to set up a channel consisting of
 * channel-hmacs and a channel-with-server. Suppose you have some HMAC channels
 * on the client side, let's call them A1, ... An, and some on the server side,
 * let's call them B1, ..., Bn. On the server side there's also a
 * channel-with-server called S. Then the channel-endpoint-connector C is used as
 * child of An, the endpoint is B1 and C is also a child of Bn. S is used as
 * server of C. (Note that A1 ... An and B1 ...  Bn perform exactly the same
 * thing, i.e the key of A1 is the same as the one of B1 and so on, so they can
 * be reused). In pseudo code this can look like the following code:
 *
 *  * create channel-with-server S
 *  * create channel-endpoint-connector C with S as server
 *  * create the channel-hmacs such that C is the child of An.
 *  * set A1 as endpoint of C
 *
 * @file channel-endpoint-connector.h
 */

#include "channel.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/** Create a new endpoint connector channel.
 *
 * @param[out] channel The endpoint connector channel
 * @param[in] server The server channel. The server channel is owned by the
 *  newly created channel and will be freed automatically.
 *
 * @return @ref ERROR_OK if no error occurred
 * @return @ref ERROR_MEMORY Out of memory
 * @return @ref ERROR_INVALID_ARGUMENTS If either the channel or the server is a
 *  @a NULL pointer
 * @return @ref ERROR_UNKNOWN Any unspecified error occurred
 */
int channel_endpoint_connector_new(channel_t** channel, channel_t* server);

/** Set endpoint.
 *
 * @param[in] channel A non-NULL endpoint connector channel.
 * @param[in] endpoint A non-NULL channel.
 * @return @ref ERROR_OK on success,
 * @return @ref ERROR_INVALID_ARGUMENTS Invalid arguments have been passed
 */
int channel_endpoint_connector_set_endpoint(channel_t* channel, channel_t* endpoint);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif
