#include "simple-memory-buffer.h"
#include <stdlib.h>
#include <string.h>

typedef struct simple_memory_buffer_s
{
  unsigned char* buffer;
  size_t size;
  size_t position;
} simple_memory_buffer_t;

static int
simple_memory_buffer_read(data_store_t* datastore, unsigned char* byte)
{
  if (!datastore || !datastore->data || !byte) {
    return ERROR_INVALID_ARGUMENTS;
  }

  simple_memory_buffer_t* buffer = datastore->data;
  if (buffer->position >= buffer->size) {
    return ERROR_EOF;
  }

  // Read a byte.
  *byte = buffer->buffer[buffer->position++];
  return 0;
}

static int
simple_memory_buffer_write(data_store_t* datastore, unsigned char byte)
{
  if (!datastore || !datastore->data) {
    return ERROR_INVALID_ARGUMENTS;
  }

  simple_memory_buffer_t* buffer = datastore->data;
  if (buffer->position >= buffer->size) {
    unsigned char* tmp = realloc(buffer->buffer, buffer->size + 1);
    if (!tmp) {
      return ERROR_MEMORY;
    }
    buffer->buffer = tmp;
    ++buffer->size;
  }

  // Write a byte.
  buffer->buffer[buffer->position++] = byte;
  return 0;
}

int
simple_memory_buffer_new(data_store_t* handle, const unsigned char* data, size_t size)
{
  if (!handle) {
    return ERROR_INVALID_ARGUMENTS;
  }

  /* Allocate memory for our handle. */
  simple_memory_buffer_t* buffer = malloc(sizeof(simple_memory_buffer_t));
  if (!buffer) {
    return ERROR_MEMORY;
  }

  /* Initialize data store info */
  handle->data       = buffer;
  handle->read_byte  = &simple_memory_buffer_read;
  handle->write_byte = &simple_memory_buffer_write;

  /* Zero out everything. */
  buffer->buffer   = NULL;
  buffer->size     = 0;
  buffer->position = 0;

  /* Copy data if we have pre existing data. */
  if (size != 0) {
    buffer->buffer = malloc(size);
    if (buffer->buffer == NULL) {
      simple_memory_buffer_free(handle);
      return ERROR_MEMORY;
    }

    buffer->size = size;

    if (data != NULL) {
      memcpy(buffer->buffer, data, size);
    }
  }

  return ERROR_OK;
}

int simple_memory_buffer_free(data_store_t* handle)
{
  if (!handle || !handle->data) {
    return ERROR_INVALID_ARGUMENTS;
  }

  // Free everything.
  simple_memory_buffer_t* buffer = handle->data;
  free(buffer->buffer);
  free(buffer);

  return ERROR_OK;
}

int simple_memory_buffer_get_data(data_store_t* handle, unsigned char** data)
{
  if (!handle || !handle->data || !data) {
    return ERROR_INVALID_ARGUMENTS;
  }

  *data = ((simple_memory_buffer_t*)handle->data)->buffer;

  return ERROR_OK;
}

int simple_memory_buffer_get_size(data_store_t* handle, size_t* size)
{
  if (!handle || !handle->data || !size) {
    return ERROR_INVALID_ARGUMENTS;
  }

  *size = ((simple_memory_buffer_t*)handle->data)->size;

  return ERROR_OK;
}
