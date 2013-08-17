/** @brief Database interface
 *
 * This file contains the implementation of the database of 'the registry'.
 *
 * @file database.c
 */

#ifndef REALPATH
#define REALPATH
#define _XOPEN_SOURCE 500
#include <features.h>
#endif // REALPATH

#include "database.h"
#include "../errors.h"
#include <sqlite3.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <limits.h>
#include "../memory.h"
#include "../datastructure.h"
#include <math.h>



int check_blob_path(const char* blobpath, const char* referencepath);
int removeReferencedBlobFile(database_handle_t* handle, const char* domain, const char* key);

int begin(database_handle_t* handle){
  sqlite3_stmt *ppStmt = NULL;
  const char** pzTail = NULL;
  if(sqlite3_prepare_v2(handle->db, "BEGIN;", -1, &ppStmt, pzTail) != SQLITE_OK){
    return ERROR_DATABASE_INVALID;
  }

  while (42){
      int retval = sqlite3_step(ppStmt);

      if(retval == SQLITE_DONE){    
        break;
      }
      else if(retval == SQLITE_BUSY){
        continue;
      }
      else {
        return ERROR_DATABASE_INVALID;
      } 
  }

  if(sqlite3_finalize(ppStmt) != SQLITE_OK){
    return ERROR_DATABASE_INVALID;
  }
  return ERROR_OK;
}

int commit(database_handle_t* handle){
  sqlite3_stmt *ppStmt = NULL;
  const char** pzTail = NULL;
  if(sqlite3_prepare_v2(handle->db, "COMMIT;", -1, &ppStmt, pzTail) != SQLITE_OK){
    return ERROR_DATABASE_INVALID;
  }

  while (42){
      int retval = sqlite3_step(ppStmt);

      if(retval == SQLITE_DONE){    
        break;
      }
      else if(retval == SQLITE_BUSY){
        continue;
      }
      else {
        return ERROR_DATABASE_INVALID;
      } 
  }

  if(sqlite3_finalize(ppStmt) != SQLITE_OK){
    return ERROR_DATABASE_INVALID;
  }
  return ERROR_OK;
}

int rollback(database_handle_t* handle){
  sqlite3_stmt *ppStmt = NULL;
  const char** pzTail = NULL;
  if(sqlite3_prepare_v2(handle->db, "ROLLBACK;", -1, &ppStmt, pzTail) != SQLITE_OK){
    return ERROR_DATABASE_INVALID;
  }

  while (42){
      int retval = sqlite3_step(ppStmt);

      if(retval == SQLITE_DONE){    
        break;
      }
      else if(retval == SQLITE_BUSY){
        continue;
      }
      else {
        return ERROR_DATABASE_INVALID;
      } 
  }

  if(sqlite3_finalize(ppStmt) != SQLITE_OK){
    return ERROR_DATABASE_INVALID;
  }
  return ERROR_OK;
}

int
database_open(database_handle_t** handle, const char* path)
{
  /* check for invalid arguments */
  if(handle == NULL || path == NULL)
    return ERROR_INVALID_ARGUMENTS;

  /* check if path is a regular file */
  struct stat sb;
  if(stat(path, &sb) != 0){
    //printf("test\n");
    return ERROR_DATABASE_OPEN;
  }
  if(!S_ISREG(sb.st_mode)){        /* check if regular file */
    //printf("regular file error");
    return ERROR_DATABASE_OPEN;
  }
  /* check if directory */
  if(S_ISDIR(sb.st_mode)){
    //printf("directory ok\n");
    return ERROR_DATABASE_OPEN;
  }

  /* allocate database_handle_t */
  database_handle_t* dbhandle = NULL;
  if(requestMemory((void**)&dbhandle, sizeof(database_handle_t)) != ERROR_OK){
    return ERROR_MEMORY;
  }

  if(dbhandle == NULL){
    return ERROR_MEMORY; 
  }

  // opens the database defined in path with read/write access
  // database must already exist otherwise an error occur
  // handle is null if not enough memory exists otherwise no error occurs
  // use default sqlite3_vfs object
  if(sqlite3_open_v2(path, &dbhandle->db, SQLITE_OPEN_READWRITE, NULL) != SQLITE_OK){
    //printf("%s\n", sqlite3_errmsg(dbhandle->db));
    sqlite3_close(dbhandle->db);
    freeMemory(dbhandle);
    return ERROR_DATABASE_OPEN;
  }

  if(dbhandle->db == NULL){
    sqlite3_close(dbhandle->db);
    freeMemory(dbhandle);
    return ERROR_MEMORY;
  }

  /* check if database is in a well defined state */
  const char* type = NULL;
  int notnull = 0, primarykey = 0, autoinc = 0;

  /** Datatypes */
  if(sqlite3_table_column_metadata(dbhandle->db,   /* Connection handle*/
                                   NULL,           /* Database name */
                                   "Datatypes",    /* Table name */
                                   "type",         /* Column name */
                                   &type,          /* OUT: data type */
                                   NULL,           /* OUT: sequence name */
                                   &notnull,       /* OUT: true if not null constraint */
                                   &primarykey,    /* OUT: true if private key */
                                   &autoinc)       /* OUT: true if auto inc */
                                   != SQLITE_OK){
    sqlite3_close(dbhandle->db);
    freeMemory(dbhandle);
    return ERROR_DATABASE_INVALID;
  }
  if(strcmp(type, "TEXT") != 0 ||
     notnull == 0 ||
     primarykey == 0){
    sqlite3_close(dbhandle->db);
    freeMemory(dbhandle);
    return ERROR_DATABASE_INVALID;
  }
  notnull = 0; primarykey = 0; autoinc = 0;

  /** KeyInfo */
  /* id */
  if(sqlite3_table_column_metadata(dbhandle->db,   /* Connection handle*/
                                   NULL,           /* Database name */
                                   "KeyInfo",      /* Table name */
                                   "id",           /* Column name */
                                   &type,          /* OUT: data type */
                                   NULL,           /* OUT: sequence name */
                                   &notnull,       /* OUT: true if not null constraint */
                                   &primarykey,    /* OUT: true if private key */
                                   &autoinc)       /* OUT: true if auto inc */
                                   != SQLITE_OK){
    sqlite3_close(dbhandle->db);
    freeMemory(dbhandle);
    return ERROR_DATABASE_INVALID;
  }
  if(strcmp(type, "INTEGER") != 0 ||
     notnull == 0 ||
     primarykey == 0 ||
     autoinc == 0){
    sqlite3_close(dbhandle->db);
    freeMemory(dbhandle);
    return ERROR_DATABASE_INVALID;
  }
  notnull = 0; primarykey = 0; autoinc = 0;

  /* domain */
  if(sqlite3_table_column_metadata(dbhandle->db,   /* Connection handle*/
                                   NULL,           /* Database name */
                                   "KeyInfo",      /* Table name */
                                   "domain",       /* Column name */
                                   &type,          /* OUT: data type */
                                   NULL,           /* OUT: sequence name */
                                   &notnull,       /* OUT: true if not null constraint */
                                   &primarykey,    /* OUT: true if private key */
                                   &autoinc)       /* OUT: true if auto inc */
                                   != SQLITE_OK){
    sqlite3_close(dbhandle->db);
    freeMemory(dbhandle);
    return ERROR_DATABASE_INVALID;
  }
  if(strcmp(type, "TEXT") != 0){
    sqlite3_close(dbhandle->db);
    freeMemory(dbhandle);
    return ERROR_DATABASE_INVALID;
  }
  notnull = 0; primarykey = 0; autoinc = 0;

  /* key */
  if(sqlite3_table_column_metadata(dbhandle->db,   /* Connection handle*/
                                   NULL,           /* Database name */
                                   "KeyInfo",      /* Table name */
                                   "key",          /* Column name */
                                   &type,          /* OUT: data type */
                                   NULL,           /* OUT: sequence name */
                                   &notnull,       /* OUT: true if not null constraint */
                                   &primarykey,    /* OUT: true if private key */
                                   &autoinc)       /* OUT: true if auto inc */
                                   != SQLITE_OK){
    sqlite3_close(dbhandle->db);
    freeMemory(dbhandle);
    return ERROR_DATABASE_INVALID;
  }
  if(strcmp(type, "TEXT") != 0 ||
     notnull == 0){
    sqlite3_close(dbhandle->db);
    freeMemory(dbhandle);
    return ERROR_DATABASE_INVALID;
  }
  notnull = 0; primarykey = 0; autoinc = 0;

  /* datatype */
  if(sqlite3_table_column_metadata(dbhandle->db,   /* Connection handle*/
                                   NULL,           /* Database name */
                                   "KeyInfo",      /* Table name */
                                   "datatype",     /* Column name */
                                   &type,          /* OUT: data type */
                                   NULL,           /* OUT: sequence name */
                                   &notnull,       /* OUT: true if not null constraint */
                                   &primarykey,    /* OUT: true if private key */
                                   &autoinc)       /* OUT: true if auto inc */
                                   != SQLITE_OK){
    sqlite3_close(dbhandle->db);
    freeMemory(dbhandle);
    return ERROR_DATABASE_INVALID;
  }
  if(strcmp(type, "TEXT") != 0 ||
     notnull == 0){
    sqlite3_close(dbhandle->db);
    freeMemory(dbhandle);
    return ERROR_DATABASE_INVALID;
  }
  notnull = 0; primarykey = 0; autoinc = 0;

  /** ValueInt64 */
  /* id */
  if(sqlite3_table_column_metadata(dbhandle->db,   /* Connection handle*/
                                   NULL,           /* Database name */
                                   "ValueInt64",   /* Table name */
                                   "id",           /* Column name */
                                   &type,          /* OUT: data type */
                                   NULL,           /* OUT: sequence name */
                                   &notnull,       /* OUT: true if not null constraint */
                                   &primarykey,    /* OUT: true if private key */
                                   &autoinc)       /* OUT: true if auto inc */
                                   != SQLITE_OK){
    sqlite3_close(dbhandle->db);
    freeMemory(dbhandle);
    return ERROR_DATABASE_INVALID;
  }
  if(strcmp(type, "INTEGER") != 0 ||
     notnull == 0 ||
     primarykey == 0){
    sqlite3_close(dbhandle->db);
    freeMemory(dbhandle);
    return ERROR_DATABASE_INVALID;
  }
  notnull = 0; primarykey = 0; autoinc = 0;

  /* value */
  if(sqlite3_table_column_metadata(dbhandle->db,   /* Connection handle*/
                                   NULL,           /* Database name */
                                   "ValueInt64",   /* Table name */
                                   "value",        /* Column name */
                                   &type,          /* OUT: data type */
                                   NULL,           /* OUT: sequence name */
                                   &notnull,       /* OUT: true if not null constraint */
                                   &primarykey,    /* OUT: true if private key */
                                   &autoinc)       /* OUT: true if auto inc */
                                   != SQLITE_OK){
    sqlite3_close(dbhandle->db);
    freeMemory(dbhandle);
    return ERROR_DATABASE_INVALID;
  }
  if(strcmp(type, "INTEGER") != 0 ||
     notnull == 0 ){
    sqlite3_close(dbhandle->db);
    freeMemory(dbhandle);
    return ERROR_DATABASE_INVALID;
  }
  notnull = 0; primarykey = 0; autoinc = 0;

  /** ValueDouble */
  /* id */
  if(sqlite3_table_column_metadata(dbhandle->db,   /* Connection handle*/
                                   NULL,           /* Database name */
                                   "ValueDouble",  /* Table name */
                                   "id",           /* Column name */
                                   &type,          /* OUT: data type */
                                   NULL,           /* OUT: sequence name */
                                   &notnull,       /* OUT: true if not null constraint */
                                   &primarykey,    /* OUT: true if private key */
                                   &autoinc)       /* OUT: true if auto inc */
                                   != SQLITE_OK){
    sqlite3_close(dbhandle->db);
    freeMemory(dbhandle);
    return ERROR_DATABASE_INVALID;
  }
  if(strcmp(type, "INTEGER") != 0 ||
     notnull == 0 ||
     primarykey == 0){
    sqlite3_close(dbhandle->db);
    freeMemory(dbhandle);
    return ERROR_DATABASE_INVALID;
  }
  notnull = 0; primarykey = 0; autoinc = 0;

  /* value */
  if(sqlite3_table_column_metadata(dbhandle->db,   /* Connection handle*/
                                   NULL,           /* Database name */
                                   "ValueDouble",  /* Table name */
                                   "value",        /* Column name */
                                   &type,          /* OUT: data type */
                                   NULL,           /* OUT: sequence name */
                                   &notnull,       /* OUT: true if not null constraint */
                                   &primarykey,    /* OUT: true if private key */
                                   &autoinc)       /* OUT: true if auto inc */
                                   != SQLITE_OK){
    sqlite3_close(dbhandle->db);
    freeMemory(dbhandle);
    return ERROR_DATABASE_INVALID;
  }
  if(strcmp(type, "REAL") != 0 ||
     notnull == 0 ){
    sqlite3_close(dbhandle->db);
    freeMemory(dbhandle);
    return ERROR_DATABASE_INVALID;
  }
  notnull = 0; primarykey = 0; autoinc = 0;

  /** ValueString */
  /* id */
  if(sqlite3_table_column_metadata(dbhandle->db,   /* Connection handle*/
                                   NULL,           /* Database name */
                                   "ValueString",  /* Table name */
                                   "id",           /* Column name */
                                   &type,          /* OUT: data type */
                                   NULL,           /* OUT: sequence name */
                                   &notnull,       /* OUT: true if not null constraint */
                                   &primarykey,    /* OUT: true if private key */
                                   &autoinc)       /* OUT: true if auto inc */
                                   != SQLITE_OK){
    sqlite3_close(dbhandle->db);
    freeMemory(dbhandle);
    return ERROR_DATABASE_INVALID;
  }
  if(strcmp(type, "INTEGER") != 0 ||
     notnull == 0 ||
     primarykey == 0){
    sqlite3_close(dbhandle->db);
    freeMemory(dbhandle);
    return ERROR_DATABASE_INVALID;
  }
  notnull = 0; primarykey = 0; autoinc = 0;

  /* value */
  if(sqlite3_table_column_metadata(dbhandle->db,   /* Connection handle*/
                                   NULL,           /* Database name */
                                   "ValueString",  /* Table name */
                                   "value",        /* Column name */
                                   &type,          /* OUT: data type */
                                   NULL,           /* OUT: sequence name */
                                   &notnull,       /* OUT: true if not null constraint */
                                   &primarykey,    /* OUT: true if private key */
                                   &autoinc)       /* OUT: true if auto inc */
                                   != SQLITE_OK){
    sqlite3_close(dbhandle->db);
    freeMemory(dbhandle);
    return ERROR_DATABASE_INVALID;
  }
  if(strcmp(type, "TEXT") != 0 ||
     notnull == 0 ){
    sqlite3_close(dbhandle->db);
    freeMemory(dbhandle);
    return ERROR_DATABASE_INVALID;
  }
  notnull = 0; primarykey = 0; autoinc = 0;

  /** ValueBlob */
  /* id */
  if(sqlite3_table_column_metadata(dbhandle->db,   /* Connection handle*/
                                   NULL,           /* Database name */
                                   "ValueBlob",   /* Table name */
                                   "id",           /* Column name */
                                   &type,          /* OUT: data type */
                                   NULL,           /* OUT: sequence name */
                                   &notnull,       /* OUT: true if not null constraint */
                                   &primarykey,    /* OUT: true if private key */
                                   &autoinc)       /* OUT: true if auto inc */
                                   != SQLITE_OK){
    sqlite3_close(dbhandle->db);
    freeMemory(dbhandle);
    return ERROR_DATABASE_INVALID;
  }
  if(strcmp(type, "INTEGER") != 0 ||
     notnull == 0 ||
     primarykey == 0){
    sqlite3_close(dbhandle->db);
    freeMemory(dbhandle);
    return ERROR_DATABASE_INVALID;
  }
  notnull = 0; primarykey = 0; autoinc = 0;

  /* path */
  if(sqlite3_table_column_metadata(dbhandle->db,   /* Connection handle*/
                                   NULL,           /* Database name */
                                   "ValueBlob",    /* Table name */
                                   "path",         /* Column name */
                                   &type,          /* OUT: data type */
                                   NULL,           /* OUT: sequence name */
                                   &notnull,       /* OUT: true if not null constraint */
                                   &primarykey,    /* OUT: true if private key */
                                   &autoinc)       /* OUT: true if auto inc */
                                   != SQLITE_OK){
    sqlite3_close(dbhandle->db);
    freeMemory(dbhandle);
    return ERROR_DATABASE_INVALID;
  }
  if(strcmp(type, "TEXT") != 0 ||
     notnull == 0 ){
    sqlite3_close(dbhandle->db);
    freeMemory(dbhandle);
    return ERROR_DATABASE_INVALID;
  }


  /* check if the blob-path is an absolute path and is a directory */
  sqlite3_stmt *ppStmt = NULL;
  const char** pzTail = NULL;
  char* statement = "SELECT ValueString.`value` as `value` FROM KeyInfo INNER JOIN ValueString ON KeyInfo.`id` = ValueString.`id`WHERE KeyInfo.`datatype` = 'String' AND KeyInfo.`key`= 'blob-path';";

  begin(dbhandle);

  if(sqlite3_prepare_v2(dbhandle->db, statement, -1, &ppStmt, pzTail) != SQLITE_OK){
    //printf("prepare: %s\n", sqlite3_errmsg(dbhandle->db));
    sqlite3_finalize(ppStmt);
    rollback(dbhandle);
    sqlite3_close(dbhandle->db);
    freeMemory(dbhandle);
    return ERROR_DATABASE_INVALID;
  }

  while (42){
      int retval = sqlite3_step(ppStmt);

      if(retval == SQLITE_ROW){    
        if(sqlite3_column_type(ppStmt, 0) != SQLITE3_TEXT){
          sqlite3_finalize(ppStmt);
          rollback(dbhandle);
          sqlite3_close(dbhandle->db);
          freeMemory(dbhandle);
          return ERROR_DATABASE_INVALID;
        } 

        char * dbentry = NULL;
        dbentry = (char*)sqlite3_column_text(ppStmt, 0); 
        if(requestMemory((void**)&dbhandle->blobpath, strlen(dbentry)+1) != ERROR_OK){
          sqlite3_finalize(ppStmt);
          rollback(dbhandle);
          sqlite3_close(dbhandle->db);
          freeMemory(dbhandle);
          return ERROR_MEMORY;
        }
        memcpy(dbhandle->blobpath, dbentry, strlen(dbentry));
        dbhandle->blobpath[strlen(dbentry)] = '\0';
        break;
      }
      else if(retval == SQLITE_DONE){
        sqlite3_finalize(ppStmt);
        rollback(dbhandle);
        sqlite3_close(dbhandle->db);
        freeMemory(dbhandle);
        return ERROR_DATABASE_INVALID;
      }
      else {
        //printf("step: %s\n", sqlite3_errmsg(dbhandle->db));
        sqlite3_finalize(ppStmt);
        rollback(dbhandle);
        sqlite3_close(dbhandle->db);
        freeMemory(dbhandle);
        return ERROR_DATABASE_INVALID;
      } 
  }

  if(sqlite3_finalize(ppStmt) != SQLITE_OK){
    //printf("finalize: %s\n", sqlite3_errmsg(dbhandle->db));
    rollback(dbhandle);
    sqlite3_close(dbhandle->db);
    freeMemory(dbhandle->blobpath);
    freeMemory(dbhandle);
    return ERROR_DATABASE_INVALID;
  }

  commit(dbhandle);

  if(stat(dbhandle->blobpath, &sb) != 0){
    sqlite3_close(dbhandle->db);
    freeMemory(dbhandle->blobpath);
    freeMemory(dbhandle);
    return ERROR_DATABASE_INVALID;
  }
  /* check if directory */
  if(!S_ISDIR(sb.st_mode)){        
    //printf("directory error2\n");
    sqlite3_close(dbhandle->db);
    freeMemory(dbhandle->blobpath);
    freeMemory(dbhandle);
    return ERROR_DATABASE_INVALID;
  }
  /*absolute path*/
  if(dbhandle->blobpath[0] != '/'){
    //printf("absolute path2");
    sqlite3_close(dbhandle->db);
    freeMemory(dbhandle->blobpath);
    freeMemory(dbhandle);
    return ERROR_DATABASE_INVALID;
  }

  *handle = dbhandle;

  return ERROR_OK;
}


int
database_close(database_handle_t* handle)
{
  if(handle == NULL || handle->db == NULL || handle->blobpath == NULL || strlen(handle->blobpath) == 0){
    return ERROR_INVALID_ARGUMENTS;
  }
  /* close db connection */
  while(sqlite3_close(handle->db) != SQLITE_OK){
   //printf("close: %s\n", sqlite3_errmsg(handle->db));
  }
  /* free memory for blob-path */
  freeMemory(handle->blobpath);
  /* free handle */  
  free(handle);

  return ERROR_OK;
}


int
database_get_type(database_handle_t* handle, const char* domain, 
                  const char* key, database_value_type_t* type)
{
  /* Input checks */
  if(handle == NULL || handle->db == NULL || handle->blobpath == NULL || domain  == NULL || key == NULL || strlen(handle->blobpath) == 0 || strlen(domain) == 0 || strlen(key) == 0 || type == NULL)
    return ERROR_INVALID_ARGUMENTS;

  /* Some variables */
  char* statement = NULL;
  sqlite3_stmt *ppStmt = NULL;
  const char** pzTail = NULL;
  unsigned int error = 0;

  error = requestMemory((void**)&statement, 65);
  if(error != ERROR_OK)
    return error;

  strcpy(statement, "SELECT datatype FROM KeyInfo WHERE domain = :dom and key = :key;");

  begin(handle);

  if(sqlite3_prepare_v2(handle->db, statement, -1, &ppStmt, pzTail) != SQLITE_OK){
    //printf("prepare: %s\n", sqlite3_errmsg(handle->db));
    sqlite3_finalize(ppStmt);
    rollback(handle);
    freeMemory(statement); 
    return ERROR_DATABASE_INVALID;
  }
  freeMemory(statement);

  int parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":dom");
  if(parameterIndex == 0){
    //printf("bind parameter domain: %s\n", sqlite3_errmsg(handle->db));
    sqlite3_finalize(ppStmt);
    rollback(handle);
    return ERROR_DATABASE_INVALID;
  }

  if(sqlite3_bind_text(ppStmt, parameterIndex, domain, -1, SQLITE_TRANSIENT) != SQLITE_OK){
    //printf("bind domain: %s\n", sqlite3_errmsg(handle->db));
    sqlite3_finalize(ppStmt);
    rollback(handle);
    return ERROR_DATABASE_INVALID;
  }
  
  parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":key");
  if(parameterIndex == 0){
    //printf("bind parameter key: %s\n", sqlite3_errmsg(handle->db));
    sqlite3_finalize(ppStmt);
    rollback(handle);
    return ERROR_DATABASE_INVALID;
  }

  if(sqlite3_bind_text(ppStmt, parameterIndex, key, -1, SQLITE_TRANSIENT) != SQLITE_OK){
    //printf("bind key: %s\n", sqlite3_errmsg(handle->db));  
    sqlite3_finalize(ppStmt);
    rollback(handle);
    return ERROR_DATABASE_INVALID;
  }

  while (42){
      int retval = sqlite3_step(ppStmt);

      if(retval == SQLITE_ROW){    
        if(sqlite3_column_type(ppStmt, 0) != SQLITE3_TEXT){
          sqlite3_finalize(ppStmt);
          rollback(handle);
          return ERROR_DATABASE_TYPE_MISMATCH;
        } 
        char* datatype = NULL;
        if(requestMemory((void**)&datatype, 20) != ERROR_OK){
          sqlite3_finalize(ppStmt);
          rollback(handle);
          return ERROR_MEMORY;
        }
        strcpy(datatype, (char*)sqlite3_column_text(ppStmt, 0)); 
        if(!strcmp(datatype, "Int64")){
          *type = DATABASE_TYPE_INT64;
        }        
        else if(!strcmp(datatype, "Double")){
          *type = DATABASE_TYPE_DOUBLE;
        }
        else if(!strcmp(datatype, "String")){
          *type = DATABASE_TYPE_STRING;
        }
        else if(!strcmp(datatype, "Blob")){
          *type = DATABASE_TYPE_BLOB;
        }
        else {
          free(datatype);
          sqlite3_finalize(ppStmt);
          rollback(handle);
          return ERROR_DATABASE_TYPE_UNKNOWN;
        }
        free(datatype);
        break;
      }
      else if(retval == SQLITE_DONE){
        sqlite3_finalize(ppStmt);
        rollback(handle);
        return ERROR_DATABASE_NO_SUCH_KEY;
      }
      else {
        sqlite3_finalize(ppStmt);
        rollback(handle);
        //printf("step: %s\n", sqlite3_errmsg(handle->db));
        return ERROR_DATABASE_INVALID;
      } 
  }

  if(sqlite3_reset(ppStmt) != SQLITE_OK){
    //printf("reset: %s\n", sqlite3_errmsg(handle->db));
    sqlite3_finalize(ppStmt);
    rollback(handle);
    return ERROR_DATABASE_INVALID;
  }

  if(sqlite3_finalize(ppStmt) != SQLITE_OK){
    //printf("finalize: %s\n", sqlite3_errmsg(handle->db));
    rollback(handle);
    return ERROR_DATABASE_INVALID;
  }
  commit(handle);

  return ERROR_OK;
}


int
database_enum_keys(database_handle_t* handle, const char* domain,
                   const char* pattern, size_t* count, size_t* size, char** keys)
{
 if(handle == NULL || handle->db == NULL || handle->blobpath == NULL || domain == NULL || pattern == NULL || count == NULL || size == NULL || keys == NULL || strlen(handle->blobpath) == 0 || strlen(domain) == 0)
    return ERROR_INVALID_ARGUMENTS;
  
  char* statement = NULL;
  sqlite3_stmt *ppStmt = NULL;
  const char** pzTail = NULL;

  unsigned int sizeOfStmt = 80;
  unsigned int error = 0;
  error = requestMemory((void**)&statement, sizeOfStmt);
  if(error != ERROR_OK)
    return error;

  strcpy(statement, "SELECT key FROM KeyInfo WHERE domain = :dom AND key GLOB :pat ORDER BY key ASC;");

  begin(handle);
  if(sqlite3_prepare_v2(handle->db, statement, -1, &ppStmt, pzTail) != SQLITE_OK){
    //printf("prepare: %s\n", sqlite3_errmsg(handle->db));
    sqlite3_finalize(ppStmt);
    rollback(handle);
    freeMemory(statement); 
    return ERROR_DATABASE_INVALID;
  }
  freeMemory(statement);  

  int parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":dom");
  if(parameterIndex == 0){
    sqlite3_finalize(ppStmt);
    rollback(handle);
    //printf("bind parameter domain: %s\n", sqlite3_errmsg(handle->db));
    return ERROR_DATABASE_INVALID;
  }

  if(sqlite3_bind_text(ppStmt, parameterIndex, domain, -1, SQLITE_TRANSIENT) != SQLITE_OK){
    sqlite3_finalize(ppStmt);
    rollback(handle);
    //printf("bind domain: %s\n", sqlite3_errmsg(handle->db));
    return ERROR_DATABASE_INVALID;
  }
  
  parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":pat");
  if(parameterIndex == 0){
    sqlite3_finalize(ppStmt);
    rollback(handle);
    //printf("bind parameter pattern: %s\n", sqlite3_errmsg(handle->db));
    return ERROR_DATABASE_INVALID;
  }

  if(sqlite3_bind_text(ppStmt, parameterIndex, pattern, -1, SQLITE_TRANSIENT) != SQLITE_OK){
    sqlite3_finalize(ppStmt);
    rollback(handle);
    //printf("bind pattern: %s\n", sqlite3_errmsg(handle->db));  
    return ERROR_DATABASE_INVALID;
  }

  while (42){
      int retval = sqlite3_step(ppStmt);
      
      if(retval == SQLITE_ROW){     
        if(sqlite3_column_type(ppStmt, 0) != SQLITE3_TEXT){
          sqlite3_finalize(ppStmt);
          rollback(handle);
          return ERROR_DATABASE_TYPE_MISMATCH;
        }      
        char * dbentry = NULL;
        dbentry = (char*)sqlite3_column_text(ppStmt, 0); 

        if(editMemory((void**)&*keys, (*size)+strlen(dbentry)+1) != ERROR_OK){ 
          sqlite3_finalize(ppStmt); 
          rollback(handle);
          return ERROR_MEMORY;
        }
        unsigned int i = 0;
        for(i = (*size); i < (*size)+strlen(dbentry); i++ ){
          (*keys)[i] = dbentry[i-(*size)];
        }
        (*keys)[(*size)+strlen(dbentry)] = '\0';
        (*size) += strlen(dbentry) + 1;
        (*count)++;
      }
      else if(retval == SQLITE_DONE){
        break;
      }
      else {
        //printf("step: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        return ERROR_DATABASE_INVALID;
      } 
  }

  if(sqlite3_reset(ppStmt) != SQLITE_OK){
    sqlite3_finalize(ppStmt);
    rollback(handle);
    //printf("reset: %s\n", sqlite3_errmsg(handle->db));
    return ERROR_DATABASE_INVALID;
  }

  if(sqlite3_finalize(ppStmt) != SQLITE_OK){
    //printf("finalize: %s\n", sqlite3_errmsg(handle->db));
    rollback(handle);
    return ERROR_DATABASE_INVALID;
  }

  commit(handle);
  return ERROR_OK;
}


int
database_get_int64(database_handle_t* handle, const char* domain,
                   const char* key, int64_t* value)
{
  if(handle == NULL || handle->db == NULL || handle->blobpath == NULL || domain == NULL || key == NULL || value == NULL || strlen(domain) == 0 || strlen(key) == 0 || strlen(handle->blobpath) == 0)
    return ERROR_INVALID_ARGUMENTS;
  
  char* statement = NULL;
  sqlite3_stmt *ppStmt = NULL;
  const char** pzTail = NULL;

  unsigned int sizeOfStmt = 193;
  if(requestMemory((void**)&statement, sizeOfStmt) != ERROR_OK){
    return ERROR_MEMORY;
  }

  strcpy(statement, "SELECT ValueInt64.`value` as `value` FROM KeyInfo INNER JOIN ValueInt64 ON KeyInfo.`id` = ValueInt64.`id`WHERE KeyInfo.`datatype` = 'Int64' AND KeyInfo.`domain` = :dom AND KeyInfo.`key`= :key;");

  begin(handle);
  if(sqlite3_prepare_v2(handle->db, statement, -1, &ppStmt, pzTail) != SQLITE_OK){
    sqlite3_finalize(ppStmt);
    rollback(handle);
    //printf("prepare: %s\n", sqlite3_errmsg(handle->db));
    freeMemory(statement); 
    return ERROR_DATABASE_INVALID;
  }
  freeMemory(statement);  

  int parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":dom");
  if(parameterIndex == 0){
    sqlite3_finalize(ppStmt);
    rollback(handle);
    //printf("bind parameter domain: %s\n", sqlite3_errmsg(handle->db));
    return ERROR_DATABASE_INVALID;
  }

  if(sqlite3_bind_text(ppStmt, parameterIndex, domain, -1, SQLITE_TRANSIENT) != SQLITE_OK){
    sqlite3_finalize(ppStmt);
    rollback(handle);
    //printf("bind domain: %s\n", sqlite3_errmsg(handle->db));
    return ERROR_DATABASE_INVALID;
  }
  
  parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":key");
  if(parameterIndex == 0){
    sqlite3_finalize(ppStmt);
    rollback(handle);
    //printf("bind parameter key: %s\n", sqlite3_errmsg(handle->db));
    return ERROR_DATABASE_INVALID;
  }

  if(sqlite3_bind_text(ppStmt, parameterIndex, key, -1, SQLITE_TRANSIENT) != SQLITE_OK){
    sqlite3_finalize(ppStmt);
    rollback(handle);
    //printf("bind key: %s\n", sqlite3_errmsg(handle->db));  
    return ERROR_DATABASE_INVALID;
  }

  while (42){
      int retval = sqlite3_step(ppStmt);

      if(retval == SQLITE_ROW){     
        if(sqlite3_column_type(ppStmt, 0) != SQLITE_INTEGER){
          sqlite3_finalize(ppStmt);
          rollback(handle);
          return ERROR_DATABASE_TYPE_MISMATCH;
        }      
        *value = (int64_t)sqlite3_column_int64(ppStmt, 0);
        break;
      }
      else if(retval == SQLITE_DONE){
        //printf("no such key1");
        sqlite3_finalize(ppStmt);
        rollback(handle);
        return ERROR_DATABASE_NO_SUCH_KEY;
      }
      else {
        //printf("step: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        return ERROR_DATABASE_INVALID;
      } 
  }

  if(sqlite3_reset(ppStmt) != SQLITE_OK){
    //printf("reset: %s\n", sqlite3_errmsg(handle->db));
    sqlite3_finalize(ppStmt);
    rollback(handle);
    return ERROR_DATABASE_INVALID;
  }

  if(sqlite3_finalize(ppStmt) != SQLITE_OK){
    //printf("finalize: %s\n", sqlite3_errmsg(handle->db));
    rollback(handle);
    return ERROR_DATABASE_INVALID;
  }
  commit(handle);

  return ERROR_OK;
} 

int
database_set_int64(database_handle_t* handle, const char* domain,
                   const char* key, int64_t value)
{
  /* Input checks */
  if(handle == NULL || handle->db == NULL || handle->blobpath == NULL || domain  == NULL || key == NULL || strlen(handle->blobpath) == 0 || strlen(domain) == 0 || strlen(key) == 0)
    return ERROR_INVALID_ARGUMENTS;

  /* Some variables */
  char* statement = NULL;
  sqlite3_stmt *ppStmt = NULL;
  const char** pzTail = NULL;

  /* Check if already existing */
  char* datatype = NULL;
  unsigned int id = 0;

  if(requestMemory((void**)&datatype, 7) != ERROR_OK)
    return ERROR_MEMORY;
  if(requestMemory((void**)&statement, 69) != ERROR_OK)
    return ERROR_MEMORY;

  strcpy(statement, "SELECT id, datatype FROM KeyInfo WHERE domain = :dom and key = :key;");

  begin(handle);
  if(sqlite3_prepare_v2(handle->db, statement, -1, &ppStmt, pzTail) != SQLITE_OK){
    //printf("prepare: %s\n", sqlite3_errmsg(handle->db));
    sqlite3_finalize(ppStmt);
    rollback(handle);
    freeMemory(statement);
    freeMemory(datatype);  
    return ERROR_DATABASE_INVALID;
  }
  freeMemory(statement);

  int parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":dom");
  if(parameterIndex == 0){
    //printf("bind parameter domain: %s\n", sqlite3_errmsg(handle->db));
    sqlite3_finalize(ppStmt);
    rollback(handle);
    freeMemory(datatype);  
    return ERROR_DATABASE_INVALID;
  }

  if(sqlite3_bind_text(ppStmt, parameterIndex, domain, -1, SQLITE_TRANSIENT) != SQLITE_OK){
    //printf("bind domain: %s\n", sqlite3_errmsg(handle->db));
    sqlite3_finalize(ppStmt);
    rollback(handle);
    freeMemory(datatype);  
    return ERROR_DATABASE_INVALID;
  }
  
  parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":key");
  if(parameterIndex == 0){
    //printf("bind parameter key: %s\n", sqlite3_errmsg(handle->db));
    sqlite3_finalize(ppStmt);
    rollback(handle);
    freeMemory(datatype);  
    return ERROR_DATABASE_INVALID;
  }

  if(sqlite3_bind_text(ppStmt, parameterIndex, key, -1, SQLITE_TRANSIENT) != SQLITE_OK){
    //printf("bind key: %s\n", sqlite3_errmsg(handle->db));  
    sqlite3_finalize(ppStmt);
    rollback(handle);
    freeMemory(datatype);  
    return ERROR_DATABASE_INVALID;
  }

  while (42){
      int retval = sqlite3_step(ppStmt);

      if(retval == SQLITE_ROW){
        if(sqlite3_column_type(ppStmt, 0) != SQLITE_INTEGER){
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          return ERROR_DATABASE_TYPE_MISMATCH;
        }
        id = sqlite3_column_int(ppStmt, 0);     
        strcpy(datatype, (char*)sqlite3_column_text(ppStmt, 1)); 
        break;
      }
      else if(retval == SQLITE_DONE){
        freeMemory(datatype);  
        datatype = NULL;
        break;
      }
      else {
        //printf("step: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      } 
  }

  if(sqlite3_reset(ppStmt) != SQLITE_OK){
    //printf("reset: %s\n", sqlite3_errmsg(handle->db));
    sqlite3_finalize(ppStmt);
    rollback(handle);
    freeMemory(datatype);  
    return ERROR_DATABASE_INVALID;
  }

  if(sqlite3_finalize(ppStmt) != SQLITE_OK){
    //printf("finalize: %s\n", sqlite3_errmsg(handle->db));
    rollback(handle);
    freeMemory(datatype);  
    return ERROR_DATABASE_INVALID;
  }
  commit(handle);
  begin(handle);
  /* key doesn't exist */
  if(datatype == NULL){
    /* Insert into Keyinfo */
      if(requestMemory((void**)&statement, 79) != ERROR_OK){
        freeMemory(datatype);
        rollback(handle);
        return ERROR_MEMORY;
      }

      strcpy(statement, "INSERT INTO KeyInfo(`domain`, `key`, `datatype`) VALUES (:dom, :key, 'Int64');");

      if(sqlite3_prepare_v2(handle->db, statement, -1, &ppStmt, pzTail) != SQLITE_OK){
        //printf("prepare: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(statement);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }
      freeMemory(statement);

      parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":dom");
      if(parameterIndex == 0){
        //printf("bind parameter domain: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_bind_text(ppStmt, parameterIndex, domain, -1, SQLITE_TRANSIENT) != SQLITE_OK){
        //printf("bind domain: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }
      
      parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":key");
      if(parameterIndex == 0){
        //printf("bind parameter key: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_bind_text(ppStmt, parameterIndex, key, -1, SQLITE_TRANSIENT) != SQLITE_OK){
        //printf("bind key: %s\n", sqlite3_errmsg(handle->db)); 
        sqlite3_finalize(ppStmt); 
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      while(42){
        int retval = sqlite3_step(ppStmt);
        if(retval == SQLITE_DONE){
          break;
        }
        else if(retval == SQLITE_ERROR){
          //printf("fucking step1: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          return ERROR_DATABASE_INVALID;
        }
        else if(retval == SQLITE_MISUSE){
          //printf("fucking step2: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          return ERROR_DATABASE_INVALID;
        }
      }

      if(sqlite3_reset(ppStmt) != SQLITE_OK){
        //printf("reset: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_finalize(ppStmt) != SQLITE_OK){
        //printf("finalize: %s\n", sqlite3_errmsg(handle->db));
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      /* Insert into ValueInt64 */
      if(requestMemory((void**)&statement, 57) != ERROR_OK){
        freeMemory(datatype);  
        rollback(handle);
        return ERROR_MEMORY; 
      }

      strcpy(statement, "INSERT INTO ValueInt64(`id`, `value`) VALUES (:id,:val);");

      if(sqlite3_prepare_v2(handle->db, statement, -1, &ppStmt, pzTail) != SQLITE_OK){
        //printf("prepare: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(statement);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }
      freeMemory(statement);

      parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":id");
      if(parameterIndex == 0){
        //printf("bind parameter index: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }
        
      if(sqlite3_bind_int64(ppStmt, parameterIndex, sqlite3_last_insert_rowid(handle->db)) != SQLITE_OK){
        //printf("bind id: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      } 

      parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":val");
      if(parameterIndex == 0){
        //printf("bind parameter index: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_bind_int64(ppStmt, parameterIndex, (sqlite3_int64)value) != SQLITE_OK){
        //printf("bind value: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      while(42){
        int retval = sqlite3_step(ppStmt);
        if(retval == SQLITE_DONE){
          break;
        }
        else if(retval == SQLITE_ERROR){
          //printf("fucking step1: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          return ERROR_DATABASE_INVALID;
        }
        else if(retval == SQLITE_MISUSE){
          //printf("fucking step2: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          return ERROR_DATABASE_INVALID;
        }
      }

      if(sqlite3_reset(ppStmt) != SQLITE_OK){
        //printf("reset: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_finalize(ppStmt) != SQLITE_OK){
        //printf("finalize: %s\n", sqlite3_errmsg(handle->db));
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }
      commit(handle);
  }
  else{ /* key already exist */    
    /* Datatype is the same - just update value */
    if(!strcmp(datatype, "Int64")){  
      /* Update ValueInt64 */
      if(requestMemory((void**)&statement, 51) != ERROR_OK){
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_MEMORY;
      }

      strcpy(statement, "UPDATE ValueInt64 SET value = :val WHERE id = :id;");

      if(sqlite3_prepare_v2(handle->db, statement, -1, &ppStmt, pzTail) != SQLITE_OK){
        //printf("prepare: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(statement);
        freeMemory(datatype);   
        return ERROR_DATABASE_INVALID;
      }
      freeMemory(statement);

      parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":val");
      if(parameterIndex == 0){
        //printf("bind parameter index: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }
        
      if(sqlite3_bind_int64(ppStmt, parameterIndex, value) != SQLITE_OK){
        //printf("bind id: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      } 

      parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":id");
      if(parameterIndex == 0){
        //printf("bind parameter index: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_bind_int(ppStmt, parameterIndex, id) != SQLITE_OK){
        //printf("bind value: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      while(42){
        int retval = sqlite3_step(ppStmt);
        if(retval == SQLITE_DONE){
          break;
        }
        else if(retval == SQLITE_ERROR){
          //printf("fucking step1: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          return ERROR_DATABASE_INVALID;
        }
        else if(retval == SQLITE_MISUSE){
          //printf("fucking step2: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          return ERROR_DATABASE_INVALID;
        }
      }

      if(sqlite3_reset(ppStmt) != SQLITE_OK){
        //printf("reset: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_finalize(ppStmt) != SQLITE_OK){
        //printf("finalize: %s\n", sqlite3_errmsg(handle->db));
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }
    }
    else{ /* Different datatype - delete, update and insert */
      
      commit(handle);
      //remove referenced blob file
      if(!strcmp(datatype, "Blob")){
        int ret = removeReferencedBlobFile(handle, domain, key);
        if(ret != ERROR_OK)
          return ret;
      }
      begin(handle);
      /* Delete from ValueXtable */
      if(requestMemory((void**)&statement, 40) != ERROR_OK){
        rollback(handle);
        freeMemory(datatype);        
        return ERROR_MEMORY;
      }

      strcpy(statement, "DELETE FROM Value");
      strcat(statement, datatype);
      strcat(statement, " WHERE id = :id;");

      if(sqlite3_prepare_v2(handle->db, statement, -1, &ppStmt, pzTail) != SQLITE_OK){
        //printf("prepare: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(statement);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      } 
      freeMemory(statement);

      parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":id");
      if(parameterIndex == 0){
        //printf("bind parameter index: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_bind_int(ppStmt, parameterIndex, id) != SQLITE_OK){
        //printf("bind value: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      while(42){
        int retval = sqlite3_step(ppStmt);
        if(retval == SQLITE_DONE){
          break;
        }
        else if(retval == SQLITE_ERROR){
          //printf("fucking step1: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          return ERROR_DATABASE_INVALID;
        }
        else if(retval == SQLITE_MISUSE){
          //printf("fucking step2: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          return ERROR_DATABASE_INVALID;
        }
      }

      if(sqlite3_reset(ppStmt) != SQLITE_OK){
        //printf("reset: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_finalize(ppStmt) != SQLITE_OK){
        //printf("finalize: %s\n", sqlite3_errmsg(handle->db));
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      /* Delete from KeyInfo */
      if(requestMemory((void**)&statement, 36) != ERROR_OK){
        freeMemory(datatype);    
        rollback(handle);    
        return ERROR_MEMORY;
      }

      strcpy(statement, "DELETE FROM KeyInfo WHERE id = :id;");

      if(sqlite3_prepare_v2(handle->db, statement, -1, &ppStmt, pzTail) != SQLITE_OK){
        //printf("prepare: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(statement);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }
      freeMemory(statement);

      parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":id");
      if(parameterIndex == 0){
        //printf("bind parameter index: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_bind_int(ppStmt, parameterIndex, id) != SQLITE_OK){
        //printf("bind value: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      while(42){
        int retval = sqlite3_step(ppStmt);
        if(retval == SQLITE_DONE){
          break;
        }
        else if(retval == SQLITE_ERROR){
          //printf("fucking step1: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          return ERROR_DATABASE_INVALID;
        }
        else if(retval == SQLITE_MISUSE){
          //printf("fucking step2: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          return ERROR_DATABASE_INVALID;
        }
      }

      if(sqlite3_reset(ppStmt) != SQLITE_OK){
        //printf("reset: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_finalize(ppStmt) != SQLITE_OK){
        //printf("finalize: %s\n", sqlite3_errmsg(handle->db));
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

    /* Insert into Keyinfo */
      if(requestMemory((void**)&statement, 90) != ERROR_OK){
        freeMemory(datatype);  
        rollback(handle);      
        return ERROR_MEMORY;
      }

      strcpy(statement, "INSERT INTO KeyInfo(`id`, `domain`, `key`, `datatype`) VALUES (:id, :dom, :key, 'Int64');");

      if(sqlite3_prepare_v2(handle->db, statement, -1, &ppStmt, pzTail) != SQLITE_OK){
        //printf("prepare: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(statement);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      } 
      freeMemory(statement);

      parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":id");
      if(parameterIndex == 0){
        //printf("bind parameter index: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_bind_int(ppStmt, parameterIndex, id) != SQLITE_OK){
        //printf("bind value: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":dom");
      if(parameterIndex == 0){
        //printf("bind parameter domain: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_bind_text(ppStmt, parameterIndex, domain, -1, SQLITE_TRANSIENT) != SQLITE_OK){
        //printf("bind domain: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }
      
      parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":key");
      if(parameterIndex == 0){
        //printf("bind parameter key: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_bind_text(ppStmt, parameterIndex, key, -1, SQLITE_TRANSIENT) != SQLITE_OK){
        //printf("bind key: %s\n", sqlite3_errmsg(handle->db));  
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      while(42){
        int retval = sqlite3_step(ppStmt);
        if(retval == SQLITE_DONE){
          break;
        }
        else if(retval == SQLITE_ERROR){
          //printf("fucking step1: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          return ERROR_DATABASE_INVALID;
        }
        else if(retval == SQLITE_MISUSE){
          //printf("fucking step2: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          return ERROR_DATABASE_INVALID;
        }
      }

      if(sqlite3_reset(ppStmt) != SQLITE_OK){
        //printf("reset: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_finalize(ppStmt) != SQLITE_OK){
        //printf("finalize: %s\n", sqlite3_errmsg(handle->db));
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      /* Insert into ValueInt64 */
      if(requestMemory((void**)&statement, 57) != ERROR_OK){
        rollback(handle);
        freeMemory(datatype);        
        return ERROR_MEMORY;
      }

      strcpy(statement, "INSERT INTO ValueInt64(`id`, `value`) VALUES (:id,:val);");

      if(sqlite3_prepare_v2(handle->db, statement, -1, &ppStmt, pzTail) != SQLITE_OK){
        //printf("prepare: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(statement);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }
      freeMemory(statement);

      parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":id");
      if(parameterIndex == 0){
        //printf("bind parameter index: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }
        
      if(sqlite3_bind_int(ppStmt, parameterIndex, id) != SQLITE_OK){
        //printf("bind id: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      } 

      parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":val");
      if(parameterIndex == 0){
        //printf("bind parameter index: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_bind_int64(ppStmt, parameterIndex, (sqlite3_int64)value) != SQLITE_OK){
        //printf("bind value: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      while(42){
        int retval = sqlite3_step(ppStmt);
        if(retval == SQLITE_DONE){
          break;
        }
        else if(retval == SQLITE_ERROR){
          //printf("fucking step1: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          return ERROR_DATABASE_INVALID;
        }
        else if(retval == SQLITE_MISUSE){
          //printf("fucking step2: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          return ERROR_DATABASE_INVALID;
        }
      }

      if(sqlite3_reset(ppStmt) != SQLITE_OK){
        //printf("reset: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_finalize(ppStmt) != SQLITE_OK){
        //printf("finalize: %s\n", sqlite3_errmsg(handle->db));
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }
    }
    commit(handle);    
  }
  freeMemory(datatype);  
  return ERROR_OK;
}


int
database_get_double(database_handle_t* handle, const char* domain,
                    const char* key, double* value)
{
  if(handle == NULL || handle->db == NULL || handle->blobpath == NULL || domain == NULL || key == NULL || value == NULL || strlen(handle->blobpath) == 0 || strlen(domain) == 0 || strlen(key) == 0)
    return ERROR_INVALID_ARGUMENTS;
  
  char* statement = NULL;
  sqlite3_stmt *ppStmt = NULL;
  const char** pzTail = NULL;

  unsigned int sizeOfStmt = 197;
  unsigned int error = 0;
  error = requestMemory((void**)&statement, sizeOfStmt);
  if(error != ERROR_OK)
    return error;

  strcpy(statement, "SELECT ValueDouble.`value` as `value` FROM KeyInfo INNER JOIN ValueDouble ON KeyInfo.`id` = ValueDouble.`id`WHERE KeyInfo.`datatype` = 'Double' AND KeyInfo.`domain` = :dom AND KeyInfo.`key`= :key;");

  begin(handle);
  if(sqlite3_prepare_v2(handle->db, statement, -1, &ppStmt, pzTail) != SQLITE_OK){
    //printf("prepare: %s\n", sqlite3_errmsg(handle->db));
    sqlite3_finalize(ppStmt);
    rollback(handle);
    freeMemory(statement);
    return ERROR_DATABASE_INVALID;
  }
  freeMemory(statement);

  int parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":dom");
  if(parameterIndex == 0){
    //printf("bind parameter domain: %s\n", sqlite3_errmsg(handle->db));
    sqlite3_finalize(ppStmt);
    rollback(handle);
    return ERROR_DATABASE_INVALID;
  }

  if(sqlite3_bind_text(ppStmt, parameterIndex, domain, -1, SQLITE_TRANSIENT) != SQLITE_OK){
    //printf("bind domain: %s\n", sqlite3_errmsg(handle->db));
    sqlite3_finalize(ppStmt);
    rollback(handle);
    return ERROR_DATABASE_INVALID;
  }
  
  parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":key");
  if(parameterIndex == 0){
    //printf("bind parameter key: %s\n", sqlite3_errmsg(handle->db));
    sqlite3_finalize(ppStmt);
    rollback(handle);
    return ERROR_DATABASE_INVALID;
  }

  if(sqlite3_bind_text(ppStmt, parameterIndex, key, -1, SQLITE_TRANSIENT) != SQLITE_OK){
    //printf("bind key: %s\n", sqlite3_errmsg(handle->db));  
    sqlite3_finalize(ppStmt);
    rollback(handle);
    return ERROR_DATABASE_INVALID;
  }

  while (42){
      int retval = sqlite3_step(ppStmt);

      if(retval == SQLITE_ROW){     
        if(sqlite3_column_type(ppStmt, 0) != SQLITE_FLOAT){
          sqlite3_finalize(ppStmt);
          rollback(handle);
          return ERROR_DATABASE_TYPE_MISMATCH;
        }      
        *value = sqlite3_column_double(ppStmt, 0);
        break;
      }
      else if(retval == SQLITE_DONE){
        //printf("no such key2");
        sqlite3_finalize(ppStmt);
        rollback(handle);
        return ERROR_DATABASE_NO_SUCH_KEY;
      }
      else {
        //printf("step: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        return ERROR_DATABASE_INVALID;
      } 
  }

  if(sqlite3_reset(ppStmt) != SQLITE_OK){
    //printf("reset: %s\n", sqlite3_errmsg(handle->db));
    sqlite3_finalize(ppStmt);
    rollback(handle);
    return ERROR_DATABASE_INVALID;
  }

  if(sqlite3_finalize(ppStmt) != SQLITE_OK){
    //printf("finalize: %s\n", sqlite3_errmsg(handle->db));
    rollback(handle);
    return ERROR_DATABASE_INVALID;
  }
  commit(handle);

  return ERROR_OK;
}


int
database_set_double(database_handle_t* handle, const char* domain,
                    const char* key, double value)
{
  /* Input checks */
  if(handle == NULL || handle->db == NULL || handle->blobpath == NULL || domain  == NULL || key == NULL || strlen(handle->blobpath) == 0 || strlen(domain) == 0 || strlen(key) == 0 || isnan(value))
    return ERROR_INVALID_ARGUMENTS;

  /* Some variables */
  char* statement = NULL;
  sqlite3_stmt *ppStmt = NULL;
  const char** pzTail = NULL;

  /* Check if already existing */
  char* datatype = NULL;
  unsigned int id = 0;

  if(requestMemory((void**)&datatype, 7) != ERROR_OK)
    return ERROR_MEMORY;

  if(requestMemory((void**)&statement, 69) != ERROR_OK)
    return ERROR_MEMORY;

  strcpy(statement, "SELECT id, datatype FROM KeyInfo WHERE domain = :dom and key = :key;");

  begin(handle);
  if(sqlite3_prepare_v2(handle->db, statement, -1, &ppStmt, pzTail) != SQLITE_OK){
    //printf("prepare1: %s\n", sqlite3_errmsg(handle->db));
    sqlite3_finalize(ppStmt);
    rollback(handle);
    freeMemory(statement);
    freeMemory(datatype);  
    return ERROR_DATABASE_INVALID;
  }
  freeMemory(statement);

  int parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":dom");
  if(parameterIndex == 0){
    //printf("bind parameter domain: %s\n", sqlite3_errmsg(handle->db));
    sqlite3_finalize(ppStmt);
    rollback(handle);
    freeMemory(datatype);  
    return ERROR_DATABASE_INVALID;
  }

  if(sqlite3_bind_text(ppStmt, parameterIndex, domain, -1, SQLITE_TRANSIENT) != SQLITE_OK){
    //printf("bind domain: %s\n", sqlite3_errmsg(handle->db));
    sqlite3_finalize(ppStmt);
    rollback(handle);
    freeMemory(datatype);  
    return ERROR_DATABASE_INVALID;
  }
  
  parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":key");
  if(parameterIndex == 0){
    //printf("bind parameter key: %s\n", sqlite3_errmsg(handle->db));
    sqlite3_finalize(ppStmt);
    rollback(handle);
    freeMemory(datatype);  
    return ERROR_DATABASE_INVALID;
  }

  if(sqlite3_bind_text(ppStmt, parameterIndex, key, -1, SQLITE_TRANSIENT) != SQLITE_OK){
    //printf("bind key: %s\n", sqlite3_errmsg(handle->db));  
    sqlite3_finalize(ppStmt);
    rollback(handle);
    freeMemory(datatype);  
    return ERROR_DATABASE_INVALID;
  }

  while (42){
      int retval = sqlite3_step(ppStmt);

      if(retval == SQLITE_ROW){
        if(sqlite3_column_type(ppStmt, 0) != SQLITE_INTEGER){
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          return ERROR_DATABASE_TYPE_MISMATCH;
        }
        id = sqlite3_column_int(ppStmt, 0);    
        strcpy(datatype, (char*)sqlite3_column_text(ppStmt, 1)); 
        break;
      }
      else if(retval == SQLITE_DONE){
        freeMemory(datatype); 
        datatype = NULL;
        break;
      }
      else {
        //printf("step: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      } 
  }

  if(sqlite3_reset(ppStmt) != SQLITE_OK){
    //printf("reset: %s\n", sqlite3_errmsg(handle->db));
    sqlite3_finalize(ppStmt);
    rollback(handle);
    freeMemory(datatype);  
    return ERROR_DATABASE_INVALID;
  }

  if(sqlite3_finalize(ppStmt) != SQLITE_OK){
    //printf("finalize: %s\n", sqlite3_errmsg(handle->db));
    rollback(handle);
    freeMemory(datatype);  
    return ERROR_DATABASE_INVALID;
  }
  commit(handle);
  begin(handle);

  /* key doesn't exist */
  if(datatype == NULL){
    /* Insert into Keyinfo */
      if(requestMemory((void**)&statement, 80) != ERROR_OK){
        rollback(handle);
        freeMemory(datatype);
        return ERROR_MEMORY;
      }

      strcpy(statement, "INSERT INTO KeyInfo(`domain`, `key`, `datatype`) VALUES (:dom, :key, 'Double');");

      if(sqlite3_prepare_v2(handle->db, statement, -1, &ppStmt, pzTail) != SQLITE_OK){
        //printf("prepare2: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(statement);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }
      freeMemory(statement);

      parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":dom");
      if(parameterIndex == 0){
        //printf("bind parameter domain: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_bind_text(ppStmt, parameterIndex, domain, -1, SQLITE_TRANSIENT) != SQLITE_OK){
        //printf("bind domain: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }
      
      parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":key");
      if(parameterIndex == 0){
        //printf("bind parameter key: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_bind_text(ppStmt, parameterIndex, key, -1, SQLITE_TRANSIENT) != SQLITE_OK){
        //printf("bind key: %s\n", sqlite3_errmsg(handle->db));  
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      while(42){
        int retval = sqlite3_step(ppStmt);
        if(retval == SQLITE_DONE){
          break;
        }
        else if(retval == SQLITE_ERROR){
          //printf("fucking step1: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          return ERROR_DATABASE_INVALID;
        }
        else if(retval == SQLITE_MISUSE){
          //printf("fucking step2: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          return ERROR_DATABASE_INVALID;
        }
      }

      if(sqlite3_reset(ppStmt) != SQLITE_OK){
        //printf("reset: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_finalize(ppStmt) != SQLITE_OK){
        //printf("finalize: %s\n", sqlite3_errmsg(handle->db));
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      /* Insert into ValueDouble */
      if(requestMemory((void**)&statement, 58) != ERROR_OK){
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_MEMORY; 
      }

      strcpy(statement, "INSERT INTO ValueDouble(`id`, `value`) VALUES (:id,:val);");

      if(sqlite3_prepare_v2(handle->db, statement, -1, &ppStmt, pzTail) != SQLITE_OK){
        //printf("prepare3: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(statement);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }
      freeMemory(statement);

      parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":id");
      if(parameterIndex == 0){
        //printf("bind parameter index: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }
        
      if(sqlite3_bind_int64(ppStmt, parameterIndex, sqlite3_last_insert_rowid(handle->db)) != SQLITE_OK){
        //printf("bind id: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      } 

      parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":val");
      if(parameterIndex == 0){
        //printf("bind parameter index: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_bind_double(ppStmt, parameterIndex, value) != SQLITE_OK){
        //printf("bind value: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      while(42){
        int retval = sqlite3_step(ppStmt);
        if(retval == SQLITE_DONE){
          break;
        }
        else if(retval == SQLITE_ERROR){
          //printf("fucking step1: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          return ERROR_DATABASE_INVALID;
        }
        else if(retval == SQLITE_MISUSE){
          //printf("fucking step2: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          return ERROR_DATABASE_INVALID;
        }
      }

      if(sqlite3_reset(ppStmt) != SQLITE_OK){
        //printf("reset: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_finalize(ppStmt) != SQLITE_OK){
        //printf("finalize: %s\n", sqlite3_errmsg(handle->db));
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }
    commit(handle);
  }
  else{ /* key already exist */    
    /* Datatype is the same - just update value */
    if(!strcmp(datatype, "Double")){  

      commit(handle);
      //remove referenced blob file
      if(!strcmp(datatype, "Blob")){
        int ret = removeReferencedBlobFile(handle, domain, key);
        if(ret != ERROR_OK)
          return ret;
      }
      begin(handle);

      /* Update ValueDouble */
      if(requestMemory((void**)&statement, 52) != ERROR_OK){
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_MEMORY;
      }

      strcpy(statement, "UPDATE ValueDouble SET value = :val WHERE id = :id;");

      if(sqlite3_prepare_v2(handle->db, statement, -1, &ppStmt, pzTail) != SQLITE_OK){
        //printf("prepare4: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(statement);
        freeMemory(datatype);   
        return ERROR_DATABASE_INVALID;
      }
      freeMemory(statement);

      parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":val");
      if(parameterIndex == 0){
        //printf("bind parameter index: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }
        
      if(sqlite3_bind_double(ppStmt, parameterIndex, value) != SQLITE_OK){
        //printf("bind value: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      } 

      parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":id");
      if(parameterIndex == 0){
        //printf("bind parameter index: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_bind_int(ppStmt, parameterIndex, id) != SQLITE_OK){
        //printf("bind id: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      while(42){
        int retval = sqlite3_step(ppStmt);
        if(retval == SQLITE_DONE){
          break;
        }
        else if(retval == SQLITE_ERROR){
          //printf("fucking step1: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          return ERROR_DATABASE_INVALID;
        }
        else if(retval == SQLITE_MISUSE){
          //printf("fucking step2: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          return ERROR_DATABASE_INVALID;
        }
      }

      if(sqlite3_reset(ppStmt) != SQLITE_OK){
        //printf("reset: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_finalize(ppStmt) != SQLITE_OK){
        //printf("finalize: %s\n", sqlite3_errmsg(handle->db));
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }
    }
    else{ /* Different datatype - delete, update and insert */
      
      /* Delete from ValueXtable */
      if(requestMemory((void**)&statement, 40) != ERROR_OK){
        rollback(handle);
        freeMemory(datatype);        
        return ERROR_MEMORY;
      }

      strcpy(statement, "DELETE FROM Value");
      strcat(statement, datatype);
      strcat(statement, " WHERE id = :id;");

      if(sqlite3_prepare_v2(handle->db, statement, -1, &ppStmt, pzTail) != SQLITE_OK){
        //printf("prepare5: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(statement);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      } 
      freeMemory(statement);

      parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":id");
      if(parameterIndex == 0){
        //printf("bind parameter index: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_bind_int(ppStmt, parameterIndex, id) != SQLITE_OK){
        //printf("bind id: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      while(42){
        int retval = sqlite3_step(ppStmt);
        if(retval == SQLITE_DONE){
          break;
        }
        else if(retval == SQLITE_ERROR){
          //printf("fucking step1: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          return ERROR_DATABASE_INVALID;
        }
        else if(retval == SQLITE_MISUSE){
          //printf("fucking step2: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          return ERROR_DATABASE_INVALID;
        }
      }

      if(sqlite3_reset(ppStmt) != SQLITE_OK){
        //printf("reset: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_finalize(ppStmt) != SQLITE_OK){
        //printf("finalize: %s\n", sqlite3_errmsg(handle->db));
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      /* Delete from KeyInfo */
      if(requestMemory((void**)&statement, 36) != ERROR_OK){
        rollback(handle);
        freeMemory(datatype);        
        return ERROR_MEMORY;
      }

      strcpy(statement, "DELETE FROM KeyInfo WHERE id = :id;");

      if(sqlite3_prepare_v2(handle->db, statement, -1, &ppStmt, pzTail) != SQLITE_OK){
        //printf("prepare6: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(statement);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }
      freeMemory(statement);

      parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":id");
      if(parameterIndex == 0){
        //printf("bind parameter index: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_bind_int(ppStmt, parameterIndex, id) != SQLITE_OK){
        //printf("bind id: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      while(42){
        int retval = sqlite3_step(ppStmt);
        if(retval == SQLITE_DONE){
          break;
        }
        else if(retval == SQLITE_ERROR){
          //printf("fucking step1: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          return ERROR_DATABASE_INVALID;
        }
        else if(retval == SQLITE_MISUSE){
          //printf("fucking step2: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          return ERROR_DATABASE_INVALID;
        }
      }

      if(sqlite3_reset(ppStmt) != SQLITE_OK){
        //printf("reset: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_finalize(ppStmt) != SQLITE_OK){
        //printf("finalize: %s\n", sqlite3_errmsg(handle->db));
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

    /* Insert into Keyinfo */
      if(requestMemory((void**)&statement, 91) != ERROR_OK){
        freeMemory(datatype);        
        rollback(handle);
        return ERROR_MEMORY;
      }

      strcpy(statement, "INSERT INTO KeyInfo(`id`, `domain`, `key`, `datatype`) VALUES (:id, :dom, :key, 'Double');");

      if(sqlite3_prepare_v2(handle->db, statement, -1, &ppStmt, pzTail) != SQLITE_OK){
        //printf("prepare7: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(statement);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      } 
      freeMemory(statement);

      parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":id");
      if(parameterIndex == 0){
        //printf("bind parameter index: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_bind_int(ppStmt, parameterIndex, id) != SQLITE_OK){
        //printf("bind id: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":dom");
      if(parameterIndex == 0){
        //printf("bind parameter domain: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_bind_text(ppStmt, parameterIndex, domain, -1, SQLITE_TRANSIENT) != SQLITE_OK){
        //printf("bind domain: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }
      
      parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":key");
      if(parameterIndex == 0){
        //printf("bind parameter key: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_bind_text(ppStmt, parameterIndex, key, -1, SQLITE_TRANSIENT) != SQLITE_OK){
        //printf("bind key: %s\n", sqlite3_errmsg(handle->db));  
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      while(42){
        int retval = sqlite3_step(ppStmt);
        if(retval == SQLITE_DONE){
          break;
        }
        else if(retval == SQLITE_ERROR){
          //printf("fucking step1: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          return ERROR_DATABASE_INVALID;
        }
        else if(retval == SQLITE_MISUSE){
          //printf("fucking step2: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          return ERROR_DATABASE_INVALID;
        }
      }

      if(sqlite3_reset(ppStmt) != SQLITE_OK){
        //printf("reset: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_finalize(ppStmt) != SQLITE_OK){
        //printf("finalize: %s\n", sqlite3_errmsg(handle->db));
        freeMemory(datatype);  
        rollback(handle);
        return ERROR_DATABASE_INVALID;
      }

      /* Insert into ValueDouble */
      if(requestMemory((void**)&statement, 58) != ERROR_OK){
        freeMemory(datatype);        
        rollback(handle);
        return ERROR_MEMORY;
      }

      strcpy(statement, "INSERT INTO ValueDouble(`id`, `value`) VALUES (:id,:val);");

      if(sqlite3_prepare_v2(handle->db, statement, -1, &ppStmt, pzTail) != SQLITE_OK){
        //printf("prepare8: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(statement);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }
      freeMemory(statement);

      parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":id");
      if(parameterIndex == 0){
        //printf("bind parameter index: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }
        
      if(sqlite3_bind_int(ppStmt, parameterIndex, id) != SQLITE_OK){
        //printf("bind id: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      } 

      parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":val");
      if(parameterIndex == 0){
        //printf("bind parameter index: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_bind_double(ppStmt, parameterIndex, value) != SQLITE_OK){
        //printf("bind value: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      while(42){
        int retval = sqlite3_step(ppStmt);
        if(retval == SQLITE_DONE){
          break;
        }
        else if(retval == SQLITE_ERROR){
          //printf("fucking step1: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          return ERROR_DATABASE_INVALID;
        }
        else if(retval == SQLITE_MISUSE){
          //printf("fucking step2: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          return ERROR_DATABASE_INVALID;
        }
      }

      if(sqlite3_reset(ppStmt) != SQLITE_OK){
        //printf("reset: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_finalize(ppStmt) != SQLITE_OK){
        //printf("finalize: %s\n", sqlite3_errmsg(handle->db));
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }
    }    
    commit(handle);
  }
  freeMemory(datatype);  
  return ERROR_OK;
}


int
database_get_string(database_handle_t* handle, const char* domain,
                    const char* key, char** value)
{
 if(handle == NULL || handle->db == NULL || handle->blobpath == NULL || domain == NULL || key == NULL || value == NULL || strlen(handle->blobpath) == 0 || strlen(domain) == 0 || strlen(key) == 0)
    return ERROR_INVALID_ARGUMENTS;
  
  char* statement = NULL;
  sqlite3_stmt *ppStmt = NULL;
  const char** pzTail = NULL;

  unsigned int sizeOfStmt = 197;
  if(requestMemory((void**)&statement, sizeOfStmt) != ERROR_OK)
    return ERROR_MEMORY;

  strcpy(statement, "SELECT ValueString.`value` as `value` FROM KeyInfo INNER JOIN ValueString ON KeyInfo.`id` = ValueString.`id`WHERE KeyInfo.`datatype` = 'String' AND KeyInfo.`domain` = :dom AND KeyInfo.`key`= :key;");

  begin(handle);
  if(sqlite3_prepare_v2(handle->db, statement, -1, &ppStmt, pzTail) != SQLITE_OK){
    //printf("prepare: %s\n", sqlite3_errmsg(handle->db));
    sqlite3_finalize(ppStmt);
    rollback(handle);
    freeMemory(statement);
    return ERROR_DATABASE_INVALID;
  }
  freeMemory(statement);

  int parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":dom");
  if(parameterIndex == 0){
    //printf("bind parameter domain: %s\n", sqlite3_errmsg(handle->db));
    sqlite3_finalize(ppStmt);
    rollback(handle);
    return ERROR_DATABASE_INVALID;
  }

  if(sqlite3_bind_text(ppStmt, parameterIndex, domain, -1, SQLITE_TRANSIENT) != SQLITE_OK){
    //printf("bind domain: %s\n", sqlite3_errmsg(handle->db));
    sqlite3_finalize(ppStmt);
    rollback(handle);
    return ERROR_DATABASE_INVALID;
  }
  
  parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":key");
  if(parameterIndex == 0){
    //printf("bind parameter key: %s\n", sqlite3_errmsg(handle->db));
    sqlite3_finalize(ppStmt);
    rollback(handle);
    return ERROR_DATABASE_INVALID;
  }

  if(sqlite3_bind_text(ppStmt, parameterIndex, key, -1, SQLITE_TRANSIENT) != SQLITE_OK){
    //printf("bind key: %s\n", sqlite3_errmsg(handle->db));  
    sqlite3_finalize(ppStmt);
    rollback(handle);
    return ERROR_DATABASE_INVALID;
  }

  while (42){
      int retval = sqlite3_step(ppStmt);

      if(retval == SQLITE_ROW){   
        if(sqlite3_column_type(ppStmt, 0) != SQLITE3_TEXT){
          sqlite3_finalize(ppStmt);
          rollback(handle);
          return ERROR_DATABASE_TYPE_MISMATCH;
        } 
     
        char * dbentry = NULL;
        dbentry = (char*)sqlite3_column_text(ppStmt, 0); 
        if(requestMemory((void**)&*value, strlen(dbentry)+1) != ERROR_OK){
          sqlite3_finalize(ppStmt);
          rollback(handle);
          return ERROR_MEMORY;
        }
        memcpy(*value, dbentry, strlen(dbentry));
        (*value)[strlen(dbentry)] = '\0';
        break;
      }
      else if(retval == SQLITE_DONE){
        sqlite3_finalize(ppStmt);
        rollback(handle);
        return ERROR_DATABASE_NO_SUCH_KEY;
      }
      else {
        //printf("step: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        return ERROR_DATABASE_INVALID;
      } 
  }

  if(sqlite3_reset(ppStmt) != SQLITE_OK){
    //printf("reset: %s\n", sqlite3_errmsg(handle->db));
    sqlite3_finalize(ppStmt);
    rollback(handle);
    return ERROR_DATABASE_INVALID;
  }

  if(sqlite3_finalize(ppStmt) != SQLITE_OK){
    //printf("finalize: %s\n", sqlite3_errmsg(handle->db));
    rollback(handle);
    return ERROR_DATABASE_INVALID;
  }

  commit(handle);
  return ERROR_OK;
}


int
database_set_string(database_handle_t* handle, const char* domain,
                    const char* key, const char* value)
{
  /* Input checks */
  if(handle == NULL || handle->db == NULL || handle->blobpath == NULL || domain  == NULL || key == NULL || strlen(handle->blobpath) == 0 || strlen(domain) == 0 || strlen(key) == 0 || value == NULL)
    return ERROR_INVALID_ARGUMENTS;

  /* Some variables */
  char* statement = NULL;
  sqlite3_stmt *ppStmt = NULL;
  const char** pzTail = NULL;
  unsigned int error = 0;

  /* Check if already existing */
  char* datatype = NULL;
  unsigned int id = 0;
  if(requestMemory((void**)&datatype, 7) != ERROR_OK)
    return ERROR_MEMORY;
  if(requestMemory((void**)&statement, 69) != ERROR_OK)
    return ERROR_MEMORY;

  strcpy(statement, "SELECT id, datatype FROM KeyInfo WHERE domain = :dom and key = :key;");

  begin(handle);
  if(sqlite3_prepare_v2(handle->db, statement, -1, &ppStmt, pzTail) != SQLITE_OK){
    //printf("prepare1: %s\n", sqlite3_errmsg(handle->db));
    sqlite3_finalize(ppStmt);
    rollback(handle);
    freeMemory(statement);
    freeMemory(datatype);  
    return ERROR_DATABASE_INVALID;
  }
  freeMemory(statement);

  int parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":dom");
  if(parameterIndex == 0){
    //printf("bind parameter domain: %s\n", sqlite3_errmsg(handle->db));
    sqlite3_finalize(ppStmt);
    rollback(handle);
    freeMemory(datatype);  
    return ERROR_DATABASE_INVALID;
  }

  if(sqlite3_bind_text(ppStmt, parameterIndex, domain, -1, SQLITE_TRANSIENT) != SQLITE_OK){
    //printf("bind domain: %s\n", sqlite3_errmsg(handle->db));
    sqlite3_finalize(ppStmt);
    rollback(handle);
    freeMemory(datatype);  
    return ERROR_DATABASE_INVALID;
  }
  
  parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":key");
  if(parameterIndex == 0){
    //printf("bind parameter key: %s\n", sqlite3_errmsg(handle->db));
    sqlite3_finalize(ppStmt);
    rollback(handle);
    freeMemory(datatype);  
    return ERROR_DATABASE_INVALID;
  }

  if(sqlite3_bind_text(ppStmt, parameterIndex, key, -1, SQLITE_TRANSIENT) != SQLITE_OK){
    //printf("bind key: %s\n", sqlite3_errmsg(handle->db));  
    sqlite3_finalize(ppStmt);
    rollback(handle);
    freeMemory(datatype);  
    return ERROR_DATABASE_INVALID;
  }

  while (42){
      int retval = sqlite3_step(ppStmt);

      if(retval == SQLITE_ROW){
        if(sqlite3_column_type(ppStmt, 0) != SQLITE_INTEGER){
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          return ERROR_DATABASE_TYPE_MISMATCH;
        }
        id = sqlite3_column_int(ppStmt, 0);    
        strcpy(datatype, (char*)sqlite3_column_text(ppStmt, 1)); 
        break;
      }
      else if(retval == SQLITE_DONE){
        freeMemory(datatype); 
        datatype = NULL;
        break;
      }
      else {
        //printf("step: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      } 
  }

  if(sqlite3_reset(ppStmt) != SQLITE_OK){
    //printf("reset: %s\n", sqlite3_errmsg(handle->db));
    sqlite3_finalize(ppStmt);
    rollback(handle);
    freeMemory(datatype);  
    return ERROR_DATABASE_INVALID;
  }

  if(sqlite3_finalize(ppStmt) != SQLITE_OK){
    //printf("finalize: %s\n", sqlite3_errmsg(handle->db));
    rollback(handle);
    freeMemory(datatype);  
    return ERROR_DATABASE_INVALID;
  }
  commit(handle);
  begin(handle);
  /* key doesn't exist */
  if(datatype == NULL){
    /* Insert into Keyinfo */
      if(requestMemory((void**)&statement, 80) != ERROR_OK){
        rollback(handle);
        freeMemory(datatype);
        return ERROR_MEMORY;
      }

      strcpy(statement, "INSERT INTO KeyInfo(`domain`, `key`, `datatype`) VALUES (:dom, :key, 'String');");

      if(sqlite3_prepare_v2(handle->db, statement, -1, &ppStmt, pzTail) != SQLITE_OK){
        //printf("prepare2: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(statement);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }
      freeMemory(statement);

      parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":dom");
      if(parameterIndex == 0){
        //printf("bind parameter domain: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_bind_text(ppStmt, parameterIndex, domain, -1, SQLITE_TRANSIENT) != SQLITE_OK){
        //printf("bind domain: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }
      
      parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":key");
      if(parameterIndex == 0){
        //printf("bind parameter key: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_bind_text(ppStmt, parameterIndex, key, -1, SQLITE_TRANSIENT) != SQLITE_OK){
        //printf("bind key: %s\n", sqlite3_errmsg(handle->db));  
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      while(42){
        int retval = sqlite3_step(ppStmt);
        if(retval == SQLITE_DONE){
          break;
        }
        else if(retval == SQLITE_ERROR){
          //printf("fucking step1: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          return ERROR_DATABASE_INVALID;
        }
        else if(retval == SQLITE_MISUSE){
          //printf("fucking step2: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          return ERROR_DATABASE_INVALID;
        }
      }

      if(sqlite3_reset(ppStmt) != SQLITE_OK){
        //printf("reset: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_finalize(ppStmt) != SQLITE_OK){
        //printf("finalize: %s\n", sqlite3_errmsg(handle->db));
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      /* Insert into ValueString */
      if(requestMemory((void**)&statement, 58) != ERROR_OK){
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_MEMORY; 
      }

      strcpy(statement, "INSERT INTO ValueString(`id`, `value`) VALUES (:id,:val);");

      if(sqlite3_prepare_v2(handle->db, statement, -1, &ppStmt, pzTail) != SQLITE_OK){
        //printf("prepare3: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(statement);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }
      freeMemory(statement);

      parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":id");
      if(parameterIndex == 0){
        //printf("bind parameter index: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }
        
      if(sqlite3_bind_int64(ppStmt, parameterIndex, sqlite3_last_insert_rowid(handle->db)) != SQLITE_OK){
        //printf("bind id: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      } 

      parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":val");
      if(parameterIndex == 0){
        //printf("bind parameter index: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_bind_text(ppStmt, parameterIndex, value, -1, SQLITE_TRANSIENT) != SQLITE_OK){
        //printf("bind value: %s\n", sqlite3_errmsg(handle->db));  
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      while(42){
        int retval = sqlite3_step(ppStmt);
        if(retval == SQLITE_DONE){
          break;
        }
        else if(retval == SQLITE_ERROR){
          //printf("fucking step1: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          return ERROR_DATABASE_INVALID;
        }
        else if(retval == SQLITE_MISUSE){
          //printf("fucking step2: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          return ERROR_DATABASE_INVALID;
        }
      }

      if(sqlite3_reset(ppStmt) != SQLITE_OK){
        //printf("reset: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_finalize(ppStmt) != SQLITE_OK){
        //printf("finalize: %s\n", sqlite3_errmsg(handle->db));
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }
     commit(handle);
  }
  else{ /* key already exist */    
    /* Datatype is the same - just update value */
    if(!strcmp(datatype, "String")){  

      commit(handle);
      //remove referenced blob file
      if(!strcmp(datatype, "Blob")){
        int ret = removeReferencedBlobFile(handle, domain, key);
        if(ret != ERROR_OK)
          return ret;
      }
      begin(handle);

      /* Update ValueDouble */
      if(requestMemory((void**)&statement, 52) != ERROR_OK){
        rollback(handle);
        freeMemory(datatype);  
        return error;
      }

      strcpy(statement, "UPDATE ValueString SET value = :val WHERE id = :id;");

      if(sqlite3_prepare_v2(handle->db, statement, -1, &ppStmt, pzTail) != SQLITE_OK){
        //printf("prepare4: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(statement);
        freeMemory(datatype);   
        return ERROR_DATABASE_INVALID;
      }
      freeMemory(statement);

      parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":val");
      if(parameterIndex == 0){
        //printf("bind parameter index: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }
        
      if(sqlite3_bind_text(ppStmt, parameterIndex, value, -1, SQLITE_TRANSIENT) != SQLITE_OK){
        //printf("bind value: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);  
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":id");
      if(parameterIndex == 0){
        //printf("bind parameter index: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_bind_int(ppStmt, parameterIndex, id) != SQLITE_OK){
        //printf("bind id: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      while(42){
        int retval = sqlite3_step(ppStmt);
        if(retval == SQLITE_DONE){
          break;
        }
        else if(retval == SQLITE_ERROR){
          //printf("fucking step1: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          return ERROR_DATABASE_INVALID;
        }
        else if(retval == SQLITE_MISUSE){
          //printf("fucking step2: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          return ERROR_DATABASE_INVALID;
        }
      }

      if(sqlite3_reset(ppStmt) != SQLITE_OK){
        //printf("reset: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_finalize(ppStmt) != SQLITE_OK){
        //printf("finalize: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }
    }
    else{ /* Different datatype - delete, update and insert */
      
      /* Delete from ValueXtable */
      if(requestMemory((void**)&statement, 40) != ERROR_OK){
        rollback(handle);
        freeMemory(datatype);        
        return ERROR_MEMORY;
      }

      strcpy(statement, "DELETE FROM Value");
      strcat(statement, datatype);
      strcat(statement, " WHERE id = :id;");

      if(sqlite3_prepare_v2(handle->db, statement, -1, &ppStmt, pzTail) != SQLITE_OK){
        //printf("prepare5: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(statement);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      } 
      freeMemory(statement);

      parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":id");
      if(parameterIndex == 0){
        //printf("bind parameter index: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_bind_int(ppStmt, parameterIndex, id) != SQLITE_OK){
        //printf("bind id: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      while(42){
        int retval = sqlite3_step(ppStmt);
        if(retval == SQLITE_DONE){
          break;
        }
        else if(retval == SQLITE_ERROR){
          //printf("fucking step1: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          return ERROR_DATABASE_INVALID;
        }
        else if(retval == SQLITE_MISUSE){
          //printf("fucking step2: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          return ERROR_DATABASE_INVALID;
        }
      }

      if(sqlite3_reset(ppStmt) != SQLITE_OK){
        //printf("reset: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_finalize(ppStmt) != SQLITE_OK){
        //printf("finalize: %s\n", sqlite3_errmsg(handle->db));
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      /* Delete from KeyInfo */
      if(requestMemory((void**)&statement, 36) != ERROR_OK){
        rollback(handle);
        freeMemory(datatype);        
        return ERROR_MEMORY;
      }

      strcpy(statement, "DELETE FROM KeyInfo WHERE id = :id;");

      if(sqlite3_prepare_v2(handle->db, statement, -1, &ppStmt, pzTail) != SQLITE_OK){
        //printf("prepare6: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(statement);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }
      freeMemory(statement);

      parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":id");
      if(parameterIndex == 0){
        //printf("bind parameter index: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_bind_int(ppStmt, parameterIndex, id) != SQLITE_OK){
        //printf("bind id: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      while(42){
        int retval = sqlite3_step(ppStmt);
        if(retval == SQLITE_DONE){
          break;
        }
        else if(retval == SQLITE_ERROR){
          //printf("fucking step1: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          return ERROR_DATABASE_INVALID;
        }
        else if(retval == SQLITE_MISUSE){
          //printf("fucking step2: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          return ERROR_DATABASE_INVALID;
        }
      }

      if(sqlite3_reset(ppStmt) != SQLITE_OK){
        //printf("reset: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_finalize(ppStmt) != SQLITE_OK){
        //printf("finalize: %s\n", sqlite3_errmsg(handle->db));
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

    /* Insert into Keyinfo */
      if(requestMemory((void**)&statement, 91) != ERROR_OK){
        rollback(handle);
        freeMemory(datatype);        
        return ERROR_MEMORY;
      }

      strcpy(statement, "INSERT INTO KeyInfo(`id`, `domain`, `key`, `datatype`) VALUES (:id, :dom, :key, 'String');");

      if(sqlite3_prepare_v2(handle->db, statement, -1, &ppStmt, pzTail) != SQLITE_OK){
        //printf("prepare7: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(statement);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      } 
      freeMemory(statement);

      parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":id");
      if(parameterIndex == 0){
        //printf("bind parameter index: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_bind_int(ppStmt, parameterIndex, id) != SQLITE_OK){
        //printf("bind id: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":dom");
      if(parameterIndex == 0){
        //printf("bind parameter domain: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_bind_text(ppStmt, parameterIndex, domain, -1, SQLITE_TRANSIENT) != SQLITE_OK){
        //printf("bind domain: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }
      
      parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":key");
      if(parameterIndex == 0){
        //printf("bind parameter key: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_bind_text(ppStmt, parameterIndex, key, -1, SQLITE_TRANSIENT) != SQLITE_OK){
        //printf("bind key: %s\n", sqlite3_errmsg(handle->db));  
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      while(42){
        int retval = sqlite3_step(ppStmt);
        if(retval == SQLITE_DONE){
          break;
        }
        else if(retval == SQLITE_ERROR){
          //printf("fucking step1: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          return ERROR_DATABASE_INVALID;
        }
        else if(retval == SQLITE_MISUSE){
          //printf("fucking step2: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          return ERROR_DATABASE_INVALID;
        }
      }

      if(sqlite3_reset(ppStmt) != SQLITE_OK){
        //printf("reset: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_finalize(ppStmt) != SQLITE_OK){
        //printf("finalize: %s\n", sqlite3_errmsg(handle->db));
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      /* Insert into ValueString */
      if(requestMemory((void**)&statement, 58) != ERROR_OK){
        rollback(handle);
        freeMemory(datatype);        
        return ERROR_MEMORY;
      }

      strcpy(statement, "INSERT INTO ValueString(`id`, `value`) VALUES (:id,:val);");

      if(sqlite3_prepare_v2(handle->db, statement, -1, &ppStmt, pzTail) != SQLITE_OK){
        //printf("prepare8: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(statement);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }
      freeMemory(statement);

      parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":id");
      if(parameterIndex == 0){
        //printf("bind parameter index: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }
        
      if(sqlite3_bind_int(ppStmt, parameterIndex, id) != SQLITE_OK){
        //printf("bind id: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      } 

      parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":val");
      if(parameterIndex == 0){
        //printf("bind parameter index: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_bind_text(ppStmt, parameterIndex, value, -1, SQLITE_TRANSIENT) != SQLITE_OK){
        //printf("bind value: %s\n", sqlite3_errmsg(handle->db));  
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      while(42){
        int retval = sqlite3_step(ppStmt);
        if(retval == SQLITE_DONE){
          break;
        }
        else if(retval == SQLITE_ERROR){
          //printf("fucking step1: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          return ERROR_DATABASE_INVALID;
        }
        else if(retval == SQLITE_MISUSE){
          //printf("fucking step2: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          return ERROR_DATABASE_INVALID;
        }
      }

      if(sqlite3_reset(ppStmt) != SQLITE_OK){
        //printf("reset: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_finalize(ppStmt) != SQLITE_OK){
        //printf("finalize: %s\n", sqlite3_errmsg(handle->db));
        rollback(handle);
        freeMemory(datatype);  
        return ERROR_DATABASE_INVALID;
      }
    }    
    commit(handle);
  }
  freeMemory(datatype);  
  return ERROR_OK;
}


int
database_get_blob(database_handle_t* handle, const char* domain,
                  const char* key, unsigned char** value, size_t* size)
{
 if(handle == NULL || handle->db == NULL || handle->blobpath == NULL || domain == NULL || key == NULL || value == NULL || strlen(handle->blobpath) == 0 || strlen(domain) == 0 || strlen(key) == 0)
    return ERROR_INVALID_ARGUMENTS;
  
  char* statement = NULL;
  sqlite3_stmt *ppStmt = NULL;
  const char** pzTail = NULL;

  unsigned int sizeOfStmt = 188;

  if(requestMemory((void**)&statement, sizeOfStmt) != ERROR_OK)
    return ERROR_MEMORY;
  char* blobpath = NULL;

  strcpy(statement, "SELECT ValueBlob.`path` as `path` FROM KeyInfo INNER JOIN ValueBlob ON KeyInfo.`id` = ValueBlob.`id`WHERE KeyInfo.`datatype` = 'Blob' AND KeyInfo.`domain` = :dom AND KeyInfo.`key`= :key;");

  begin(handle);
  if(sqlite3_prepare_v2(handle->db, statement, -1, &ppStmt, pzTail) != SQLITE_OK){
    //printf("prepare: %s\n", sqlite3_errmsg(handle->db));
    sqlite3_finalize(ppStmt);
    rollback(handle);
    freeMemory(statement);
    return ERROR_DATABASE_INVALID;
  }
  freeMemory(statement);

  int parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":dom");
  if(parameterIndex == 0){
    //printf("bind parameter domain: %s\n", sqlite3_errmsg(handle->db));
    rollback(handle);
    sqlite3_finalize(ppStmt);
    return ERROR_DATABASE_INVALID;
  }

  if(sqlite3_bind_text(ppStmt, parameterIndex, domain, -1, SQLITE_TRANSIENT) != SQLITE_OK){
    //printf("bind domain: %s\n", sqlite3_errmsg(handle->db));
    rollback(handle);
    sqlite3_finalize(ppStmt);
    return ERROR_DATABASE_INVALID;
  }
  
  parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":key");
  if(parameterIndex == 0){
    //printf("bind parameter key: %s\n", sqlite3_errmsg(handle->db));
    rollback(handle);
    sqlite3_finalize(ppStmt);
    return ERROR_DATABASE_INVALID;
  }

  if(sqlite3_bind_text(ppStmt, parameterIndex, key, -1, SQLITE_TRANSIENT) != SQLITE_OK){
    //printf("bind key: %s\n", sqlite3_errmsg(handle->db));
    rollback(handle);  
    sqlite3_finalize(ppStmt);
    return ERROR_DATABASE_INVALID;
  }

  while (42){
      int retval = sqlite3_step(ppStmt);

      if(retval == SQLITE_ROW){   
        if(sqlite3_column_type(ppStmt, 0) != SQLITE3_TEXT){
          rollback(handle);
          sqlite3_finalize(ppStmt);
          return ERROR_DATABASE_TYPE_MISMATCH;
        }      

        char * dbentry = NULL;
        dbentry = (char*)sqlite3_column_text(ppStmt, 0); 
        if(requestMemory((void**)&blobpath, strlen(dbentry)+1) != ERROR_OK){
          rollback(handle);
          sqlite3_finalize(ppStmt);
          return ERROR_MEMORY;
        }
        memcpy(blobpath, dbentry, strlen(dbentry));
        blobpath[strlen(dbentry)] = '\0';
        break;
      }
      else if(retval == SQLITE_DONE){
        rollback(handle);
        sqlite3_finalize(ppStmt);
        return ERROR_DATABASE_NO_SUCH_KEY;
      }
      else {
        //printf("step: %s\n", sqlite3_errmsg(handle->db));
        rollback(handle);
        sqlite3_finalize(ppStmt);
        return ERROR_DATABASE_INVALID;
      } 
  }

  if(sqlite3_reset(ppStmt) != SQLITE_OK){
    //printf("reset: %s\n", sqlite3_errmsg(handle->db));
    rollback(handle);
    sqlite3_finalize(ppStmt);
    freeMemory(blobpath);
    return ERROR_DATABASE_INVALID;
  }

  if(sqlite3_finalize(ppStmt) != SQLITE_OK){
    //printf("finalize: %s\n", sqlite3_errmsg(handle->db));
    rollback(handle);
    freeMemory(blobpath);
    return ERROR_DATABASE_INVALID;
  }
  commit(handle);

  /* check for relative path */
  if(blobpath[0] == '/'){
    freeMemory(blobpath);
    return ERROR_DATABASE_INVALID;
  }

  char* pathtoblob = NULL;
  if(requestMemory((void**)&pathtoblob, strlen(handle->blobpath) + 1 + strlen(blobpath)+1) != ERROR_OK){
    freeMemory(blobpath);
    return ERROR_MEMORY;
  }
  strcpy(pathtoblob, handle->blobpath);
  strcat(pathtoblob, "/");
  strcat(pathtoblob, blobpath);
  pathtoblob[strlen(handle->blobpath) + 1 + strlen(blobpath)] = '\0';
  freeMemory(blobpath);



  /* check if path: is a regular file */
  struct stat sb;
  if(stat(pathtoblob, &sb) != 0){
    //printf("something gone wrong... stat\n");
    freeMemory(pathtoblob);
    return ERROR_DATABASE_INVALID;
  }
  if(!S_ISREG(sb.st_mode)){        /* check if regular file */
    //printf("regular file");
    freeMemory(pathtoblob);
    return ERROR_DATABASE_INVALID;
  }

  int error = check_blob_path(pathtoblob, handle->blobpath);
  if(error != ERROR_OK){
    freeMemory(pathtoblob);
    //printf("something gone wrong... check_blob_path\n");
    return error;
  }

  /* get blob */
  FILE *file = NULL;
  file = fopen(pathtoblob, "rb");
  if(file == NULL){
    //perror("the following error occured: ");
    freeMemory(pathtoblob);
    return ERROR_DATABASE_IO;
  }
  freeMemory(pathtoblob);
  
  fseek(file, 0, SEEK_END);
  *size = ftell(file);
  rewind(file);

  if(requestMemory((void**)&*value, sizeof(char)* *size) != ERROR_OK)
    return ERROR_MEMORY;

  size_t result = fread(*value, 1, *size, file);
  if(result != *size){
    //printf("read error\n");
    freeMemory(value);
    return ERROR_DATABASE_IO;
  }

  if(fclose(file) != 0){
    //printf("fclose error\n");
    return ERROR_DATABASE_IO;
  }

  return ERROR_OK;
}

int check_blob_path(const char* blobpath, const char* referencepath){

  /* check if path is inside blob directory */
  char *path = NULL;
  path = realpath(blobpath, NULL);
  if(path == NULL){
    //printf("error path not found!\n");
    freeMemory(path);
    return ERROR_DATABASE_INVALID;
  }
  if(strncmp(path, referencepath, strlen(referencepath)) != 0){
    freeMemory(path);
    return ERROR_DATABASE_INVALID;
  }
  freeMemory(path);
  
  return ERROR_OK;
}


int
database_set_blob(database_handle_t* handle, const char* domain,
                  const char* key, const unsigned char* value, size_t size)
{
/* Input checks */
  if(handle == NULL || handle->db == NULL || handle->blobpath == NULL || domain  == NULL || key == NULL || strlen(handle->blobpath) == 0 || strlen(domain) == 0 || strlen(key) == 0)
    return ERROR_INVALID_ARGUMENTS;

  /* Some variables */
  char* statement = NULL;
  sqlite3_stmt *ppStmt = NULL;
  const char** pzTail = NULL;

  char* domain_path = NULL;
  if(requestMemory((void**)&domain_path, strlen(domain)+1)){
    return ERROR_MEMORY;
  }
  unsigned int j = 0; 
  for(j = 0; j < strlen(domain); j++){
    if(domain[j] == ' ')
      domain_path[j] = '_';
    else if(domain[j] == '/')
      domain_path[j] = '_';
    else
      domain_path[j] = domain[j];
  }
  domain_path[strlen(domain)] = '\0';

  char* key_path = NULL;
  if(requestMemory((void**)&key_path, strlen(key)+1)){
    freeMemory(domain_path);
    return ERROR_MEMORY;
  }
  for(j = 0; j < strlen(key); j++){
    if(key[j] == ' ')
      key_path[j] = '_';
    else if(key[j] == '/')
      key_path[j] = '_';
    else
      key_path[j] = key[j];
  }
  key_path[strlen(key)] = '\0';

  char* path = NULL;
  if(requestMemory((void**)&path, strlen(domain_path)+1+strlen(key_path)+1) != ERROR_OK){
    freeMemory(domain_path);
    freeMemory(key_path);
    return ERROR_MEMORY;
  }

  strcpy(path, domain_path);
  strcat(path, "/");

  char* createdirectory_path = NULL;
  if(requestMemory((void**)&createdirectory_path, strlen(handle->blobpath) + 1 + strlen(domain_path)+1) != ERROR_OK){
    freeMemory(domain_path);
    freeMemory(key_path);
    freeMemory(path);
    return ERROR_MEMORY;
  }
  strcpy(createdirectory_path, handle->blobpath);
  strcat(createdirectory_path, "/");
  strcat(createdirectory_path, domain_path);
  createdirectory_path[strlen(handle->blobpath) + 1 +  strlen(domain_path)] = '\0';

  struct stat sb;
  if(stat(createdirectory_path, &sb) != 0){
    if(mkdir(createdirectory_path, 0777) != 0){
      freeMemory(createdirectory_path);
      freeMemory(domain_path);
      freeMemory(key_path);
      freeMemory(path); 
      return ERROR_DATABASE_IO;
    }
  }
  freeMemory(createdirectory_path);

  strcat(path, key_path);
  path[strlen(domain_path) + 1 + strlen(key_path)] = '\0';

  freeMemory(domain_path);
  freeMemory(key_path);

  /* check for relative path */
  if(path[0] == '/'){
    //printf("no relative path");
    return ERROR_DATABASE_INVALID;
  }

 char* pathtoblob = NULL;
  if(requestMemory((void**)&pathtoblob, strlen(handle->blobpath) + 1 + strlen(path) + 1) != ERROR_OK){
    freeMemory(path);
    return ERROR_MEMORY;
  }
  strcpy(pathtoblob, handle->blobpath);
  strcat(pathtoblob, "/");
  strcat(pathtoblob, path);
  pathtoblob[strlen(handle->blobpath) + 1 + strlen(path)] = '\0';

  FILE *file = NULL;
  file = fopen(pathtoblob, "wb");
  if(file == NULL){
    //perror("the following error occured: ");
    freeMemory(pathtoblob);
    //printf("cannot open file!\n");
    freeMemory(path);
    return ERROR_DATABASE_IO;
  }

  /* check if path: is a regular file
                    has a relative path 
                    is inside blob directory*/
  int error = check_blob_path(pathtoblob, handle->blobpath);
  if(error != ERROR_OK){
    remove(pathtoblob);
    freeMemory(pathtoblob);
    return error;
  }

  if(fwrite(value, 1, size, file) != size){
    //printf("cannot write file!\n");
    freeMemory(pathtoblob);
    freeMemory(path);
    return ERROR_DATABASE_IO;
  }

  if(fclose(file) != 0){
    //printf("cannot close file!\n");
    freeMemory(pathtoblob);
    freeMemory(path);
    return ERROR_DATABASE_IO;
  }

  /* Check if already existing */
  char* datatype = NULL;
  unsigned int id = 0;

  if(requestMemory((void**)&datatype, 7) != ERROR_OK){
    remove(pathtoblob);
    freeMemory(pathtoblob); 
    freeMemory(path); 
    return ERROR_MEMORY;
  }
  if(requestMemory((void**)&statement, 69) != ERROR_OK){
    remove(pathtoblob);
    freeMemory(pathtoblob);  
    freeMemory(datatype); 
    freeMemory(path);
    return ERROR_MEMORY;
  }

  strcpy(statement, "SELECT id, datatype FROM KeyInfo WHERE domain = :dom and key = :key;");

  begin(handle);
  if(sqlite3_prepare_v2(handle->db, statement, -1, &ppStmt, pzTail) != SQLITE_OK){
    //printf("prepare1: %s\n", sqlite3_errmsg(handle->db));
    sqlite3_finalize(ppStmt);
    rollback(handle);
    freeMemory(statement);
    freeMemory(datatype);  
    remove(pathtoblob);
    freeMemory(pathtoblob);
    freeMemory(path);
    return ERROR_DATABASE_INVALID;
  }
  freeMemory(statement);

  int parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":dom");
  if(parameterIndex == 0){
    //printf("bind parameter domain: %s\n", sqlite3_errmsg(handle->db));
    sqlite3_finalize(ppStmt);
    rollback(handle);
    freeMemory(datatype);  
    remove(pathtoblob);
    freeMemory(pathtoblob);
    freeMemory(path);
    return ERROR_DATABASE_INVALID;
  }

  if(sqlite3_bind_text(ppStmt, parameterIndex, domain, -1, SQLITE_TRANSIENT) != SQLITE_OK){
    //printf("bind domain: %s\n", sqlite3_errmsg(handle->db));
    sqlite3_finalize(ppStmt);
    rollback(handle);
    freeMemory(datatype);  
    remove(pathtoblob);
    freeMemory(pathtoblob);
    freeMemory(path);
    return ERROR_DATABASE_INVALID;
  }
  
  parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":key");
  if(parameterIndex == 0){
    //printf("bind parameter key: %s\n", sqlite3_errmsg(handle->db));
    sqlite3_finalize(ppStmt);
    rollback(handle);
    freeMemory(datatype);  
    remove(pathtoblob);
    freeMemory(pathtoblob);
    freeMemory(path);
    return ERROR_DATABASE_INVALID;
  }

  if(sqlite3_bind_text(ppStmt, parameterIndex, key, -1, SQLITE_TRANSIENT) != SQLITE_OK){
    //printf("bind key: %s\n", sqlite3_errmsg(handle->db));  
    sqlite3_finalize(ppStmt);
    rollback(handle);
    freeMemory(datatype);  
    remove(pathtoblob);
    freeMemory(pathtoblob);
    freeMemory(path);
    return ERROR_DATABASE_INVALID;
  }

  while (42){
      int retval = sqlite3_step(ppStmt);

      if(retval == SQLITE_ROW){
        if(sqlite3_column_type(ppStmt, 0) != SQLITE_INTEGER){
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          remove(pathtoblob);
          freeMemory(pathtoblob);
          freeMemory(path);
          return ERROR_DATABASE_TYPE_MISMATCH;
        }
        id = sqlite3_column_int(ppStmt, 0);    
        if(sqlite3_column_type(ppStmt, 0) != SQLITE_INTEGER){
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);
          remove(pathtoblob);
          freeMemory(pathtoblob);
          freeMemory(path);
          return ERROR_DATABASE_TYPE_MISMATCH;
        }  
        strcpy(datatype, (char*)sqlite3_column_text(ppStmt, 1)); 
        break;
      }
      else if(retval == SQLITE_DONE){
        freeMemory(datatype); 
        datatype = NULL;
        break;
      }
      else {
        //printf("step: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        remove(pathtoblob);
        freeMemory(pathtoblob);
        freeMemory(path);
        return ERROR_DATABASE_INVALID;
      } 
  }

  if(sqlite3_reset(ppStmt) != SQLITE_OK){
    //printf("reset: %s\n", sqlite3_errmsg(handle->db));
    sqlite3_finalize(ppStmt);
    rollback(handle);
    freeMemory(datatype);  
    remove(pathtoblob);
    freeMemory(pathtoblob);
    freeMemory(path);
    return ERROR_DATABASE_INVALID;
  }

  if(sqlite3_finalize(ppStmt) != SQLITE_OK){
    //printf("finalize: %s\n", sqlite3_errmsg(handle->db));
    rollback(handle);
    freeMemory(datatype);  
    remove(pathtoblob);
    freeMemory(pathtoblob);
    freeMemory(path);
    return ERROR_DATABASE_INVALID;
  }

  /* key doesn't exist */
  if(datatype == NULL){
    /* Insert into Keyinfo */
      if(requestMemory((void**)&statement, 78) != ERROR_OK){
        rollback(handle);
        freeMemory(datatype);
        remove(pathtoblob);
        freeMemory(pathtoblob);
        freeMemory(path);
        return ERROR_MEMORY;
      }

      strcpy(statement, "INSERT INTO KeyInfo(`domain`, `key`, `datatype`) VALUES (:dom, :key, 'Blob');");

      if(sqlite3_prepare_v2(handle->db, statement, -1, &ppStmt, pzTail) != SQLITE_OK){
        //printf("prepare2: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(statement);
        freeMemory(datatype); 
        remove(pathtoblob);
        freeMemory(pathtoblob); 
        freeMemory(path);
        return ERROR_DATABASE_INVALID;
      }
      freeMemory(statement);

      parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":dom");
      if(parameterIndex == 0){
        //printf("bind parameter domain: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        remove(pathtoblob);
        freeMemory(pathtoblob);
        freeMemory(path);
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_bind_text(ppStmt, parameterIndex, domain, -1, SQLITE_TRANSIENT) != SQLITE_OK){
        //printf("bind domain: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        remove(pathtoblob);
        freeMemory(pathtoblob);
        freeMemory(path);
        return ERROR_DATABASE_INVALID;
      }
      
      parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":key");
      if(parameterIndex == 0){
        //printf("bind parameter key: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        remove(pathtoblob);
        freeMemory(pathtoblob);
        freeMemory(path);
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_bind_text(ppStmt, parameterIndex, key, -1, SQLITE_TRANSIENT) != SQLITE_OK){
        //printf("bind key: %s\n", sqlite3_errmsg(handle->db));  
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        remove(pathtoblob);
        freeMemory(pathtoblob);
        freeMemory(path);
        return ERROR_DATABASE_INVALID;
      }

      while(42){
        int retval = sqlite3_step(ppStmt);
        if(retval == SQLITE_DONE){
          break;
        }
        else if(retval == SQLITE_ERROR){
          //printf("fucking step1: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          remove(pathtoblob);
          freeMemory(pathtoblob);
          freeMemory(path);
          return ERROR_DATABASE_INVALID;
        }
        else if(retval == SQLITE_MISUSE){
          //printf("fucking step2: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          remove(pathtoblob);
          freeMemory(pathtoblob);
          freeMemory(path);
          return ERROR_DATABASE_INVALID;
        }
      }

      if(sqlite3_reset(ppStmt) != SQLITE_OK){
        //printf("reset: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        remove(pathtoblob);
        freeMemory(pathtoblob);
        freeMemory(path);
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_finalize(ppStmt) != SQLITE_OK){
        //printf("finalize: %s\n", sqlite3_errmsg(handle->db));
        rollback(handle);
        freeMemory(datatype);  
        remove(pathtoblob);
        freeMemory(pathtoblob);
        freeMemory(path);
        return ERROR_DATABASE_INVALID;
      }

      /* Insert into ValueBlob */
      if(requestMemory((void**)&statement, 55) != ERROR_OK){
        rollback(handle);
        freeMemory(datatype);  
        remove(pathtoblob);
        freeMemory(pathtoblob);
        freeMemory(path);
        return ERROR_MEMORY; 
      }

      strcpy(statement, "INSERT INTO ValueBlob(`id`, `path`) VALUES (:id,:pat);");

      if(sqlite3_prepare_v2(handle->db, statement, -1, &ppStmt, pzTail) != SQLITE_OK){
        //printf("prepare3: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(statement);
        freeMemory(path);
        freeMemory(datatype);  
        remove(pathtoblob);
        freeMemory(pathtoblob);
        return ERROR_DATABASE_INVALID;
      }
      freeMemory(statement);

      parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":id");
      if(parameterIndex == 0){
        //printf("bind parameter index: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(path);
        freeMemory(datatype);  
        remove(pathtoblob);
        freeMemory(pathtoblob);
        return ERROR_DATABASE_INVALID;
      }
        
      if(sqlite3_bind_int64(ppStmt, parameterIndex, sqlite3_last_insert_rowid(handle->db)) != SQLITE_OK){
        //printf("bind id: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(path);
        freeMemory(datatype);  
        remove(pathtoblob);
        freeMemory(pathtoblob);
        return ERROR_DATABASE_INVALID;
      } 

      parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":pat");
      if(parameterIndex == 0){
        //printf("bind parameter index: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(path);
        freeMemory(datatype);  
        remove(pathtoblob);
        freeMemory(pathtoblob);
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_bind_text(ppStmt, parameterIndex, path, -1, SQLITE_TRANSIENT) != SQLITE_OK){
        //printf("bind value: %s\n", sqlite3_errmsg(handle->db));  
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(path);
        freeMemory(datatype);  
        remove(pathtoblob);
        freeMemory(pathtoblob);
        return ERROR_DATABASE_INVALID;
      }

      while(42){
        int retval = sqlite3_step(ppStmt);
        if(retval == SQLITE_DONE){
          break;
        }
        else if(retval == SQLITE_ERROR){
          //printf("fucking step1: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          remove(pathtoblob);
          freeMemory(pathtoblob);
          freeMemory(path);
          return ERROR_DATABASE_INVALID;
        }
        else if(retval == SQLITE_MISUSE){
          //printf("fucking step2: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          remove(pathtoblob);
          freeMemory(pathtoblob);
          freeMemory(path);
          return ERROR_DATABASE_INVALID;
        }
      }

      if(sqlite3_reset(ppStmt) != SQLITE_OK){
        //printf("reset: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        remove(pathtoblob);
        freeMemory(pathtoblob);
        freeMemory(path);
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_finalize(ppStmt) != SQLITE_OK){
        //printf("finalize: %s\n", sqlite3_errmsg(handle->db));
        rollback(handle);
        freeMemory(datatype);  
        remove(pathtoblob);
        freeMemory(pathtoblob);
        freeMemory(path);
        return ERROR_DATABASE_INVALID;
      }
  }
  else{ /* key already exist */    
    /* Datatype is the same - just update value */
    if(!strcmp(datatype, "Blob")){ 
      /* Update ValueDouble */
      if(requestMemory((void**)&statement, 49) != ERROR_OK){
        rollback(handle);
        freeMemory(datatype);  
        remove(pathtoblob);
        freeMemory(pathtoblob);
        freeMemory(path);
        return ERROR_MEMORY;
      }

      strcpy(statement, "UPDATE ValueBlob SET path = :pat WHERE id = :id;");

      if(sqlite3_prepare_v2(handle->db, statement, -1, &ppStmt, pzTail) != SQLITE_OK){
        //printf("prepare4: %s\n", sqlite3_errmsg(handle->db));
        rollback(handle);
        freeMemory(statement);
        freeMemory(path);
        freeMemory(datatype);   
        remove(pathtoblob);
        freeMemory(pathtoblob);
        return ERROR_DATABASE_INVALID;
      }
      freeMemory(statement);

      parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":pat");
      if(parameterIndex == 0){
        //printf("bind parameter index: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(path);
        freeMemory(datatype);  
        remove(pathtoblob);
        freeMemory(pathtoblob);
        return ERROR_DATABASE_INVALID;
      }
        
      if(sqlite3_bind_text(ppStmt, parameterIndex, path, -1, SQLITE_TRANSIENT) != SQLITE_OK){
        //printf("bind path: %s\n", sqlite3_errmsg(handle->db));  
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(path);
        freeMemory(datatype);  
        remove(pathtoblob);
        freeMemory(pathtoblob);
        return ERROR_DATABASE_INVALID;
      }

      parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":id");
      if(parameterIndex == 0){
        //printf("bind parameter index: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        remove(pathtoblob);
        freeMemory(pathtoblob);
        freeMemory(path);
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_bind_int(ppStmt, parameterIndex, id) != SQLITE_OK){
        //printf("bind id: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        remove(pathtoblob);
        freeMemory(pathtoblob);
        freeMemory(path);
        return ERROR_DATABASE_INVALID;
      }

      while(42){
        int retval = sqlite3_step(ppStmt);
        if(retval == SQLITE_DONE){
          break;
        }
        else if(retval == SQLITE_ERROR){
          //printf("fucking step1: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          remove(pathtoblob);
          freeMemory(pathtoblob);
          freeMemory(path);
          return ERROR_DATABASE_INVALID;
        }
        else if(retval == SQLITE_MISUSE){
          //printf("fucking step2: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          remove(pathtoblob);
          freeMemory(pathtoblob);
          freeMemory(path);
          return ERROR_DATABASE_INVALID;
        }
      }

      if(sqlite3_reset(ppStmt) != SQLITE_OK){
        //printf("reset: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        remove(pathtoblob);
        freeMemory(pathtoblob);
        freeMemory(path);
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_finalize(ppStmt) != SQLITE_OK){
        //printf("finalize: %s\n", sqlite3_errmsg(handle->db));
        rollback(handle);
        freeMemory(datatype);  
        remove(pathtoblob);
        freeMemory(pathtoblob);
        freeMemory(path);
        return ERROR_DATABASE_INVALID;
      }
    }
    else{ /* Different datatype - delete, update and insert */
      
      /* Delete from ValueXtable */
      if(requestMemory((void**)&statement, 40) != ERROR_OK){
        rollback(handle);
        freeMemory(datatype);    
        remove(pathtoblob);
        freeMemory(pathtoblob);    
        freeMemory(path);
        return ERROR_MEMORY;
      }

      strcpy(statement, "DELETE FROM Value");
      strcat(statement, datatype);
      strcat(statement, " WHERE id = :id;");

      if(sqlite3_prepare_v2(handle->db, statement, -1, &ppStmt, pzTail) != SQLITE_OK){
        //printf("prepare5: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(statement);
        freeMemory(datatype);  
        remove(pathtoblob);
        freeMemory(pathtoblob);
        freeMemory(path);
        return ERROR_DATABASE_INVALID;
      } 
      freeMemory(statement);

      parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":id");
      if(parameterIndex == 0){
        //printf("bind parameter index: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        remove(pathtoblob);
        freeMemory(pathtoblob);
        freeMemory(path);
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_bind_int(ppStmt, parameterIndex, id) != SQLITE_OK){
        //printf("bind id: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        remove(pathtoblob);
        freeMemory(pathtoblob);
        freeMemory(path);
        return ERROR_DATABASE_INVALID;
      }

      while(42){
        int retval = sqlite3_step(ppStmt);
        if(retval == SQLITE_DONE){
          break;
        }
        else if(retval == SQLITE_ERROR){
          //printf("fucking step1: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          remove(pathtoblob);
          freeMemory(pathtoblob);
          freeMemory(path);
          return ERROR_DATABASE_INVALID;
        }
        else if(retval == SQLITE_MISUSE){
          //printf("fucking step2: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          remove(pathtoblob);
          freeMemory(pathtoblob);
          freeMemory(path);
          return ERROR_DATABASE_INVALID;
        }
      }

      if(sqlite3_reset(ppStmt) != SQLITE_OK){
        //printf("reset: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        remove(pathtoblob);
        freeMemory(pathtoblob);
        freeMemory(path);
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_finalize(ppStmt) != SQLITE_OK){
        //printf("finalize: %s\n", sqlite3_errmsg(handle->db));
        freeMemory(datatype);  
        rollback(handle);
        remove(pathtoblob);
        freeMemory(pathtoblob);
        freeMemory(path);
        return ERROR_DATABASE_INVALID;
      }

      /* Delete from KeyInfo */
      if(requestMemory((void**)&statement, 36) != ERROR_OK){
        freeMemory(datatype);   
        rollback(handle);
        remove(pathtoblob);
        freeMemory(pathtoblob); 
        freeMemory(path);    
        return ERROR_MEMORY;
      }

      strcpy(statement, "DELETE FROM KeyInfo WHERE id = :id;");

      if(sqlite3_prepare_v2(handle->db, statement, -1, &ppStmt, pzTail) != SQLITE_OK){
        //printf("prepare6: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(statement);
        freeMemory(datatype);  
        remove(pathtoblob);
        freeMemory(pathtoblob);
        freeMemory(path);
        return ERROR_DATABASE_INVALID;
      }
      freeMemory(statement);

      parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":id");
      if(parameterIndex == 0){
        //printf("bind parameter index: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        remove(pathtoblob);
        freeMemory(pathtoblob);
        freeMemory(path);
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_bind_int(ppStmt, parameterIndex, id) != SQLITE_OK){
        //printf("bind id: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        remove(pathtoblob);
        freeMemory(pathtoblob);
        freeMemory(path);
        return ERROR_DATABASE_INVALID;
      }

      while(42){
        int retval = sqlite3_step(ppStmt);
        if(retval == SQLITE_DONE){
          break;
        }
        else if(retval == SQLITE_ERROR){
          //printf("fucking step1: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          remove(pathtoblob);
          freeMemory(pathtoblob);
          freeMemory(path);
          return ERROR_DATABASE_INVALID;
        }
        else if(retval == SQLITE_MISUSE){
          //printf("fucking step2: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          remove(pathtoblob);
          freeMemory(pathtoblob);
          freeMemory(path);
          return ERROR_DATABASE_INVALID;
        }
      }

      if(sqlite3_reset(ppStmt) != SQLITE_OK){
        //printf("reset: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        remove(pathtoblob);
        freeMemory(pathtoblob);
        freeMemory(path);
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_finalize(ppStmt) != SQLITE_OK){
        //printf("finalize: %s\n", sqlite3_errmsg(handle->db));
        rollback(handle);
        freeMemory(datatype);  
        remove(pathtoblob);
        freeMemory(pathtoblob);
        freeMemory(path);
        return ERROR_DATABASE_INVALID;
      }

    /* Insert into Keyinfo */
      if(requestMemory((void**)&statement, 91) != ERROR_OK){
        rollback(handle);
        freeMemory(datatype);  
        remove(pathtoblob);
        freeMemory(pathtoblob);
        freeMemory(path);
        return ERROR_MEMORY;
      }

      strcpy(statement, "INSERT INTO KeyInfo(`id`, `domain`, `key`, `datatype`) VALUES (:id, :dom, :key, 'Blob');");

      if(sqlite3_prepare_v2(handle->db, statement, -1, &ppStmt, pzTail) != SQLITE_OK){
        //printf("prepare7: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(statement);
        freeMemory(datatype);  
        remove(pathtoblob);
        freeMemory(pathtoblob);
        freeMemory(path);
        return ERROR_DATABASE_INVALID;
      } 
      freeMemory(statement);

      parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":id");
      if(parameterIndex == 0){
        //printf("bind parameter index: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        remove(pathtoblob);
        freeMemory(pathtoblob);
        freeMemory(path);
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_bind_int(ppStmt, parameterIndex, id) != SQLITE_OK){
        //printf("bind id: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        remove(pathtoblob);
        freeMemory(pathtoblob);
        freeMemory(path);
        return ERROR_DATABASE_INVALID;
      }

      parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":dom");
      if(parameterIndex == 0){
        //printf("bind parameter domain: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        remove(pathtoblob);
        freeMemory(pathtoblob);
        freeMemory(path);
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_bind_text(ppStmt, parameterIndex, domain, -1, SQLITE_TRANSIENT) != SQLITE_OK){
        //printf("bind domain: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        remove(pathtoblob);
        freeMemory(pathtoblob);
        freeMemory(path);
        return ERROR_DATABASE_INVALID;
      }
      
      parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":key");
      if(parameterIndex == 0){
        //printf("bind parameter key: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        remove(pathtoblob);
        freeMemory(pathtoblob);
        freeMemory(path);
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_bind_text(ppStmt, parameterIndex, key, -1, SQLITE_TRANSIENT) != SQLITE_OK){
        //printf("bind key: %s\n", sqlite3_errmsg(handle->db));  
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        remove(pathtoblob);
        freeMemory(pathtoblob);
        freeMemory(path);
        return ERROR_DATABASE_INVALID;
      }

      while(42){
        int retval = sqlite3_step(ppStmt);
        if(retval == SQLITE_DONE){
          break;
        }
        else if(retval == SQLITE_ERROR){
          //printf("fucking step1: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          remove(pathtoblob);
          freeMemory(pathtoblob);
          freeMemory(path);
          return ERROR_DATABASE_INVALID;
        }
        else if(retval == SQLITE_MISUSE){
          //printf("fucking step2: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          remove(pathtoblob);
          freeMemory(pathtoblob);
          freeMemory(path);
          return ERROR_DATABASE_INVALID;
        }
      }

      if(sqlite3_reset(ppStmt) != SQLITE_OK){
        //printf("reset: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        remove(pathtoblob);
        freeMemory(pathtoblob);
        freeMemory(path);
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_finalize(ppStmt) != SQLITE_OK){
        //printf("finalize: %s\n", sqlite3_errmsg(handle->db));
        rollback(handle);
        freeMemory(datatype);  
        remove(pathtoblob);
        freeMemory(pathtoblob);
        freeMemory(path);
        return ERROR_DATABASE_INVALID;
      }

      /* Insert into ValueBlob */
      if(requestMemory((void**)&statement, 55) != ERROR_OK){
        rollback(handle);
        freeMemory(datatype);   
        remove(pathtoblob);
        freeMemory(pathtoblob);  
        freeMemory(path);   
        return ERROR_MEMORY;
      }

      strcpy(statement, "INSERT INTO ValueBlob(`id`, `path`) VALUES (:id,:pat);");

      if(sqlite3_prepare_v2(handle->db, statement, -1, &ppStmt, pzTail) != SQLITE_OK){
        //printf("prepare8: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(path);
        freeMemory(statement);
        freeMemory(datatype);  
        remove(pathtoblob);
        freeMemory(pathtoblob);
        freeMemory(path);
        return ERROR_DATABASE_INVALID;
      }
      freeMemory(statement);

      parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":id");
      if(parameterIndex == 0){
        //printf("bind parameter index: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(path);
        freeMemory(datatype);  
        remove(pathtoblob);
        freeMemory(pathtoblob);
        freeMemory(path);
        return ERROR_DATABASE_INVALID;
      }
        
      if(sqlite3_bind_int(ppStmt, parameterIndex, id) != SQLITE_OK){
        //printf("bind id: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(path);
        freeMemory(datatype);  
        remove(pathtoblob);
        freeMemory(pathtoblob);
        freeMemory(path);
        return ERROR_DATABASE_INVALID;
      } 

      parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":pat");
      if(parameterIndex == 0){
        //printf("bind parameter index: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(path);
        freeMemory(datatype);  
        remove(pathtoblob);
        freeMemory(pathtoblob);
        freeMemory(path);
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_bind_text(ppStmt, parameterIndex, path, -1, SQLITE_TRANSIENT) != SQLITE_OK){
        //printf("bind value: %s\n", sqlite3_errmsg(handle->db));  
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(path);
        freeMemory(datatype);  
        remove(pathtoblob);
        freeMemory(pathtoblob);
        freeMemory(path);
        return ERROR_DATABASE_INVALID;
      }
      freeMemory(path);

      while(42){
        int retval = sqlite3_step(ppStmt);
        if(retval == SQLITE_DONE){
          break;
        }
        else if(retval == SQLITE_ERROR){
          //printf("fucking step1: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          remove(pathtoblob);
          freeMemory(pathtoblob);
          freeMemory(path);
          return ERROR_DATABASE_INVALID;
        }
        else if(retval == SQLITE_MISUSE){
          //printf("fucking step2: %s\n", sqlite3_errmsg(handle->db));
          sqlite3_finalize(ppStmt);
          rollback(handle);
          freeMemory(datatype);  
          remove(pathtoblob);
          freeMemory(pathtoblob);
          freeMemory(path);
          return ERROR_DATABASE_INVALID;
        }
      }

      if(sqlite3_reset(ppStmt) != SQLITE_OK){
        //printf("reset: %s\n", sqlite3_errmsg(handle->db));
        sqlite3_finalize(ppStmt);
        rollback(handle);
        freeMemory(datatype);  
        remove(pathtoblob);
        freeMemory(pathtoblob);
        freeMemory(path);
        return ERROR_DATABASE_INVALID;
      }

      if(sqlite3_finalize(ppStmt) != SQLITE_OK){
        //printf("finalize: %s\n", sqlite3_errmsg(handle->db));
        rollback(handle);
        freeMemory(datatype);  
        remove(pathtoblob);
        freeMemory(pathtoblob);
        freeMemory(path);
        return ERROR_DATABASE_INVALID;
      }
    }    
  }
  commit(handle);
  freeMemory(pathtoblob);
  freeMemory(path);
  freeMemory(datatype);
  return ERROR_OK;
}


int removeReferencedBlobFile(database_handle_t* handle, const char* domain, const char* key){
  char* statement = NULL;
  sqlite3_stmt *ppStmt = NULL;
  const char** pzTail = NULL;

  unsigned int sizeOfStmt = 188;

  if(requestMemory((void**)&statement, sizeOfStmt) != ERROR_OK)
    return ERROR_MEMORY;
  char* blobpath = NULL;

  strcpy(statement, "SELECT ValueBlob.`path` as `path` FROM KeyInfo INNER JOIN ValueBlob ON KeyInfo.`id` = ValueBlob.`id`WHERE KeyInfo.`datatype` = 'Blob' AND KeyInfo.`domain` = :dom AND KeyInfo.`key`= :key;");

  begin(handle);
  if(sqlite3_prepare_v2(handle->db, statement, -1, &ppStmt, pzTail) != SQLITE_OK){
    //printf("prepare: %s\n", sqlite3_errmsg(handle->db));
    sqlite3_finalize(ppStmt);
    rollback(handle);
    freeMemory(statement);
    return ERROR_DATABASE_INVALID;
  }
  freeMemory(statement);

  int parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":dom");
  if(parameterIndex == 0){
    //printf("bind parameter domain: %s\n", sqlite3_errmsg(handle->db));
    rollback(handle);
    sqlite3_finalize(ppStmt);
    return ERROR_DATABASE_INVALID;
  }

  if(sqlite3_bind_text(ppStmt, parameterIndex, domain, -1, SQLITE_TRANSIENT) != SQLITE_OK){
    //printf("bind domain: %s\n", sqlite3_errmsg(handle->db));
    rollback(handle);
    sqlite3_finalize(ppStmt);
    return ERROR_DATABASE_INVALID;
  }
  
  parameterIndex = sqlite3_bind_parameter_index(ppStmt, ":key");
  if(parameterIndex == 0){
    //printf("bind parameter key: %s\n", sqlite3_errmsg(handle->db));
    rollback(handle);
    sqlite3_finalize(ppStmt);
    return ERROR_DATABASE_INVALID;
  }

  if(sqlite3_bind_text(ppStmt, parameterIndex, key, -1, SQLITE_TRANSIENT) != SQLITE_OK){
    //printf("bind key: %s\n", sqlite3_errmsg(handle->db));
    rollback(handle);  
    sqlite3_finalize(ppStmt);
    return ERROR_DATABASE_INVALID;
  }

  while (42){
      int retval = sqlite3_step(ppStmt);

      if(retval == SQLITE_ROW){   
        if(sqlite3_column_type(ppStmt, 0) != SQLITE3_TEXT){
          rollback(handle);
          sqlite3_finalize(ppStmt);
          return ERROR_DATABASE_TYPE_MISMATCH;
        }      

        char * dbentry = NULL;
        dbentry = (char*)sqlite3_column_text(ppStmt, 0); 
        if(requestMemory((void**)&blobpath, strlen(dbentry)+1) != ERROR_OK){
          rollback(handle);
          sqlite3_finalize(ppStmt);
          return ERROR_MEMORY;
        }
        memcpy(blobpath, dbentry, strlen(dbentry));
        blobpath[strlen(dbentry)] = '\0';
        break;
      }
      else if(retval == SQLITE_DONE){
        rollback(handle);
        sqlite3_finalize(ppStmt);
        return ERROR_DATABASE_NO_SUCH_KEY;
      }
      else {
        //printf("step: %s\n", sqlite3_errmsg(handle->db));
        rollback(handle);
        sqlite3_finalize(ppStmt);
        return ERROR_DATABASE_INVALID;
      } 
  }

  if(sqlite3_reset(ppStmt) != SQLITE_OK){
    //printf("reset: %s\n", sqlite3_errmsg(handle->db));
    rollback(handle);
    sqlite3_finalize(ppStmt);
    freeMemory(blobpath);
    return ERROR_DATABASE_INVALID;
  }

  if(sqlite3_finalize(ppStmt) != SQLITE_OK){
    //printf("finalize: %s\n", sqlite3_errmsg(handle->db));
    rollback(handle);
    freeMemory(blobpath);
    return ERROR_DATABASE_INVALID;
  }
  commit(handle);

  /* check for relative path */
  if(blobpath[0] == '/'){
    freeMemory(blobpath);
    return ERROR_DATABASE_INVALID;
  }

  char* pathtoblob = NULL;
  if(requestMemory((void**)&pathtoblob, strlen(handle->blobpath) + 1 + strlen(blobpath)+1) != ERROR_OK){
    freeMemory(blobpath);
    return ERROR_MEMORY;
  }
  strcpy(pathtoblob, handle->blobpath);
  strcat(pathtoblob, "/");
  strcat(pathtoblob, blobpath);
  pathtoblob[strlen(handle->blobpath) + 1 + strlen(blobpath)] = '\0';
  freeMemory(blobpath);

  /* check if path: is a regular file */
  struct stat sb;
  if(stat(pathtoblob, &sb) != 0){
    //printf("something gone wrong... stat\n");
    freeMemory(pathtoblob);
    return ERROR_DATABASE_INVALID;
  }
  if(!S_ISREG(sb.st_mode)){        /* check if regular file */
    //printf("regular file");
    freeMemory(pathtoblob);
    return ERROR_DATABASE_INVALID;
  }

  int error = check_blob_path(pathtoblob, handle->blobpath);
  if(error != ERROR_OK){
    freeMemory(pathtoblob);
    //printf("something gone wrong... check_blob_path\n");
    return error;
  }

  /* remove referenced blob */
  /* regarding to database.h nobody cares if working or not */
  remove(pathtoblob);

  return ERROR_OK;
}



