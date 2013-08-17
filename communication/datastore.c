/* See LICENSE file for license and copyright information */

#include "datastore.h"

int
data_store_read_byte(data_store_t* datastore, unsigned char* byte)
{
  if (datastore == NULL || datastore->read_byte == NULL || byte == NULL) {
    return ERROR_INVALID_ARGUMENTS;
  }

  return datastore->read_byte(datastore, byte);
}

int
data_store_write_byte(data_store_t* datastore, unsigned char byte)
{
  if (datastore == NULL || datastore->write_byte == NULL) {
    return ERROR_INVALID_ARGUMENTS;
  }

  return datastore->write_byte(datastore, byte);
}
