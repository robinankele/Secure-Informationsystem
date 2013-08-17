/** @brief Server interface
 *
 * This file contains the serverside implementation of 'the registry'.
 *
 * @file server.c
 */

#include <stdio.h>
#include <string.h>
#include "server.h"
#include "../errors.h"
#include "../memory.h"
#include "../datastructure.h"
#include "database.h"
#include "../communication/channel.h"
#include "../communication/simple-memory-buffer.h"
#include "../communication/datastore.h"
#include "../communication/bpack.h"


/* Prototyping */
/* -------------------------------------------------------------------------- */
int sendPacket(data_store_t *ds, size_t *response_size, unsigned char **response);

/* Implementation */
/* -------------------------------------------------------------------------- */
int
server_init(server_t** server, const char* database)
{
  if(server == NULL || database == NULL)
    return ERROR_INVALID_ARGUMENTS;

  /* request memory for server */
  if(requestMemory((void**)server, sizeof(server_t)) != ERROR_OK)
    return ERROR_MEMORY;
  (*server)->db = NULL;

  /* open database connection */
  database_handle_t *db = NULL;
  int retval = database_open(&db, database);
  if(retval != ERROR_OK){
    //freeMemory(*server);
    //freeMemory(db);
    return retval;
  }
  (*server)->db = db;

  return ERROR_OK;
}

/* -------------------------------------------------------------------------- */
int
server_process(server_t* server, const unsigned char* data, size_t size, 
               unsigned char** response, size_t* response_size)
{
  if(server == NULL || server->db == NULL || data == NULL || strlen((char*)data) == 0
     || size == 0 || response == NULL || response_size == NULL)
    return ERROR_INVALID_ARGUMENTS;

  /* unpack package */
  unsigned char packettype = '\0';
  unsigned char *domain = NULL;
  unsigned char* key = NULL;
  data_store_t ds;
  if(simple_memory_buffer_new(&ds, data, size) != ERROR_OK )
    return ERROR_UNKNOWN;
  
  int64_t ret = ERROR_OK;
  if((ret = data_store_read_byte(&ds, &packettype)) != ERROR_OK || 
     (ret = bunpack(&ds, "ss", &domain, &key)) != ERROR_OK){
    if(domain != NULL)
      freeMemory(domain);
    if(key != NULL)
      freeMemory(key);
  }

  /* handle incomming data */
  int64_t integer = 0;
  double dob = 0.0;
  char* string = NULL;
  unsigned char* blob = NULL;
  size_t bsize = 0;
  database_value_type_t type;
  size_t count = 0;
  int64_t count_enum = 0;
  size_t esize = 0;
  char* keys = NULL; 

  data_store_t response_ds;
  if(simple_memory_buffer_new(&response_ds, NULL, 0) != ERROR_OK){
    if(domain != NULL)
      freeMemory(domain);
    if(key != NULL)
      freeMemory(key);
    simple_memory_buffer_free(&ds);
    return ERROR_UNKNOWN;
  }

  if(ret == ERROR_OK){
    switch(packettype){

      /* Int handling */
      case PACKET_GET_INT:
         ret = database_get_int64(server->db, (char*)domain, (char*)key, &integer);
         if(ret != ERROR_OK) break;

         if(data_store_write_byte(&response_ds, PACKET_INT) != ERROR_OK ||
            bpack(&response_ds, "l", integer) != ERROR_OK)
           ret = ERROR_UNKNOWN; 
         break;

      case PACKET_SET_INT:
         if(bunpack(&ds, "l", &integer) != ERROR_OK){
           ret = ERROR_UNKNOWN; break;
          }

         ret = database_set_int64(server->db, (char*)domain, (char*)key, integer);
         if(ret != ERROR_OK) break;

         if(data_store_write_byte(&response_ds, PACKET_OK) != ERROR_OK)
           ret = ERROR_UNKNOWN; 
         break;

      /* Double handling */
      case PACKET_GET_DOUBLE:
         ret = database_get_double(server->db, (char*)domain, (char*)key, &dob);
         if(ret != ERROR_OK) break;

         if(data_store_write_byte(&response_ds, PACKET_DOUBLE) != ERROR_OK ||
            bpack(&response_ds, "d", dob) != ERROR_OK)
           ret = ERROR_UNKNOWN; 
         break;

      case PACKET_SET_DOUBLE:
         if(bunpack(&ds, "d", &dob) != ERROR_OK){
           ret = ERROR_UNKNOWN; break;
         }

         ret = database_set_double(server->db, (char*)domain, (char*)key, dob);
         if(ret != ERROR_OK) break;

         if(data_store_write_byte(&response_ds, PACKET_OK) != ERROR_OK)
           ret = ERROR_UNKNOWN; 
         break;

      /* String handling */
      case PACKET_GET_STRING:
         ret = database_get_string(server->db, (char*)domain, (char*)key, &string);
         if(ret != ERROR_OK){
           if(string)
             freeMemory(string);
           break;
         }

         if(data_store_write_byte(&response_ds, PACKET_STRING) != ERROR_OK ||
            bpack(&response_ds, "s", string) != ERROR_OK)
           ret = ERROR_UNKNOWN;
         freeMemory(string);
         break;

      case PACKET_SET_STRING:
         if(bunpack(&ds, "s", &string) != ERROR_OK){
           ret = ERROR_UNKNOWN; break;
         }

         ret = database_set_string(server->db, (char*)domain, (char*)key, string);
         if(ret != ERROR_OK) break;
         freeMemory(string);

         if(data_store_write_byte(&response_ds, PACKET_OK) != ERROR_OK)
           ret = ERROR_UNKNOWN; 
         break;

      /* Blob handling */
      case PACKET_GET_BLOB:
         ret = database_get_blob(server->db, (char*)domain, (char*)key, &blob, &bsize);
         if(ret != ERROR_OK){
           if(blob != NULL)
             freeMemory(blob);
           break;
         }

         if(data_store_write_byte(&response_ds, PACKET_BLOB) != ERROR_OK ||
            bpack(&response_ds, "b", bsize, blob) != ERROR_OK)
           ret = ERROR_UNKNOWN; 
         freeMemory(blob);
         break;

      case PACKET_SET_BLOB:
         if(bunpack(&ds, "b", &bsize, &blob) != ERROR_OK){
           ret = ERROR_UNKNOWN; break;
         }

         ret = database_set_blob(server->db, (char*)domain, (char*)key, blob, bsize);
         if(ret != ERROR_OK) break;
         freeMemory(blob);

         if(data_store_write_byte(&response_ds, PACKET_OK) != ERROR_OK)
           ret = ERROR_UNKNOWN; 
         break;

      /* Others handling */
      case PACKET_GET_ENUM:
         ret = database_enum_keys(server->db, (char*)domain, (char*)key, &count, &esize, &keys);
         if(ret != ERROR_OK){
           if(keys != NULL)
             freeMemory(keys);
           break;
         }
         
         count_enum = count;
         if(data_store_write_byte(&response_ds, PACKET_ENUM) != ERROR_OK)
           ret = ERROR_UNKNOWN; 
         if(keys == NULL){
           if(bpack(&response_ds, "l", count_enum) != ERROR_OK)
             ret = ERROR_UNKNOWN; 
         }else{
           if(bpack(&response_ds, "lb", count_enum, esize, keys) != ERROR_OK)
             ret = ERROR_UNKNOWN; 
           freeMemory(keys);
         }
         break;

      case PACKET_GET_VALUE_TYPE:
         ret = database_get_type(server->db, (char*)domain, (char*)key, &type);
         if(ret != ERROR_OK) break;

         if(data_store_write_byte(&response_ds, PACKET_TYPE) != ERROR_OK ||
            bpack(&response_ds, "l", (int64_t)type) != ERROR_OK)
           ret = ERROR_UNKNOWN; 
         break;

      case PACKET_SHUTDOWN:
        if(server_shutdown(server) != ERROR_OK){
          ret = ERROR_UNKNOWN; break;
        } 
        ret =  ERROR_SERVER_SHUTDOWN; 
        break;

      default: ret = ERROR_UNKNOWN; break;
    }
  freeMemory(domain);
  freeMemory(key);
  }

  /* free input datastore */
  if(simple_memory_buffer_free(&ds) != ERROR_OK)
    ret = ERROR_UNKNOWN;

  /* send packet */
  if(ret == ERROR_OK){
    if(sendPacket(&response_ds, response_size, response) != ERROR_OK)
      ret = ERROR_UNKNOWN;
  }

  if(simple_memory_buffer_free(&response_ds) != ERROR_OK)
    ret = ERROR_UNKNOWN;

  /* error packet */
  if(ret != ERROR_OK && ret != ERROR_SERVER_SHUTDOWN){
    data_store_t error_ds;
    if(simple_memory_buffer_new(&error_ds, NULL, 0) != ERROR_OK ||
       data_store_write_byte(&error_ds, PACKET_ERROR) != ERROR_OK ||
       bpack(&error_ds, "l", ret) != ERROR_OK){
      simple_memory_buffer_free(&error_ds);
      ret = ERROR_UNKNOWN; 
    }else{
      if((ret = sendPacket(&error_ds, response_size, response)) != ERROR_OK)
        ret = ERROR_UNKNOWN;
      simple_memory_buffer_free(&error_ds);
    }
  }

  return ret;
}

/* -------------------------------------------------------------------------- */
int
sendPacket(data_store_t *ds, size_t *response_size, unsigned char **response)
{
  if(simple_memory_buffer_get_size(ds, response_size) != ERROR_OK)
    return ERROR_UNKNOWN;

  /* request memory for response */ 
  if(requestMemory((void**)response, *response_size + 1) != ERROR_OK)
    return ERROR_MEMORY;
  
  unsigned char *buffer = NULL;
  if(simple_memory_buffer_get_data(ds, &buffer) != ERROR_OK){
    freeMemory(*response);
    return ERROR_UNKNOWN;
  }

  memcpy(*response, buffer, *response_size);
  (*response)[*response_size] = '\0';
  return ERROR_OK;
}

/* -------------------------------------------------------------------------- */
int
server_shutdown(server_t* server)
{
  if(server == NULL || server->db == NULL)
    return ERROR_INVALID_ARGUMENTS;
 
  /* free database */
  int ret = database_close(server->db);
  if(ret != ERROR_OK){
    freeMemory(server);
    return ERROR_UNKNOWN;
  }

  freeMemory(server);

  return ERROR_OK;
}

