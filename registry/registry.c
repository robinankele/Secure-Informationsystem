/** @brief Public API of libregistry.
 *
 * This file contains the client side implementation of 'the registry'
 *
 *  @file  registry.c
 */

#include "registry.h"
#include "../datastructure.h"
#include "../errors.h"
#include "../memory.h"
#include "../communication/channel.h"
#include "../communication/channel-hmac.h"
#include "../communication/channel-with-server.h"
#include "../communication/channel-endpoint-connector.h"
#include "../communication/simple-memory-buffer.h"
#include "../communication/bpack.h"
#include "../communication/datastore.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* Typedefs and Defines */
/* -------------------------------------------------------------------------- */
#define DELIMITER '|'
#define FILE "file://"
#define HMAC "hmac://"


/* Prototyping */
/* -------------------------------------------------------------------------- */
void checkDelimiterAndSetToTerminator(char **id, uint64_t *position, 
                                      uint64_t size, int8_t *delimiter);


/* Implementation */
/* -------------------------------------------------------------------------- */
int
registry_open(registry_t** handle, const char* identifier, const char* domain)
{
  if(handle == NULL)
    return ERROR_INVALID_ARGUMENTS;
  if(identifier == NULL || domain == NULL || strlen(domain) == 0){
    *handle = NULL;
    return ERROR_INVALID_ARGUMENTS;
  }
  if(strlen(identifier) == 0){
    *handle = NULL;
    return ERROR_REGISTRY_UNKNOWN_IDENTIFIER;
  }

  /* get memory for registry */
  if(requestMemory((void**)handle, sizeof(registry_t)) != ERROR_OK){
    *handle = NULL;
    return ERROR_MEMORY;
  }
  /* initialize handle */ 
  (*handle)->domain = NULL;
  (*handle)->channel = NULL;
  (*handle)->endpoint = NULL;

  /* dublicating identifier */
  char *id = NULL;
  uint64_t size = strlen(identifier);
  if(requestMemory((void**)&id, (size + 1) * sizeof(char)) != ERROR_OK){
    freeMemory(*handle);
    *handle = NULL;
    return ERROR_MEMORY;
  }
  memcpy(id, identifier, size);
  id[size] = '\0';

  char protocol[8] = {'\0'};
  uint64_t position = 0;

  if(size < 15){
    freeMemory(*handle);
    *handle = NULL;
    freeMemory(id);
    return ERROR_REGISTRY_UNKNOWN_IDENTIFIER;
  }

  memcpy(protocol, id, 7);
  protocol[7] = '\0';

  /* check if file:// */ 
  int8_t delimiter = 0;
  if(strncmp(protocol, FILE, 7) == 0){
    position += 7;
    checkDelimiterAndSetToTerminator(&id, &position, size, &delimiter);
  }else{
    freeMemory(*handle);
    *handle = NULL;
    freeMemory(id);
    return ERROR_REGISTRY_UNKNOWN_IDENTIFIER;
  }

  channel_t *channel = NULL;
  int8_t channel_ok = channel_with_server_new(&channel, id + 7);
  if(channel_ok != ERROR_OK){ 
    freeMemory(*handle);
    *handle = NULL;
    freeMemory(id);
    return channel_ok;
  }

  uint64_t start_position = 0;
  /* There is something beyond file://<path> */ 

  /* create channel-endpoint-connector */
  int8_t endpoint_existing = 0;
  if(position < size){
    int8_t channel_end = channel_endpoint_connector_new(&channel, channel);
    if(channel_end != ERROR_OK){
      freeMemory(*handle);
      *handle = NULL;
      freeMemory(id);
      if(channel_free(channel) != ERROR_OK)
        return ERROR_UNKNOWN;
      return channel_end;
    }
    (*handle)->endpoint = channel;
    endpoint_existing = 1;
  }

  while(position < size){
    memcpy(protocol, id + position, 7);

    if(strncmp(protocol, HMAC, 7) == 0){
      delimiter = 0;
      position += 7; 
      start_position = position;
      checkDelimiterAndSetToTerminator(&id, &position, size, &delimiter);

      /* create new hmac channel */
      int8_t hmac_ok = channel_hmac_new(&channel, channel);
      if(hmac_ok != ERROR_OK){
        freeMemory(*handle);
        *handle = NULL;
        freeMemory(id);
        if(channel_free(channel) != ERROR_OK)
          return ERROR_UNKNOWN;
        return hmac_ok;
      }

      /* set hmac key */
      int64_t keysize = position - start_position - delimiter;
      int8_t hmac_key_ok = ERROR_OK;   
      if(keysize == 0)
        hmac_key_ok = channel_hmac_set_key(channel, NULL, 42);
      else
        hmac_key_ok = channel_hmac_set_key(channel, (unsigned char*)id + start_position,
                                           position - start_position - delimiter);

      if(hmac_key_ok != ERROR_OK){
        freeMemory(*handle);
        *handle = NULL;
        freeMemory(id);
        if(channel_free(channel) != ERROR_OK)
          return ERROR_UNKNOWN;
        return hmac_key_ok;
      }
    }else{
      freeMemory(*handle);
      *handle = NULL;
      freeMemory(id);
      if(channel_free(channel) != ERROR_OK)
        return ERROR_UNKNOWN;
      return ERROR_REGISTRY_UNKNOWN_IDENTIFIER;
    }
  }
  
  /* check delimiter */
  if(delimiter == 1){
    freeMemory(*handle);
    *handle = NULL;
    freeMemory(id);
    if(channel_free(channel) != ERROR_OK)
      return ERROR_UNKNOWN;
    return ERROR_REGISTRY_UNKNOWN_IDENTIFIER;
  }

  /* set endpoint */
  if(endpoint_existing){
    int8_t channel_set_end = channel_endpoint_connector_set_endpoint((*handle)->endpoint, channel);
    if(channel_set_end != ERROR_OK){
      freeMemory(*handle);
      *handle = NULL;
      freeMemory(id);
      if(channel_free(channel) != ERROR_OK)
        return ERROR_UNKNOWN;
    }
  }

  /* set channel */
  (*handle)->channel = channel;

  /* set domain */
  uint64_t size_domain = strlen(domain);
  if(requestMemory((void**)&((*handle)->domain), (size_domain + 1) * sizeof(char))){
    freeMemory(*handle);
    *handle = NULL;
    freeMemory(id);
    if(channel_free(channel) != ERROR_OK)
      return ERROR_UNKNOWN;
    return ERROR_MEMORY;
  }
  memcpy(((*handle)->domain), domain, size_domain);
  ((*handle)->domain)[size_domain] = '\0';

  freeMemory(id);
  return ERROR_OK;
}

/**
 * checks for delimiter | and sets a \0 on this position
 *
 * @param[out] id the string which shall be parsed
 * @param[out] position current position in identifier
 * @param[in] size length of id string
 */
void
checkDelimiterAndSetToTerminator(char **id, uint64_t *position, uint64_t size, 
                                 int8_t *delimiter)
{
  uint64_t i = *position;
  for(; i < size; i++){
    (*position)++; 

    if((*id)[i] == DELIMITER){
      (*id)[i] = '\0';
      *delimiter = 1;
      break;
    }
  }
  (*id)[size] = '\0';
}

/* -------------------------------------------------------------------------- */
int
registry_close(registry_t* handle)
{
  if(handle == NULL || handle->channel == NULL || handle->domain == NULL)
    return ERROR_INVALID_ARGUMENTS;

  /* clean up channel */
  if(channel_free(handle->channel) != ERROR_OK){
    freeMemory(handle->domain);
    freeMemory(handle);
    return ERROR_UNKNOWN;
  }

  /* clean up domain */
  freeMemory(handle->domain);

  /* clean up registry */
  freeMemory(handle);

  return ERROR_OK;
}

/* -------------------------------------------------------------------------- */
int
registry_get_int64(registry_t* handle, const char* key, int64_t* value)
{
  if(handle == NULL || handle->domain == NULL || strlen(handle->domain) == 0 || 
     handle->channel == NULL || key == NULL || strlen(key) == 0 || value == NULL)
    return ERROR_INVALID_ARGUMENTS;

  /* pack package */
  data_store_t ds;
  if(simple_memory_buffer_new(&ds, NULL, 0) != ERROR_OK ||
     data_store_write_byte(&ds, PACKET_GET_INT) != ERROR_OK ||
     bpack(&ds, "ss", handle->domain, key) != ERROR_OK){
    simple_memory_buffer_free(&ds);
    return ERROR_UNKNOWN;
  }

  /* send package */
  unsigned char *data = NULL;
  size_t size = 0;
  if(simple_memory_buffer_get_data(&ds, &data) != ERROR_OK ||
     simple_memory_buffer_get_size(&ds, &size) != ERROR_OK){
    simple_memory_buffer_free(&ds);
    return ERROR_UNKNOWN;
  }

  int retval = ERROR_OK;
  while((retval = channel_client_write_bytes(handle->channel, data, size)) == ERROR_CHANNEL_BUSY);

  if(retval != ERROR_OK){
    simple_memory_buffer_free(&ds);
    return ERROR_UNKNOWN;
  }

  if(simple_memory_buffer_free(&ds) != ERROR_OK)
    return ERROR_UNKNOWN;

  /* receive response */
  unsigned char *res_data = NULL;
  size_t res_size = 0;
  while((retval = channel_client_read_bytes(handle->channel, &res_data, &res_size)) == ERROR_CHANNEL_BUSY);

  if(retval != ERROR_OK)
    return ERROR_UNKNOWN;
  

  /* unpack package */
  unsigned char packettype = '\0';
  data_store_t res_ds;
  if(simple_memory_buffer_new(&res_ds, res_data, res_size) != ERROR_OK ||
     data_store_read_byte(&res_ds, &packettype) != ERROR_OK){
    simple_memory_buffer_free(&res_ds);
    return ERROR_UNKNOWN;
  }
  freeMemory(res_data);

  /* handling data */
  int ret = ERROR_OK;
  int64_t errorcode = ERROR_OK;
  switch(packettype){
    case PACKET_ERROR:
      if(bunpack(&res_ds, "l", &errorcode) != ERROR_OK)
        ret = ERROR_UNKNOWN;
      if(errorcode == ERROR_DATABASE_INVALID)
        ret = ERROR_REGISTRY_INVALID_STATE; 
      if(errorcode == ERROR_DATABASE_NO_SUCH_KEY)
        ret = ERROR_REGISTRY_NO_SUCH_KEY;
      else
        ret = ERROR_UNKNOWN; break;
    case PACKET_INT: 
      if(bunpack(&res_ds, "l", value) != ERROR_OK)
        ret = ERROR_UNKNOWN; break;

    default: ret = ERROR_UNKNOWN;
  }

  if(simple_memory_buffer_free(&res_ds) != ERROR_OK)
    ret = ERROR_UNKNOWN;
  return ret;
}

/* -------------------------------------------------------------------------- */
int
registry_set_int64(registry_t* handle, const char* key, int64_t value)
{
  if(handle == NULL || handle->domain == NULL || strlen(handle->domain) == 0 || 
     handle->channel == NULL || key == NULL || strlen(key) == 0)
    return ERROR_INVALID_ARGUMENTS;

  /* pack package */
  data_store_t ds;
  if(simple_memory_buffer_new(&ds, NULL, 0) != ERROR_OK ||
     data_store_write_byte(&ds, PACKET_SET_INT) != ERROR_OK ||
     bpack(&ds, "ssl", handle->domain, key, value) != ERROR_OK){
    simple_memory_buffer_free(&ds);
    return ERROR_UNKNOWN;
  }

 /* send package */
  unsigned char *data = NULL;
  size_t size = 0;
  if(simple_memory_buffer_get_data(&ds, &data) != ERROR_OK ||
     simple_memory_buffer_get_size(&ds, &size) != ERROR_OK){
    simple_memory_buffer_free(&ds);
    return ERROR_UNKNOWN;
  }

  int retval = ERROR_OK;
  while((retval = channel_client_write_bytes(handle->channel, data, size)) == ERROR_CHANNEL_BUSY);

  if(retval != ERROR_OK){
    simple_memory_buffer_free(&ds);
    return ERROR_UNKNOWN;
  }

  if(simple_memory_buffer_free(&ds) != ERROR_OK)
    return ERROR_UNKNOWN;

  /* receive response */
  unsigned char *res_data = NULL;
  size_t res_size = 0;
  while((retval = channel_client_read_bytes(handle->channel, &res_data, &res_size)) == ERROR_CHANNEL_BUSY);

  if(retval != ERROR_OK)
    return ERROR_UNKNOWN;

  /* unpack package */
  unsigned char packettype = '\0';
  data_store_t res_ds;
  if(simple_memory_buffer_new(&res_ds, res_data, res_size) != ERROR_OK ||
     data_store_read_byte(&res_ds, &packettype) != ERROR_OK){
    simple_memory_buffer_free(&res_ds);
    return ERROR_UNKNOWN;
  }
  freeMemory(res_data);

  /* handling data */
  int ret = ERROR_OK;
  int64_t errorcode = ERROR_OK;
  switch(packettype){
    case PACKET_OK: break;
    case PACKET_ERROR:
      if(bunpack(&res_ds, "l", &errorcode) != ERROR_OK)
        ret = ERROR_UNKNOWN;
      if(errorcode == ERROR_DATABASE_INVALID)
        ret =  ERROR_REGISTRY_INVALID_STATE;
      else 
        ret = ERROR_UNKNOWN; break;

    default: ret = ERROR_UNKNOWN;
  }

  if(simple_memory_buffer_free(&res_ds) != ERROR_OK)
    ret =  ERROR_UNKNOWN;
  return ret;
}

/* -------------------------------------------------------------------------- */
int
registry_get_double(registry_t* handle, const char* key, double* value)
{
  if(handle == NULL || handle->domain == NULL || strlen(handle->domain) == 0 || 
     handle->channel == NULL || key == NULL || strlen(key) == 0 || value == NULL)
    return ERROR_INVALID_ARGUMENTS;

  /* pack package */
  data_store_t ds;
  if(simple_memory_buffer_new(&ds, NULL, 0) != ERROR_OK ||
     data_store_write_byte(&ds, PACKET_GET_DOUBLE) != ERROR_OK ||
     bpack(&ds, "ss", handle->domain, key) != ERROR_OK){
    simple_memory_buffer_free(&ds);
    return ERROR_UNKNOWN;
  }

 /* send package */
  unsigned char *data = NULL;
  size_t size = 0;
  if(simple_memory_buffer_get_data(&ds, &data) != ERROR_OK ||
     simple_memory_buffer_get_size(&ds, &size) != ERROR_OK){
    simple_memory_buffer_free(&ds);
    return ERROR_UNKNOWN;
  }

  int retval = ERROR_OK;
  while((retval = channel_client_write_bytes(handle->channel, data, size)) == ERROR_CHANNEL_BUSY);

  if(retval != ERROR_OK){
    simple_memory_buffer_free(&ds);
    return ERROR_UNKNOWN;
  }

  if(simple_memory_buffer_free(&ds) != ERROR_OK)
    return ERROR_UNKNOWN;

  /* receive response */
  unsigned char *res_data = NULL;
  size_t res_size = 0;
  while((retval = channel_client_read_bytes(handle->channel, &res_data, &res_size)) == ERROR_CHANNEL_BUSY);

  if(retval != ERROR_OK)
    return ERROR_UNKNOWN;

  /* unpack package */
  unsigned char packettype = '\0';
  data_store_t res_ds;
  if(simple_memory_buffer_new(&res_ds, res_data, res_size) != ERROR_OK ||
     data_store_read_byte(&res_ds, &packettype) != ERROR_OK){
    simple_memory_buffer_free(&res_ds);
    return ERROR_UNKNOWN;
  }
  freeMemory(res_data);

  /* handling data */
  int ret = ERROR_OK;
  int64_t errorcode = ERROR_OK;
  switch(packettype){
    case PACKET_ERROR:
      if(bunpack(&res_ds, "l", &errorcode) != ERROR_OK)
        ret = ERROR_UNKNOWN;
      if(errorcode == ERROR_DATABASE_INVALID)
        ret = ERROR_REGISTRY_INVALID_STATE; 
      if(errorcode == ERROR_DATABASE_NO_SUCH_KEY)
        ret = ERROR_REGISTRY_NO_SUCH_KEY;
      else
        ret = ERROR_UNKNOWN; break;
    case PACKET_DOUBLE: 
      if(bunpack(&res_ds, "d", value) != ERROR_OK)
        ret = ERROR_UNKNOWN; break;
    default: ret = ERROR_UNKNOWN;
  }

  if(simple_memory_buffer_free(&res_ds) != ERROR_OK)
    ret = ERROR_UNKNOWN;
  return ret;
}

/* -------------------------------------------------------------------------- */
int
registry_set_double(registry_t* handle, const char* key, double value)
{
  if(handle == NULL || handle->domain == NULL || strlen(handle->domain) == 0 || 
     handle->channel == NULL || key == NULL || strlen(key) == 0)
    return ERROR_INVALID_ARGUMENTS;

  /* pack package */
  data_store_t ds;
  if(simple_memory_buffer_new(&ds, NULL, 0) != ERROR_OK ||
     data_store_write_byte(&ds, PACKET_SET_DOUBLE) != ERROR_OK ||
     bpack(&ds, "ssd", handle->domain, key, value) != ERROR_OK){
    simple_memory_buffer_free(&ds);
    return ERROR_UNKNOWN;
  }

 /* send package */
  unsigned char *data = NULL;
  size_t size = 0;
  if(simple_memory_buffer_get_data(&ds, &data) != ERROR_OK ||
     simple_memory_buffer_get_size(&ds, &size) != ERROR_OK){
    simple_memory_buffer_free(&ds);
    return ERROR_UNKNOWN;
  }

  int retval = ERROR_OK;
  while((retval = channel_client_write_bytes(handle->channel, data, size)) == ERROR_CHANNEL_BUSY);

  if(retval != ERROR_OK){
    simple_memory_buffer_free(&ds);
    return ERROR_UNKNOWN;
  }

  if(simple_memory_buffer_free(&ds) != ERROR_OK)
    return ERROR_UNKNOWN;

  /* receive response */
  unsigned char *res_data = NULL;
  size_t res_size = 0;
  while((retval = channel_client_read_bytes(handle->channel, &res_data, &res_size)) == ERROR_CHANNEL_BUSY);

  if(retval != ERROR_OK)
    return ERROR_UNKNOWN;

  /* unpack package */
  unsigned char packettype = '\0';
  data_store_t res_ds;
  if(simple_memory_buffer_new(&res_ds, res_data, res_size) != ERROR_OK ||
     data_store_read_byte(&res_ds, &packettype) != ERROR_OK){
    simple_memory_buffer_free(&res_ds);
    return ERROR_UNKNOWN;
  }
  freeMemory(res_data);

  /* handling data */
  int ret = ERROR_OK;
  int64_t errorcode = ERROR_OK;
  switch(packettype){
    case PACKET_OK: break;
    case PACKET_ERROR:
      if(bunpack(&res_ds, "l", &errorcode) != ERROR_OK)
        ret = ERROR_UNKNOWN;
      if(errorcode == ERROR_DATABASE_INVALID)
        ret = ERROR_REGISTRY_INVALID_STATE;
      else 
        ret = ERROR_UNKNOWN; break;

    default: ret = ERROR_UNKNOWN;
  }

  if(simple_memory_buffer_free(&res_ds) != ERROR_OK)
    ret = ERROR_UNKNOWN;
  return ret;
}

/* -------------------------------------------------------------------------- */
int
registry_get_string(registry_t* handle, const char* key, char** value)
{
  if(handle == NULL || handle->domain == NULL || strlen(handle->domain) == 0 || 
     handle->channel == NULL || key == NULL || strlen(key) == 0 || value == NULL)
    return ERROR_INVALID_ARGUMENTS;

  /* pack package */
  data_store_t ds;
  if(simple_memory_buffer_new(&ds, NULL, 0) != ERROR_OK ||
     data_store_write_byte(&ds, PACKET_GET_STRING) != ERROR_OK ||
     bpack(&ds, "ss", handle->domain, key) != ERROR_OK){
    simple_memory_buffer_free(&ds);
    return ERROR_UNKNOWN;
  }

 /* send package */
  unsigned char *data = NULL;
  size_t size = 0;
  if(simple_memory_buffer_get_data(&ds, &data) != ERROR_OK ||
     simple_memory_buffer_get_size(&ds, &size) != ERROR_OK){
    simple_memory_buffer_free(&ds);
    return ERROR_UNKNOWN;
  }

  int retval = ERROR_OK;
  while((retval = channel_client_write_bytes(handle->channel, data, size)) == ERROR_CHANNEL_BUSY);

  if(retval != ERROR_OK){
    simple_memory_buffer_free(&ds);
    return ERROR_UNKNOWN;
  }

  if(simple_memory_buffer_free(&ds) != ERROR_OK)
    return ERROR_UNKNOWN;

  /* receive response */
  unsigned char *res_data = NULL;
  size_t res_size = 0;
  while((retval = channel_client_read_bytes(handle->channel, &res_data, &res_size)) == ERROR_CHANNEL_BUSY);

  if(retval != ERROR_OK)
    return ERROR_UNKNOWN;

  /* unpack package */
  unsigned char packettype = '\0';
  data_store_t res_ds;
  if(simple_memory_buffer_new(&res_ds, res_data, res_size) != ERROR_OK ||
     data_store_read_byte(&res_ds, &packettype) != ERROR_OK){
    simple_memory_buffer_free(&res_ds);
    return ERROR_UNKNOWN;
  }
  freeMemory(res_data);

  /* handling data */
  int ret = ERROR_OK;
  int64_t errorcode = ERROR_OK;
  switch(packettype){
    case PACKET_ERROR:
      if(bunpack(&res_ds, "l", &errorcode) != ERROR_OK)
        ret = ERROR_UNKNOWN;
      if(errorcode == ERROR_DATABASE_INVALID)
        ret = ERROR_REGISTRY_INVALID_STATE; 
      if(errorcode == ERROR_DATABASE_NO_SUCH_KEY)
        ret = ERROR_REGISTRY_NO_SUCH_KEY;
      else
        ret = ERROR_UNKNOWN; break;
    case PACKET_STRING: 
      if(bunpack(&res_ds, "s", value) != ERROR_OK)
        ret = ERROR_UNKNOWN; break;
    default: ret = ERROR_UNKNOWN;
  }

  if(simple_memory_buffer_free(&res_ds) != ERROR_OK)
    ret = ERROR_UNKNOWN;
  return ret;
}

/* -------------------------------------------------------------------------- */
int
registry_set_string(registry_t* handle, const char* key, const char* value)
{
  if(handle == NULL || handle->domain == NULL || strlen(handle->domain) == 0 || 
     handle->channel == NULL || key == NULL || strlen(key) == 0 || value == NULL)
    return ERROR_INVALID_ARGUMENTS;

  /* pack package */
  data_store_t ds;
  if(simple_memory_buffer_new(&ds, NULL, 0) != ERROR_OK ||
     data_store_write_byte(&ds, PACKET_SET_STRING) != ERROR_OK ||
     bpack(&ds, "sss", handle->domain, key, value) != ERROR_OK){
    simple_memory_buffer_free(&ds);
    return ERROR_UNKNOWN;
  }

 /* send package */
  unsigned char *data = NULL;
  size_t size = 0;
  if(simple_memory_buffer_get_data(&ds, &data) != ERROR_OK ||
     simple_memory_buffer_get_size(&ds, &size) != ERROR_OK){
    simple_memory_buffer_free(&ds);
    return ERROR_UNKNOWN;
  }

  int retval = ERROR_OK;
  while((retval = channel_client_write_bytes(handle->channel, data, size)) == ERROR_CHANNEL_BUSY);

  if(retval != ERROR_OK){
    simple_memory_buffer_free(&ds);
    return ERROR_UNKNOWN;
  }

  if(simple_memory_buffer_free(&ds) != ERROR_OK)
    return ERROR_UNKNOWN;

  /* receive response */
  unsigned char *res_data = NULL;
  size_t res_size = 0;
  while((retval = channel_client_read_bytes(handle->channel, &res_data, &res_size)) == ERROR_CHANNEL_BUSY);

  if(retval != ERROR_OK)
    return ERROR_UNKNOWN;

  /* unpack package */
  unsigned char packettype = '\0';
  data_store_t res_ds;
  if(simple_memory_buffer_new(&res_ds, res_data, res_size) != ERROR_OK ||
     data_store_read_byte(&res_ds, &packettype) != ERROR_OK){
    simple_memory_buffer_free(&res_ds);
    return ERROR_UNKNOWN;
  }
  freeMemory(res_data);

  /* handling data */
  int ret = ERROR_OK;
  int64_t errorcode = ERROR_OK;
  switch(packettype){
    case PACKET_OK: break;
    case PACKET_ERROR:
      if(bunpack(&res_ds, "l", &errorcode) != ERROR_OK)
        ret = ERROR_UNKNOWN;
      if(errorcode == ERROR_DATABASE_INVALID)
        ret = ERROR_REGISTRY_INVALID_STATE;
      else 
        ret = ERROR_UNKNOWN; break;

    default: ret = ERROR_UNKNOWN;
  }

  if(simple_memory_buffer_free(&res_ds) != ERROR_OK)
    ret = ERROR_UNKNOWN;
  return ret;
}

/* -------------------------------------------------------------------------- */
int
registry_get_blob(registry_t* handle, const char* key, unsigned char** value, 
                  size_t* size)
{
  if(handle == NULL || handle->domain == NULL || strlen(handle->domain) == 0 || 
     handle->channel == NULL || key == NULL || strlen(key) == 0 || value == NULL
     || size == NULL)
    return ERROR_INVALID_ARGUMENTS;

  /* pack package */
  data_store_t ds;
  if(simple_memory_buffer_new(&ds, NULL, 0) != ERROR_OK ||
     data_store_write_byte(&ds, PACKET_GET_BLOB) != ERROR_OK ||
     bpack(&ds, "ss", handle->domain, key) != ERROR_OK){
    simple_memory_buffer_free(&ds);
    return ERROR_UNKNOWN;
  }

 /* send package */
  unsigned char *data = NULL;
  size_t blob_size = 0;
  if(simple_memory_buffer_get_data(&ds, &data) != ERROR_OK ||
     simple_memory_buffer_get_size(&ds, &blob_size) != ERROR_OK){
    simple_memory_buffer_free(&ds);
    return ERROR_UNKNOWN;
  }

  int retval = ERROR_OK;
  while((retval = channel_client_write_bytes(handle->channel, data, blob_size)) == ERROR_CHANNEL_BUSY);

  if(retval != ERROR_OK){
    simple_memory_buffer_free(&ds);
    return ERROR_UNKNOWN;
  }

  if(simple_memory_buffer_free(&ds) != ERROR_OK)
    return ERROR_UNKNOWN;

  /* receive response */
  unsigned char *res_data = NULL;
  size_t res_size = 0;
  while((retval = channel_client_read_bytes(handle->channel, &res_data, &res_size)) == ERROR_CHANNEL_BUSY);

  if(retval != ERROR_OK)
    return ERROR_UNKNOWN;

  /* unpack package */
  unsigned char packettype = '\0';
  data_store_t res_ds;
  if(simple_memory_buffer_new(&res_ds, res_data, res_size) != ERROR_OK ||
     data_store_read_byte(&res_ds, &packettype) != ERROR_OK){
    simple_memory_buffer_free(&res_ds);
    return ERROR_UNKNOWN;
  }
  freeMemory(res_data);

  /* handling data */
  int ret = ERROR_OK;
  int64_t errorcode = ERROR_OK;
  switch(packettype){
    case PACKET_ERROR:
      if(bunpack(&res_ds, "l", &errorcode) != ERROR_OK)
        ret =  ERROR_UNKNOWN;
      if(errorcode == ERROR_DATABASE_INVALID)
        ret = ERROR_REGISTRY_INVALID_STATE; 
      if(errorcode == ERROR_DATABASE_NO_SUCH_KEY)
        ret = ERROR_REGISTRY_NO_SUCH_KEY;
      else
        ret = ERROR_UNKNOWN; break;
    case PACKET_BLOB: 
      if(bunpack(&res_ds, "b", size, value) != ERROR_OK)
        ret = ERROR_UNKNOWN; break;
    default: ret = ERROR_UNKNOWN;
  }

  if(simple_memory_buffer_free(&res_ds) != ERROR_OK)
    ret = ERROR_UNKNOWN;
  return ret;
}

/* -------------------------------------------------------------------------- */
int
registry_set_blob(registry_t* handle, const char* key, 
                  const unsigned char* value, size_t size)
{
  if(handle == NULL || handle->domain == NULL || strlen(handle->domain) == 0 || 
     handle->channel == NULL || key == NULL || strlen(key) == 0 || value == NULL)
    return ERROR_INVALID_ARGUMENTS;

  /* pack package */
  data_store_t ds;
  if(simple_memory_buffer_new(&ds, NULL, 0) != ERROR_OK ||
     data_store_write_byte(&ds, PACKET_SET_BLOB) != ERROR_OK ||
     bpack(&ds, "ssb", handle->domain, key, size, value) != ERROR_OK){
    simple_memory_buffer_free(&ds);
    return ERROR_UNKNOWN;
  }

 /* send package */
  unsigned char *data = NULL;
  size_t blob_size = 0;
  if(simple_memory_buffer_get_data(&ds, &data) != ERROR_OK ||
     simple_memory_buffer_get_size(&ds, &blob_size) != ERROR_OK){
    simple_memory_buffer_free(&ds);
    return ERROR_UNKNOWN;
  }

  int retval = ERROR_OK;
  while((retval = channel_client_write_bytes(handle->channel, data, blob_size)) == ERROR_CHANNEL_BUSY);

  if(retval != ERROR_OK){
    simple_memory_buffer_free(&ds);
    return ERROR_UNKNOWN;
  }

  if(simple_memory_buffer_free(&ds) != ERROR_OK)
    return ERROR_UNKNOWN;

  /* receive response */
  unsigned char *res_data = NULL;
  size_t res_size = 0;
  while((retval = channel_client_read_bytes(handle->channel, &res_data, &res_size)) == ERROR_CHANNEL_BUSY);

  if(retval != ERROR_OK)
    return ERROR_UNKNOWN;

  /* unpack package */
  unsigned char packettype = '\0';
  data_store_t res_ds;
  if(simple_memory_buffer_new(&res_ds, res_data, res_size) != ERROR_OK ||
     data_store_read_byte(&res_ds, &packettype) != ERROR_OK){
    simple_memory_buffer_free(&res_ds);
    return ERROR_UNKNOWN;
  }
  freeMemory(res_data);

  /* handling data */
  int ret = ERROR_OK;
  int64_t errorcode = ERROR_OK;
  switch(packettype){
    case PACKET_OK: break;
    case PACKET_ERROR:
      if(bunpack(&res_ds, "l", &errorcode) != ERROR_OK)
        ret = ERROR_UNKNOWN;
      if(errorcode == ERROR_DATABASE_INVALID)
        ret = ERROR_REGISTRY_INVALID_STATE;
      else 
        ret = ERROR_UNKNOWN; break;

    default: ret = ERROR_UNKNOWN;
  }

  if(simple_memory_buffer_free(&res_ds) != ERROR_OK)
    ret = ERROR_UNKNOWN;
  return ret;
}

/* -------------------------------------------------------------------------- */
int
registry_enum_keys(registry_t* handle, const char* pattern, size_t* count, 
                   size_t* size, char** keys)
{
 if(handle == NULL || handle->domain == NULL || strlen(handle->domain) == 0 || 
    handle->channel == NULL || pattern == NULL || count == NULL || size == NULL
    || keys == NULL)
    return ERROR_INVALID_ARGUMENTS;

  /* pack package */
  data_store_t ds;
  if(simple_memory_buffer_new(&ds, NULL, 0) != ERROR_OK ||
     data_store_write_byte(&ds, PACKET_GET_ENUM) != ERROR_OK ||
     bpack(&ds, "ss", handle->domain, pattern) != ERROR_OK){
    simple_memory_buffer_free(&ds);
    return ERROR_UNKNOWN;
  }

 /* send package */
  unsigned char *data = NULL;
  size_t enum_size = 0;
  if(simple_memory_buffer_get_data(&ds, &data) != ERROR_OK ||
     simple_memory_buffer_get_size(&ds, &enum_size) != ERROR_OK){
    simple_memory_buffer_free(&ds);
    return ERROR_UNKNOWN;
  }

  int retval = ERROR_OK;
  while((retval = channel_client_write_bytes(handle->channel, data, enum_size)) == ERROR_CHANNEL_BUSY);

  if(retval != ERROR_OK){
    simple_memory_buffer_free(&ds);
    return ERROR_UNKNOWN;
  }

  if(simple_memory_buffer_free(&ds) != ERROR_OK)
    return ERROR_UNKNOWN;

  /* receive response */
  unsigned char *res_data = NULL;
  size_t res_size = 0;
  while((retval = channel_client_read_bytes(handle->channel, &res_data, &res_size)) == ERROR_CHANNEL_BUSY);

  if(retval != ERROR_OK)
    return ERROR_UNKNOWN;

  /* unpack package */
  unsigned char packettype = '\0';
  data_store_t res_ds;
  if(simple_memory_buffer_new(&res_ds, res_data, res_size) != ERROR_OK ||
     data_store_read_byte(&res_ds, &packettype) != ERROR_OK){
    simple_memory_buffer_free(&res_ds);
    return ERROR_UNKNOWN; 
  }
  freeMemory(res_data);

  /* handling data */
  int ret = ERROR_OK;
  int64_t errorcode = ERROR_OK;
  int64_t count_enum = 0;
  switch(packettype){
    case PACKET_ERROR:
      if(bunpack(&res_ds, "l", &errorcode) != ERROR_OK)
        ret = ERROR_UNKNOWN;
      if(errorcode == ERROR_DATABASE_INVALID)
        ret = ERROR_REGISTRY_INVALID_STATE; 
      if(errorcode == ERROR_DATABASE_NO_SUCH_KEY)
        ret = ERROR_REGISTRY_NO_SUCH_KEY;
      else
        ret = ERROR_UNKNOWN; break;
    case PACKET_ENUM:
      if(bunpack(&res_ds, "l", &count_enum) != ERROR_OK)
        ret = ERROR_UNKNOWN; 
      *count = count_enum;
      if(count_enum > 0){
        if(bunpack(&res_ds, "b", size, keys) != ERROR_OK)
          ret = ERROR_UNKNOWN; 
      }else{
        *size = 0;
        *keys = NULL;
      } break;
    default: ret = ERROR_UNKNOWN;
  } 

  if(simple_memory_buffer_free(&res_ds) != ERROR_OK)
    ret = ERROR_UNKNOWN;
  return ret;
}

/* -------------------------------------------------------------------------- */
int
registry_key_get_value_type(registry_t* handle, const char* key, int* type)
{
  if(handle == NULL || handle->domain == NULL || strlen(handle->domain) == 0 || 
     handle->channel == NULL || key == NULL || strlen(key) == 0 || type == NULL)
    return ERROR_INVALID_ARGUMENTS;

  /* pack package */
  data_store_t ds;
  if(simple_memory_buffer_new(&ds, NULL, 0) != ERROR_OK ||
     data_store_write_byte(&ds, PACKET_GET_VALUE_TYPE) != ERROR_OK ||
     bpack(&ds, "ss", handle->domain, key) != ERROR_OK){
    simple_memory_buffer_free(&ds);
    return ERROR_UNKNOWN;
  }

 /* send package */
  unsigned char *data = NULL;
  size_t size = 0;
  if(simple_memory_buffer_get_data(&ds, &data) != ERROR_OK ||
     simple_memory_buffer_get_size(&ds, &size) != ERROR_OK){
    simple_memory_buffer_free(&ds);
    return ERROR_UNKNOWN;
  }

  int retval = ERROR_OK;
  while((retval = channel_client_write_bytes(handle->channel, data, size)) == ERROR_CHANNEL_BUSY);

  if(retval != ERROR_OK){
    simple_memory_buffer_free(&ds);
    return ERROR_UNKNOWN;
  }

  if(simple_memory_buffer_free(&ds) != ERROR_OK)
    return ERROR_UNKNOWN;

  /* receive response */
  unsigned char *res_data = NULL;
  size_t res_size = 0;
  while((retval = channel_client_read_bytes(handle->channel, &res_data, &res_size)) == ERROR_CHANNEL_BUSY);

  if(retval != ERROR_OK)
    return ERROR_UNKNOWN;

  /* unpack package */
  unsigned char packettype = '\0';
  data_store_t res_ds;
  if(simple_memory_buffer_new(&res_ds, res_data, res_size) != ERROR_OK ||
     data_store_read_byte(&res_ds, &packettype) != ERROR_OK){
    simple_memory_buffer_free(&res_ds);
    return ERROR_UNKNOWN;
  }
  freeMemory(res_data);

  /* handling data */
  int ret = ERROR_OK;
  int64_t errorcode = ERROR_OK;
  int64_t valuetype = 0;
  switch(packettype){
    case PACKET_ERROR:
      if(bunpack(&res_ds, "l", &errorcode) != ERROR_OK)
        ret = ERROR_UNKNOWN;
      if(errorcode == ERROR_DATABASE_INVALID)
        ret = ERROR_REGISTRY_INVALID_STATE; 
      if(errorcode == ERROR_DATABASE_NO_SUCH_KEY)
        ret = ERROR_REGISTRY_NO_SUCH_KEY;
      else
        ret = ERROR_UNKNOWN; break;
    case PACKET_TYPE: 
      if(bunpack(&res_ds, "l", &valuetype) != ERROR_OK)
        ret = ERROR_UNKNOWN; 
      *type = valuetype; break;
    default: ret = ERROR_UNKNOWN;
  }

  if(simple_memory_buffer_free(&res_ds) != ERROR_OK)
    ret = ERROR_UNKNOWN;

  return ret;
}

/* -------------------------------------------------------------------------- */
channel_t*
registry_get_channel(registry_t* handle)
{
  if(handle == NULL || handle->channel == NULL)
    return NULL;

  /* return channel */
  return handle->channel;
}

