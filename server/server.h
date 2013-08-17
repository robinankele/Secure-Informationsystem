#ifndef SERVER_H
#define SERVER_H

/** @brief Server interface
 *
 * @file server.h
 */

#include <stddef.h>

#include "communication/channel.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct server_s server_t;

/**
 * Initializes a server. The database referenced by @a database is opened.
 *
 * @param[out] server Pointer to the server
 * @param[in] database Path to the sqlite database file
 *
 * @return @ref ERROR_OK on success.
 * @return Any error code that is returned by @ref database_open.
 * @return @ref ERROR_MEMORY Out of memory.
 * @return @ref ERROR_INVALID_ARGUMENTS Invalid arguments have been passed.
 * @return @ref ERROR_UNKNOWN An unspecified error occurred.
 */
int server_init(server_t** server, const char* database);

/**
 * Processes a packet
 *
 * @param[in] server The server
 * @param[in] data The data
 * @param[in] size The size of data
 * @param[out] response The response data
 * @param[out] response_size The size of the response data
 *
 * @return @ref ERROR_OK on success,
 * @return @ref ERROR_SERVER_SHUTDOWN The server should shutdown
 * @return @ref ERROR_MEMORY Out of memory
 * @return @ref ERROR_INVALID_ARGUMENTS Invalid arguments have been passed
 * @return @ref ERROR_UNKNOWN An unspecified error occurred
 */
int server_process(server_t* server, const unsigned char* data, size_t size, unsigned char** response, size_t* response_size);

/**
 * Closes the server
 *
 * @param[in] server The server, not NULL.
 *
 * @return @ref ERROR_OK on success,
 * @return @ref ERROR_UNKNOWN An unspecified error occurred
 * @return @ref ERROR_INVALID_ARGUMENTS Invalid arguments have been passed
 */
int server_shutdown(server_t* server);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // SERVER_H
