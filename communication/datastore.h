/* See LICENSE file for license and copyright information */

#ifndef DATASTORE_H
#define DATASTORE_H

#include <stdint.h>
#include <stddef.h>

#include "errors.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * The data store provide a facility to store data byte-by-byte. A data store
 * provides exactly two methods:
 *
 * - @a read_byte to read a byte.
 * - @a write_byte to write a byte.
 */
typedef struct data_store_s {
  /**
   * Read a byte from the data store.
   *
   * @param[in] datastore Data store.
   * @param[out] byte Pointer to the variable receiving the byte.
   *
   * @return @ref ERROR_OK on sucess
   * @return @ref ERROR_INVALID_ARGUMENTS Invalid arguments have been passed
   * @return @ref ERROR_EOF End of file
   * @return @ref ERROR_UNKNOWN on any unspecified failure
   */
  int (*read_byte)(struct data_store_s* datastore, unsigned char* byte);

  /**
   * Write a byte to the data store
   *
   * @param[in] datastore Data store.
   * @param[out] byte Pointer to the variable receiving the byte.
   *
   * @return @ref ERROR_OK on sucess
   * @return @ref ERROR_INVALID_ARGUMENTS Invalid arguments have been passed
   * @return @ref ERROR_UNKNOWN on any unspecified failure
   */
  int (*write_byte)(struct data_store_s* datastore, unsigned char byte);

  /**
   * Data store specific data.
   */
  void* data;
} data_store_t;

/**
 * A wrapper around data store's read_byte.
 *
 * @see data_store_t.read_byte
 */
int data_store_read_byte(data_store_t* datastore, unsigned char* byte);

/**
 * A wrapper around data store's write_byte.
 *
 * @see data_store_t.write_byte
 */
int data_store_write_byte(data_store_t* datastore, unsigned char byte);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // DATASTORE_H
