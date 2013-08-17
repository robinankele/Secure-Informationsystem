#ifndef CHANNEL_WITH_SERVER_H
#define CHANNEL_WITH_SERVER_H

/**
 * @brief Convenience channel that spawns its' own server instance with the
 * database specified by the passed location.
 *
 * @file channel-with-server.h
 */

#include "communication/channel.h"

/**
 * Creates a client-only channel that has its own server spawned.
 *
 * @param[out] channel Pointer to the variable where the channel handle should
 *   be stored.
 * @param[in] database Path to the database.
 *
 * @return @ref ERROR_OK success,
 * @return @ref ERROR_MEMORY Out of memory
 * @return @ref ERROR_INVALID_ARGUMENTS If no channel or no database location
 *  has been passed
 * @return @ref ERROR_UNKNOWN Any unspecified error occured
 * */
int channel_with_server_new(channel_t** channel, const char* database);

#endif // CHANNEL_WITH_SERVER_H
