#include "channel-endpoint-connector.h"
#include "errors.h"
#include <stdlib.h>
#include <string.h>

typedef struct channel_endpoint_connector_s {
  channel_t* server;
  channel_t* endpoint;
  unsigned char* client_bytes;
  size_t client_size;
  unsigned char* server_bytes;
  size_t server_size;
} channel_endpoint_connector_t;

static int
cec_free(channel_t* channel)
{
  if (channel == NULL) {
    return ERROR_INVALID_ARGUMENTS;
  }

  channel_endpoint_connector_t* endpoint = channel->data;
  channel_free(endpoint->server);
  free(endpoint->client_bytes);
  free(endpoint->server_bytes);
  free(endpoint);
  free(channel);
  return ERROR_OK;
}

static int
cec_client_read_bytes(channel_t* channel, unsigned char** bytes, size_t* size)
{
  if (channel == NULL || channel->data == NULL || bytes == NULL || size == NULL) {
    return ERROR_INVALID_ARGUMENTS;
  }

  channel_endpoint_connector_t* endpoint = channel->data;
  if (endpoint->endpoint == NULL || endpoint->server == NULL) {
    return ERROR_INVALID_ARGUMENTS;
  }

  if (endpoint->client_bytes != NULL) {
    return ERROR_CHANNEL_BUSY;
  }

  /* get the data from the server */
  unsigned char* tmp_bytes = NULL;
  size_t tmp_size = 0;
  int err = channel_client_read_bytes(endpoint->server, &tmp_bytes, &tmp_size);
  if (err != ERROR_OK) {
    return err;
  }

  /* pass it through the chain */
  err = channel_server_write_bytes(endpoint->endpoint, tmp_bytes, tmp_size);
  free(tmp_bytes);
  if (err != ERROR_OK) {
    return err;
  }

  *bytes = endpoint->client_bytes;
  *size = endpoint->client_size;
  endpoint->client_bytes = NULL;
  endpoint->client_size = 0;
  return ERROR_OK;
}

static int
cec_client_write_bytes(channel_t* channel, const unsigned char* bytes, size_t size)
{
  if (channel == NULL || channel->data == NULL || bytes == NULL || size == 0) {
    return ERROR_INVALID_ARGUMENTS;
  }

  channel_endpoint_connector_t* endpoint = channel->data;
  if (endpoint->endpoint == NULL || endpoint->server == NULL) {
    return ERROR_INVALID_ARGUMENTS;
  }

  /* store the data */
  if (endpoint->server_bytes != NULL) {
    return ERROR_CHANNEL_BUSY;
  }

  endpoint->server_bytes = malloc(size);
  if (endpoint->server_bytes == NULL) {
    return ERROR_MEMORY;
  }

  memcpy(endpoint->server_bytes, bytes, size);
  endpoint->server_size = size;

  /* simulate a server */
  unsigned char* tmp_bytes = NULL;
  size_t tmp_size = 0;
  int err = channel_server_read_bytes(endpoint->endpoint, &tmp_bytes, &tmp_size);
  free(endpoint->server_bytes);
  endpoint->server_bytes = NULL;
  endpoint->server_size = 0;

  if (err != ERROR_OK) {
    return err;
  }

  /* write bytes to the server */
  err = channel_client_write_bytes(endpoint->server, tmp_bytes, tmp_size);
  free(tmp_bytes);

  return err;
}

static int
cec_server_read_bytes(channel_t* channel, unsigned char** bytes, size_t* size)
{
  if (channel == NULL || channel->data == NULL || bytes == NULL || size == NULL) {
    return ERROR_INVALID_ARGUMENTS;
  }

  channel_endpoint_connector_t* endpoint = channel->data;
  if (endpoint->endpoint == NULL || endpoint->server == NULL) {
    return ERROR_INVALID_ARGUMENTS;
  }

  if (endpoint->server_bytes == NULL) {
    return ERROR_CHANNEL_BUSY;
  }

  *bytes = endpoint->server_bytes;
  *size = endpoint->server_size;
  endpoint->server_bytes = NULL;
  endpoint->server_size = 0;
  return ERROR_OK;
}

static int
cec_server_write_bytes(channel_t* channel, const unsigned char* bytes, size_t size)
{
  if (channel == NULL || channel->data == NULL || bytes == NULL || size == 0) {
    return ERROR_INVALID_ARGUMENTS;
  }

  channel_endpoint_connector_t* endpoint = channel->data;
  if (endpoint->endpoint == NULL || endpoint->server == NULL) {
    return ERROR_INVALID_ARGUMENTS;
  }

  /* store the data */
  if (endpoint->client_bytes != NULL) {
    return ERROR_CHANNEL_BUSY;
  }

  endpoint->client_bytes = malloc(size);
  if (endpoint->client_bytes == NULL) {
    return ERROR_MEMORY;
  }

  memcpy(endpoint->client_bytes, bytes, size);
  endpoint->client_size = size;
  return ERROR_OK;
}

int
channel_endpoint_connector_new(channel_t** channel, channel_t* server)
{
  if (channel == NULL || server == NULL) {
    return ERROR_INVALID_ARGUMENTS;
  }

  channel_t* tmp = calloc(1, sizeof(channel_t));
  if (tmp == NULL) {
    return ERROR_MEMORY;
  }
  tmp->free = cec_free;
  tmp->client_read_bytes = cec_client_read_bytes;
  tmp->client_write_bytes = cec_client_write_bytes;
  tmp->server_read_bytes = cec_server_read_bytes;
  tmp->server_write_bytes = cec_server_write_bytes;

  channel_endpoint_connector_t* endpoint = calloc(1, sizeof(channel_endpoint_connector_t));
  if (endpoint == NULL) {
    channel_free(tmp);
    return ERROR_MEMORY;
  }

  endpoint->server = server;
  tmp->data = endpoint;
  *channel = tmp;

  return ERROR_OK;
}

int
channel_endpoint_connector_set_endpoint(channel_t* channel, channel_t* endpoint)
{
  if (channel == NULL || channel->data == NULL || endpoint == NULL) {
    return ERROR_INVALID_ARGUMENTS;
  }

  channel_endpoint_connector_t* cendpoint = channel->data;
  cendpoint->endpoint = endpoint;
  return ERROR_OK;
}
