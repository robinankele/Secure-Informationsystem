#ifndef REGISTRY_H
#define REGISTRY_H

/** @brief Public API of libregistry.
 *
 *  The error code @a ERROR_REGISTRY_INVALID_STATE is used if and only if the
 *  registry received an error packet and the error code inside is equal to
 *  ERROR_DATABASE_INVALID.
 *
 *  All the handles may not be @a NULL. All domains and keys may not be @a NULL and
 *  have to be non-empty NUL-terminated strings. The target pointers in the
 *  registry_get_* functions and in registry_enum_keys may not be @a NULL. Blobs
 *  and strings may not be @a NULL.
 *
 *  All values are passed unmodified to the server.
 *
 *  @file  registry.h
 */

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "communication/channel.h"

typedef struct registry_s registry_t;

/**
 * Open a connection to the registry.
 *
 * @param[out] handle Pointer to the registry handle.
 *   The handle will be set to @a NULL on failure.
 * @param[in] identifier The registry identifier. The identifier must not be @a NULL and
 *   describes which registry implementation is used:
 *
 *    * file://<path> - create a channel_with_server instance
 *    * hmac://<key>  - create a channel_hmac instance
 *
 *  They can be seperated by |, e.g. file://<path>|hmac://<key> creates a
 *  channel_with_server instance using the datbase found at path and also
 *  creates a channel_hmac instance with <key> as its' key. Please note that
 *  hmac://<key>|file://<path> is not valid since there is no child channel
 *  that could be passed to channel_hmac. hmac:// can occur multiple times in
 *  the identifer: file://<path>|hmac://<key>|hmac://<key> is valid and creates
 *  a chain of one channel_with_server instance and two channel_hmac instances.
 *
 *  To put the channel-hmac and channel-with-server instances together, please
 *  have a look at channel-endpoint-connector.
 *
 *  If the identifier is not valid, ERROR_REGISTRY_UNKNOWN_IDENTIFIER is
 *  returned.
 *
 * @param[in] domain The domain identifier. The identifier must be a valid
 *   domain name and can be any arbitrary non-empty non-NULL string.
 *
 * @return @ref ERROR_OK on success,
 * @return @ref ERROR_INVALID_ARGUMENTS Invalid arguments have been passed
 * @return @ref ERROR_REGISTRY_UNKNOWN_IDENTIFIER Specified identifier is not
 *   known
 * @return @ref ERROR_MEMORY Out of memory
 * @return @ref ERROR_UNKNOWN An unspecified error occurred
 */
int registry_open(registry_t** handle, const char* identifier,
    const char* domain);

/**
 * Closes the given connection to registry.
 *
 * @param[in] handle A registry handle. If the handle is @a NULL, this function
 *   is a no-op and returns @ref ERROR_INVALID_ARGUMENTS.
 *
 * @return @ref ERROR_OK on success,
 * @return @ref ERROR_UNKNOWN An unspecified error occurred
 * @return @ref ERROR_INVALID_ARGUMENTS Invalid arguments have been passed
 */
int registry_close(registry_t* handle);

/**
 * Retrieve a signed 64-bit integer value from the registry.
 *
 * @param[in] handle A valid registry handle. %TODO return values
 * @param[in] key The key name of the value that should to be retrieved.
 * @param[out] value Pointer to the variable receiving the value.
 *
 * @return @ref ERROR_OK on success,
 * @return @ref ERROR_REGISTRY_NO_SUCH_KEY Given key does not exist
 * @return @ref ERROR_REGISTRY_INVALID_STATE Corrupt database
 * @return @ref ERROR_INVALID_ARGUMENTS Invalid arguments have been passed
 * @return @ref ERROR_UNKNOWN An unspecified error occurred
 */
int registry_get_int64(registry_t* handle, const char* key, int64_t* value);

/** Set a signed 64-bit integer value in the registry.
 *
 * @param[in] handle A valid registry handle.
 * @param[in] key The key name of the value that should to be set.
 * @param[in] value The value.
 *
 * @return @ref ERROR_OK on success,
 * @return @ref ERROR_REGISTRY_INVALID_STATE Corrupt database
 * @return @ref ERROR_INVALID_ARGUMENTS Invalid arguments have been passed
 * @return @ref ERROR_UNKNOWN An unspecified error occurred
 */
int registry_set_int64(registry_t* handle, const char* key, int64_t value);

/**
 * Retrieve a double precision floating point value from the registry.
 *
 * @param[in] handle A valid registry handle.
 * @param[in] key The key name of the value that should to be retrieved.
 * @param[out] value Pointer to the variable receiving the value.
 *
 * @return @ref ERROR_OK on success,
 * @return @ref ERROR_REGISTRY_INVALID_STATE Corrupt database
 * @return @ref ERROR_REGISTRY_NO_SUCH_KEY Given key does not exist
 * @return @ref ERROR_INVALID_ARGUMENTS Invalid arguments have been passed
 * @return @ref ERROR_UNKNOWN An unspecified error occurred
 */
int registry_get_double(registry_t* handle, const char* key, double* value);

/** Set a double precission floating point value in the registry.
 *
 * @param[in] handle A valid registry handle.
 * @param[in] key The key name of the value that should to be set.
 * @param[in] value The value.
 *
 * @return @ref ERROR_OK on success,
 * @return @ref ERROR_REGISTRY_INVALID_STATE Corrupt database
 * @return @ref ERROR_INVALID_ARGUMENTS Invalid arguments have been passed
 * @return @ref ERROR_UNKNOWN An unspecified error occurred
 */
int registry_set_double(registry_t* handle, const char* key, double value);

/**
 * Retrieve a NUL-terminanted string from the registry.
 *
 * @param[in] handle A valid registry handle.
 * @param[in] key The key name of the value that should be set.
 * @param[out] value Pointer to the variable receiving the NULL-terminanted value.
 *
 * @return @ref ERROR_OK on success,
 * @return @ref ERROR_REGISTRY_INVALID_STATE Corrupt database
 * @return @ref ERROR_REGISTRY_NO_SUCH_KEY Given key does not exist
 * @return @ref ERROR_INVALID_ARGUMENTS Invalid arguments have been passed
 * @return @ref ERROR_UNKNOWN An unspecified error occurred
 */
int registry_get_string(registry_t* handle, const char* key, char** value);

/**
 * Set a NUL-terminanted string in the registry.
 *
 * @param[in] handle A valid registry handle.
 * @param[in] key The key name of the value that should be set.
 * @param[in] value A NUL-terminated string.
 *
 * @return @ref ERROR_OK on success,
 * @return @ref ERROR_REGISTRY_INVALID_STATE Corrupt database
 * @return @ref ERROR_INVALID_ARGUMENTS Invalid arguments have been passed
 * @return @ref ERROR_UNKNOWN An unspecified error occurred
 */
int registry_set_string(registry_t* handle, const char* key, const char* value);

/** Retrieve a blob value from the registry.
 *
 * @param[in] handle A valid registry handle.
 * @param[in] key The key name of the value that should be set.
 * @param[out] size Pointer to variable receiving the size of the blob.
 * @param[out] value Pointer to the variable receiving the blob.
 *
 * @return @ref ERROR_OK on success,
 * @return @ref ERROR_REGISTRY_INVALID_STATE Corrupt database
 * @return @ref ERROR_REGISTRY_NO_SUCH_KEY Given key does not exist
 * @return @ref ERROR_INVALID_ARGUMENTS Invalid arguments have been passed
 * @return @ref ERROR_UNKNOWN An unspecified error occurred
 */
int registry_get_blob(registry_t* handle, const char* key, unsigned char** value, size_t* size);

/** Set a blob value in the registry.
 *
 * @param[in] handle A valid registry handle.
 * @param[in] key The key name of the value that should be set.
 * @param[in] size Size of the blob.
 * @param[in] value The blob of @a size bytes.
 *
 * @return @ref ERROR_OK on success,
 * @return @ref ERROR_REGISTRY_INVALID_STATE Corrupt database
 * @return @ref ERROR_INVALID_ARGUMENTS Invalid arguments have been passed
 * @return @ref ERROR_UNKNOWN An unspecified error occurred
 */
int registry_set_blob(registry_t* handle, const char* key, const unsigned char* value, size_t size);

/** Enumerate keys according to a pattern.
 *
 * The keys are returned in one large string that is separated by @a 0s. For
 * valid patterns have a look at @ref database_enum_keys. The caller is
 * responsible the free the memory block referenced by keys.
 *
 * @param[in] handle A valid registry handle.
 * @param[in] pattern The key pattern.
 * @param[out] count Count of enumerated keys.
 * @param[out] size Size of the keys.
 * @param[out] keys The enumerated keys.
 *
 * @return @ref ERROR_OK on success,
 * @return @ref ERROR_REGISTRY_INVALID_STATE Corrupt database
 * @return @ref ERROR_INVALID_ARGUMENTS Invalid arguments have been passed
 * @return @ref ERROR_UNKNOWN An unspecified error occurred
 */
int registry_enum_keys(registry_t* handle, const char* pattern, size_t* count, size_t* size, char** keys);

/**
 * Retrieves the type of a key
 *
 * @param[in] handle A valid registry handle.
 * @param[in] key The key name of the value that should be used.
 * @param[out] type The type of the key
 *
 * @return @ref ERROR_OK on success,
 * @return @ref ERROR_REGISTRY_INVALID_STATE Corrupt database
 * @return @ref ERROR_REGISTRY_NO_SUCH_KEY Given key does not exist
 * @return @ref ERROR_INVALID_ARGUMENTS Invalid arguments have been passed
 * @return @ref ERROR_UNKNOWN An unspecified error occurred
 */
int registry_key_get_value_type(registry_t* handle, const char* key, int* type);

/**
 * Returns the channel object from the registry handle.
 *
 * @param handle A valid registry handle
 *
 * @return Reference to the used channel or NULL. The channel is still owned by
 * the registry.
 */
channel_t* registry_get_channel(registry_t* handle);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // REGISTRY_H
