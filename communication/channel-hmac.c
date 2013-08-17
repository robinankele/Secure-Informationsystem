/** @brief Message channel with HMAC.
 *
 * This file contains the implementation of the secure communication channel 
 * between client and server.
 *
 * @file channel-hmac.c
 */

#include <string.h>
#include <stdio.h>
#include "channel-hmac.h"
#include "../errors.h"
#include "../memory.h"
#include "../datastructure.h"
#include "../server/server.h"
#include "crypto/hmac.h"
#include "crypto/sha1.h"

/* Prototyping */
/* -------------------------------------------------------------------------- */
int channel_hmac_set_key(channel_t* channel, const unsigned char* key, size_t len);
static int ch_client_read_bytes(channel_t* channel, unsigned char** bytes, size_t* size);
static int ch_client_write_bytes(channel_t* channel, const unsigned char* bytes, size_t size);
static int ch_server_read_bytes(channel_t* channel, unsigned char** bytes, size_t* size);
static int ch_server_write_bytes(channel_t* channel, const unsigned char* bytes, size_t size);
static int ch_free(channel_t* channel);

/* Implementation */
/* -------------------------------------------------------------------------- */
int
channel_hmac_new(channel_t** channel, channel_t* child)
{
  if(channel == NULL || child == NULL)
    return ERROR_INVALID_ARGUMENTS;

  channel_t *channel_new = NULL;
  if(requestMemory((void**)&channel_new, sizeof(channel_t)) != ERROR_OK)
    return ERROR_MEMORY;
  
  /* set channel hmac functions */ 
  channel_new->client_read_bytes  = ch_client_read_bytes;
  channel_new->client_write_bytes = ch_client_write_bytes;
  channel_new->server_read_bytes  = ch_server_read_bytes;
  channel_new->server_write_bytes = ch_server_write_bytes;
  channel_new->free               = ch_free;

  channel_hmac_t *channel_hmac = NULL;
  if(requestMemory((void**)&channel_hmac, sizeof(channel_hmac_t)) != ERROR_OK)
    return ERROR_MEMORY;

  /* set channel_hmac */ 
  channel_new->data = channel_hmac;

  /* set child channel */
  channel_hmac->child = child;
  
  /* initiate values */
  channel_hmac->key = NULL;
  channel_hmac->keysize = 0;

  /* set channel */
  (*channel) = channel_new; 
  
  return ERROR_OK;
}

/* -------------------------------------------------------------------------- */
int
channel_hmac_set_key(channel_t* channel, const unsigned char* key, size_t len)
{
  if(channel == NULL || channel->data == NULL || len == 0)
    return ERROR_INVALID_ARGUMENTS;

  channel_hmac_t *channel_hmac = channel->data;
  if(key == NULL || strlen((char*)key) == 0){
    channel_hmac->key = NULL;
    channel_hmac->keysize = 0;
    return ERROR_OK;
  }

  /* set key */
  if(requestMemory((void**)&(channel_hmac->key), len * sizeof(unsigned char)))
    return ERROR_MEMORY;
  memcpy((channel_hmac->key), key, len);

  /* set keysize */
  channel_hmac->keysize = len;

  return ERROR_OK;
}

/* -------------------------------------------------------------------------- */
static int
ch_client_read_bytes(channel_t* channel, unsigned char** bytes, size_t* size)
{
  if(channel == NULL || channel->data == NULL || bytes == NULL || size == NULL)
    return ERROR_INVALID_ARGUMENTS;

  channel_hmac_t* ch = channel->data;
  if(ch->child == NULL)
    return ERROR_INVALID_ARGUMENTS;

  /* getting traffic from child channel */
  unsigned char *message = NULL;
  size_t message_size = 0;
  int ret = channel_client_read_bytes(ch->child, &message, &message_size);
  if(ret != ERROR_OK)
    return ret;

  /* read some wrong values */ 
  if(message == NULL || strlen((char*)message) == 0 || message_size == 0){
    freeMemory(message);
    return ERROR_UNKNOWN;
  }

  /* validate hmac */
  unsigned char digest[SHA1_BLOCKSIZE] = {'\0'};
  size_t digest_size = 0;
  if(ch->key != NULL && ch->keysize != 0){
    digest_size = SHA1_BLOCKSIZE;
    memcpy(digest, ((char*)message + message_size - digest_size), digest_size);

    /* cut hmac from message */
    if(editMemory((void**)&message, message_size - digest_size) != ERROR_OK){
      freeMemory(message);
      return ERROR_MEMORY;
    }

    if(hmac_verify(ch->key, ch->keysize, message, message_size - digest_size, digest) != ERROR_OK){
      freeMemory(message);
      return ERROR_UNKNOWN;
    }
  }

  /* write data to user */
  if(requestMemory((void**)bytes, (message_size - digest_size) + 1) != ERROR_OK){
    freeMemory(message);
    return ERROR_MEMORY;
  }

  memcpy(*bytes, message, message_size - digest_size);
  *size = message_size - digest_size;
  (*bytes)[*size] = '\0';

  freeMemory(message);

  return ERROR_OK;
}

/* -------------------------------------------------------------------------- */
static int
ch_server_read_bytes(channel_t* channel, unsigned char** bytes, size_t* size) 
{
   if(channel == NULL || channel->data == NULL || bytes == NULL || size == NULL)
    return ERROR_INVALID_ARGUMENTS;

  channel_hmac_t* ch = channel->data;
  if(ch->child == NULL)
    return ERROR_INVALID_ARGUMENTS;

  /* getting traffic from child channel */
  unsigned char *message = NULL;
  size_t message_size = 0;
  int ret = channel_server_read_bytes(ch->child, &message, &message_size);
  if(ret != ERROR_OK)
    return ret;

  /* read some wrong values */ 
  if(message == NULL || strlen((char*)message) == 0 || message_size == 0){
    freeMemory(message);
    return ERROR_UNKNOWN;
  }

  /* validate hmac */
  unsigned char digest[SHA1_BLOCKSIZE] = {'\0'};
  size_t digest_size = 0;
  if(ch->key != NULL && ch->keysize != 0){
    digest_size = SHA1_BLOCKSIZE;
    memcpy(digest, ((char*)message + message_size - digest_size), digest_size);

    /* cut hmac from message */
    if(editMemory((void**)&message, message_size - digest_size) != ERROR_OK){
      freeMemory(message);
      return ERROR_MEMORY;
    }

    if(hmac_verify(ch->key, ch->keysize, message, message_size - digest_size, digest) != ERROR_OK){
      freeMemory(message);
      return ERROR_UNKNOWN;
    }
  }

  /* write data to user */
  if(requestMemory((void**)bytes, message_size - digest_size + 1) != ERROR_OK){
    freeMemory(message);
    return ERROR_MEMORY;
  }

  memcpy(*bytes, message, message_size - digest_size);
  *size = message_size - digest_size;
  (*bytes)[*size] = '\0';

  freeMemory(message);

  return ERROR_OK;
}

/* -------------------------------------------------------------------------- */
static int
ch_client_write_bytes(channel_t* channel, const unsigned char* bytes, size_t size)
{
  if(channel == NULL || channel->data == NULL || bytes == NULL || strlen((char*)bytes) == 0 
     || size == 0)
    return ERROR_INVALID_ARGUMENTS;

  channel_hmac_t* ch = channel->data;
  if(ch->child == NULL)
    return ERROR_INVALID_ARGUMENTS;

  unsigned char *message = NULL;
  if(requestMemory((void**)&message, size + 1) != ERROR_OK)
    return ERROR_MEMORY;
  memcpy(message, bytes, size);
  message[size] = '\0';
  size_t message_size = size;

  /* calculate hmac */
  unsigned char digest[SHA1_BLOCKSIZE] = {'\0'};
  if(ch->key != NULL && ch->keysize != 0){
    if(hmac(ch->key, ch->keysize, message, message_size, digest) != ERROR_OK){
      freeMemory(message);
      return ERROR_UNKNOWN;
    }

    /* write data + digest*/
    if(editMemory((void**)&message, message_size + SHA1_BLOCKSIZE + 1) != ERROR_OK){
      freeMemory(message);
      return ERROR_MEMORY;
    }

    memcpy(message, bytes, message_size);
    memcpy(message + message_size, digest, SHA1_BLOCKSIZE);
    message_size = message_size + SHA1_BLOCKSIZE;
    message[message_size] = '\0';
  }

  /* forwarding traffic to child channel */
  int ret = channel_client_write_bytes(ch->child, message, message_size);
  if(ret != ERROR_OK){
    freeMemory(message);
    return ERROR_UNKNOWN;
  }
  freeMemory(message);
  
  return ret;
}

/* -------------------------------------------------------------------------- */
static int
ch_server_write_bytes(channel_t* channel, const unsigned char* bytes, size_t size)
{
    if(channel == NULL || channel->data == NULL || bytes == NULL || strlen((char*)bytes) == 0 
     || size == 0)
    return ERROR_INVALID_ARGUMENTS;

  channel_hmac_t* ch = channel->data;
  if(ch->child == NULL)
    return ERROR_INVALID_ARGUMENTS;

  unsigned char *message = NULL;
  if(requestMemory((void**)&message, size + 1) != ERROR_OK)
    return ERROR_MEMORY;
  memcpy(message, bytes, size);
  message[size] = '\0';
  size_t message_size = size;

  /* calculate hmac */
  unsigned char digest[SHA1_BLOCKSIZE] = {'\0'};
  if(ch->key != NULL && ch->keysize != 0){
    if(hmac(ch->key, ch->keysize, message, message_size, digest) != ERROR_OK){
      freeMemory(message);
      return ERROR_UNKNOWN;
    }

    /* write data + digest*/
    if(editMemory((void**)&message, message_size + SHA1_BLOCKSIZE + 1) != ERROR_OK){
      freeMemory(message);
      return ERROR_MEMORY;
    }

    memcpy(message, bytes, message_size);
    memcpy(message + message_size, digest, SHA1_BLOCKSIZE);
    message_size = message_size + SHA1_BLOCKSIZE;
    message[message_size] = '\0';
  }

  /* forwarding traffic to child channel */
  int ret = channel_server_write_bytes(ch->child, message, message_size);
  if(ret != ERROR_OK){
    freeMemory(message);
    return ERROR_UNKNOWN;
  }
  freeMemory(message);
  
  return ret;
}

/* -------------------------------------------------------------------------- */
static int
ch_free(channel_t* channel)
{
  if(channel == NULL || channel->data == NULL)
    return ERROR_INVALID_ARGUMENTS;

  channel_hmac_t* ch = channel->data;
  if(ch->child == NULL){
    freeMemory(ch);
    freeMemory(channel);
    return ERROR_INVALID_ARGUMENTS;
  }

  if(ch->key != NULL)
    freeMemory(ch->key);

  /* free child */
  if(channel_free(ch->child) != ERROR_OK){
    freeMemory(ch);
    freeMemory(channel);
    return ERROR_UNKNOWN;
  }

  /* free handles */
  freeMemory(ch);
  freeMemory(channel);

  return ERROR_OK;
}

