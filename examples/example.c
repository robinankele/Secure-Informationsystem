/*
 * Before executing this example run:
 *
 *   sqlite3 mydb.sqlite < ../sql/database-init.sql
 *
 * or
 *
 *   ../sql/create-database.sh mydb.sqlite ../sql/database-init.sql
 *
 * in the examples folder
 */

#include "registry/registry.h"
#include "server/database.h"
#include "errors.h"
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

int main(void)
{
  /**
   * Registry
   */

  /* Open the registry */
  registry_t* registry = NULL;
  assert(registry_open(&registry, "file://mydb.sqlite", "domain") == ERROR_OK);

  /* Set an integer value */
  int64_t val = 10, nval;
  assert(registry_set_int64(registry, "my integer value", val) == ERROR_OK);

  /* Check the type */
  int type;
  assert(registry_key_get_value_type(registry, "my integer value", &type) == ERROR_OK);
  assert(type == DATABASE_TYPE_INT64);

  /* Get the integer value */
  assert(registry_get_int64(registry, "my integer value", &nval) == ERROR_OK);
  assert(nval ==  val);

  /* Set and get a blob */
  const unsigned char blob[] = { 0x00, 0x6E, 0x00, 0xA7, 0x00, 0x19, 0x00, 0x03 };
  assert(registry_set_blob(registry, "oh, it's a blob", blob, sizeof(blob)) == ERROR_OK);
  size_t blob_size = 0;
  unsigned char* theblob = NULL;
  assert(registry_get_blob(registry, "oh, it's a blob", &theblob, &blob_size) == ERROR_OK);
  assert(theblob != NULL);
  assert(blob_size == sizeof(blob));
  assert(memcmp(blob, theblob, sizeof(blob)) == 0);
  free(theblob);

  /* Close registry */
  assert(registry_close(registry) == ERROR_OK);

  /**
   * Database
   */

  /* Get and set a integer from the datbase */
  database_handle_t* database = NULL;
  assert(database_open(&database, "mydb.sqlite") == ERROR_OK);

  /* Set integer value */
  assert(database_set_int64(database, "another domain", "my integer value", val) == ERROR_OK);

  /* Get integer value */
  assert(database_get_int64(database, "another domain", "my integer value", &nval) == ERROR_OK);
  assert(nval == val);

  /* Enumerate some keys */
  assert(database_set_int64(database, "enum", "key1", 0) == ERROR_OK);
  assert(database_set_int64(database, "enum", "key2", 0) == ERROR_OK);
  assert(database_set_int64(database, "enum", "key3", 0) == ERROR_OK);
  assert(database_set_int64(database, "enum", "no match", 0) == ERROR_OK);

  size_t size = 0, count = 0;
  char* keys = NULL;
  assert(database_enum_keys(database, "enum", "key*", &count, &size, &keys) == ERROR_OK);
  assert(count == 3); /* 3 results */
  assert(size == 15); /* each key is a string with 4 characters and a 0 byte */
  assert(keys != NULL);
  assert(strncmp(keys, "key1", 5) == 0); /* keys are sorted */
  assert(strncmp(keys + 5, "key2", 5) == 0);
  assert(strncmp(keys + 10, "key3", 5) == 0);
  free(keys);

  /* Get and set a string from the database */
  assert(database_set_string(database, "strings", "are awesome", "'; --") == ERROR_OK);
  char* string_value = NULL;
  assert(database_get_string(database, "strings", "are awesome", &string_value) == ERROR_OK);
  assert(string_value != NULL);
  assert(strncmp(string_value, "'; --", 6) == 0);
  free(string_value);

  /* Set and get a double */
  assert(database_set_double(database, "double", "1", INFINITY) == ERROR_OK);
  double d = 0;
  assert(database_get_double(database, "double", "1", &d) == ERROR_OK);
  assert(d == INFINITY);

  /* Close database */
  assert(database_close(database) == ERROR_OK);
}
