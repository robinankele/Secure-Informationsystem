#ifndef SIMPLE_MEMORY_BUFFER_H
#define SIMPLE_MEMORY_BUFFER_H

/**
 * @brief A simple in-memory buffer
 *
 * @file simple-memory-buffer.h
 */

#include "datastore.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * Create new in-memory data store.
 *
 * @param handle The datastore handle
 * @param data Data
 * @param size Length of data
 *
 * @return @ref ERROR_OK No error occured
 * @return @ref ERROR_MEMORY Out of memory
 * @return @ref ERROR_INVALID_ARGUMENTS Invalid arguments have been passed
 * @return @ref ERROR_UNKNOWN Any unspecified error occured
 */
int simple_memory_buffer_new(data_store_t* handle, const unsigned char* data, size_t size);

/**
 * Destroy in-memory data store.
 *
 * @param handle The data store handle
 *
 * @return @ref ERROR_OK No error occured
 * @return @ref ERROR_INVALID_ARGUMENTS Invalid arguments have been passed
 * @return @ref ERROR_UNKNOWN Any unspecified error occured
 */
int simple_memory_buffer_free(data_store_t* handle);

/**
 * Get the data of the in-memory data store.
 *
 * @param handle The data store handle
 * @param data Data buffer
 *
 * @return @ref ERROR_OK No error occured
 * @return @ref ERROR_INVALID_ARGUMENTS Invalid arguments have been passed
 * @return @ref ERROR_UNKNOWN Any unspecified error occured
 */
int simple_memory_buffer_get_data(data_store_t* handle, unsigned char** data);

/**
 * Get the size of the in-memory data store.
 *
 * @param handle The data store handle
 * @param size Location where the size should be saved at
 *
 * @return @ref ERROR_OK No error occured
 * @return @ref ERROR_INVALID_ARGUMENTS Invalid arguments have been passed
 * @return @ref ERROR_UNKNOWN Any unspecified error occured
 */
int simple_memory_buffer_get_size(data_store_t* handle, size_t* size);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif
