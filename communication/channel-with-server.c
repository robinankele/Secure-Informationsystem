#include <stdlib.h>
#include <string.h>

#include "server/server.h"
#include "communication/channel.h"
#include "communication/channel-with-server.h"
#include "errors.h"

typedef struct channel_server_s {
  size_t client_size;
  void* client_data;
  server_t* server;
} channel_server_t;

static int
cs_client_read_bytes(channel_t* channel, unsigned char** bytes, size_t* size)
{
  if (channel == NULL || channel->data == NULL || bytes == NULL || size == NULL) {
    return ERROR_INVALID_ARGUMENTS;
  }

  channel_server_t* cs = channel->data;
  if (cs->client_data == NULL) {
    return ERROR_CHANNEL_BUSY;
  }

  *bytes = cs->client_data;
  *size  = cs->client_size;

  cs->client_data = NULL;
  cs->client_size = 0;

  return ERROR_OK;
}

static int
cs_client_write_bytes(channel_t* channel, const unsigned char* bytes, size_t size)
{
  if (channel == NULL || channel->data == NULL || bytes == NULL || size == 0) {
    return ERROR_INVALID_ARGUMENTS;
  }

  channel_server_t* cs = channel->data;
  if (cs->client_data != NULL) {
    return ERROR_CHANNEL_BUSY;
  }

  unsigned char* result = NULL;
  size_t ressize = 0;
  int ret = server_process(cs->server, bytes, size, &result, &ressize);
  if (ret != ERROR_OK) {
    free(result);
    return ERROR_CHANNEL_FAILED;
  }

  while ((ret = channel_server_write_bytes(channel, result, ressize)) == ERROR_CHANNEL_BUSY);

  free(result);
  return ret;
}

static int
cs_server_write_bytes(channel_t* channel, const unsigned char* bytes, size_t size)
{
  if (channel == NULL || channel->data == NULL || bytes == NULL || size == 0) {
    return ERROR_INVALID_ARGUMENTS;
  }

  channel_server_t* cs = channel->data;
  if (cs->client_data != NULL) {
    return ERROR_CHANNEL_BUSY;
  }

  cs->client_data = malloc(size);
  if (cs->client_data == NULL) {
    return ERROR_MEMORY;
  }

  memcpy(cs->client_data, bytes, size);

  cs->client_size = size;

  return ERROR_OK;
}

static int
cs_free(channel_t* channel)
{
  if (channel == NULL) {
    return ERROR_INVALID_ARGUMENTS;
  }

  channel_server_t* cs = channel->data;
  if (cs != NULL) {
    free(cs->client_data);
    server_shutdown(cs->server);
    free(cs);
  }

  free(channel);

  return ERROR_OK;
}

int
channel_with_server_new(channel_t** channel, const char* database)
{
  if (channel == NULL || database == NULL || strlen(database) == 0) {
    return ERROR_INVALID_ARGUMENTS;
  }

  *channel = calloc(1, sizeof(channel_t));
  if (*channel == NULL) {
    return ERROR_MEMORY;
  }

  (*channel)->client_read_bytes  = cs_client_read_bytes;
  (*channel)->client_write_bytes = cs_client_write_bytes;
  (*channel)->server_read_bytes  = NULL;
  (*channel)->server_write_bytes = cs_server_write_bytes;
  (*channel)->free               = cs_free;

  (*channel)->data = calloc(1, sizeof(channel_server_t));
  if ((*channel)->data == NULL) {
    channel_free(*channel);
    return ERROR_MEMORY;
  }

  channel_server_t* cs = (*channel)->data;
  int ret = server_init(&cs->server, database);
  if (ret != ERROR_OK) {
    channel_free(*channel);
  }

  return ret;
}
