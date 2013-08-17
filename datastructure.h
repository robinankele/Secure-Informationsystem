#ifndef DATASTRUCTURE_H
#define DATASTRUCTURE_H

/** @brief Datatstructure.
 *
 * This file contains the definition of the 'registry structures'
 *
 *  @file  datastructure.c
 */

#include "communication/channel.h"
#include "server/database.h"
#include <sqlite3.h>

struct registry_s {
  channel_t *channel;                     /* channel of registry */
  channel_t *endpoint;                    /* endpoint of channel */
  char *domain;                           /* domain of registry  */
};

typedef struct channel_hmac_s {
  unsigned char *key;                     /* key of hmac channel */ 
  size_t keysize;                         /* size of key         */
  channel_t *child;                       /* child channel       */
} channel_hmac_t;

struct database_handle_s {
  sqlite3* db;
  char* blobpath;
};

struct server_s {
  database_handle_t *db;                  /* database of server */
};

#endif /* DATASTRUCTURE_H */

