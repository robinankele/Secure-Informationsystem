#include "communication/channel.h"
#include "errors.h"
#include <stdlib.h>
#include <string.h>

int
channel_client_read_bytes(channel_t* channel, unsigned char** bytes, size_t* size)
{
  if (!channel || !channel->client_read_bytes || !bytes || !size) {
    return ERROR_INVALID_ARGUMENTS;
  }

  return channel->client_read_bytes(channel, bytes, size);
}

int
channel_client_write_bytes(channel_t* channel, const unsigned char* bytes, size_t size)
{
  if (!channel || !channel->client_write_bytes || !bytes || size == 0) {
    return ERROR_INVALID_ARGUMENTS;
  }

  return channel->client_write_bytes(channel, bytes, size);
}

int
channel_server_read_bytes(channel_t* channel, unsigned char** bytes, size_t* size)
{
  if (!channel || !channel->server_read_bytes || !bytes || !size) {
    return ERROR_INVALID_ARGUMENTS;
  }

  return channel->server_read_bytes(channel, bytes, size);
}

int
channel_server_write_bytes(channel_t* channel, const unsigned char* bytes, size_t size)
{
  if (!channel || !channel->server_write_bytes || !bytes || size == 0) {
    return ERROR_INVALID_ARGUMENTS;
  }

  return channel->server_write_bytes(channel, bytes, size);
}

int
channel_free(channel_t* channel)
{
  if (channel == NULL || channel->free == NULL) {
    return ERROR_INVALID_ARGUMENTS;
  }

  return channel->free(channel);
}
