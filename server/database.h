#ifndef DATABASE_H
#define DATABASE_H

/** @brief Database interface
 *
 * The database is used as backend to store the data entered into the registry.
 * It is able to store four different types of data: 64-bit integers, double
 * precision floating point values, NUL-terminated strings and binary blobs.
 *
 * @ref database_open opens an existing database and never creates the database
 * on its own. It also performs basic sanity checks on the database's scheme. It
 * must match the scheme defined in sql/database-init.sql. Additional tables and
 * columns as well as data types may exist. However, if any tables, columns or
 * data-types are missing, constraints are not set properly (i.e.. primary key,
 * not null and auto increment)), @ref database_open fails with @ref
 * ERROR_DATABASE_INVALID. Additionally, @ref database_open reads the blob path
 * from the database. The blob path is stored as string value with domain NULL
 * and key blob-path. The path stored in this value has to exist and has to be
 * a directory.
 *
 * @ref database_get_int64, @ref database_get_double, @ref database_get_string and
 * @ref database_get_blob as well as database_get_type retrieve the value from
 * the database and return the associated type respectively. @ref
 * database_set_int64, @ref database_set_double, @ref database_set_string and @ref
 * database_set_blob set values. These four functions rollback any changes if
 * one of the queries fails or, in the case of blobs, any file system operations
 * fails. They also make sure that the database is kept clean, meaning:
 *  - If the row in KeyInfo has the id @a a and data type @a b, then there
 *    exists a row in Value@a b with the exact same key and no other Value table
 *    contains a row with id @a a.
 *  - If a row from ValueBlob is removed, the referenced file has to be deleted.
 *  - If a existing blob is overwritten and the new data is written to a new
 *    file, the old file has to be deleted.
 *  Please note that an error while removing an unreferenced blob file is not
 *  critical and is ignored.
 *
 *  Blob files are stored according to the following scheme: @a $blob-path/$path
 *  where @a $blob-path is extracted from the database in @ref database_open and
 *  @a $path is stored in the ValueBlob table. Every access of an blob file has
 *  to make sure that the file is a regular file and that the file is in @a
 *  $blob-path or any of its subdirectories.
 *
 *  Wherever a domain or key is required as argument, they both may not be @a
 *  NULL or empty strings. All arguments that are used as destination may not be
 *  @a NULL. Furthermore all @database_handle_t pointers may not be @a NULL. All
 *  other strings and blobs may not be @a NULL
 *
 * All the handles may not be @a NULL. All domains and keys may not be @a NULL
 * and have to be non-empty NUL-terminated strings. The target pointers in the
 * database_get_* functions and in database_enum_keys may not be @a NULL. Blobs
 * and strings may not be @a NULL.
 *
 * @file database.h
 */

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @a database_handle_s has to be defined in the C file implementing the
 * database API.
 */
typedef struct database_handle_s database_handle_t;

typedef enum database_value_type_e
{
  DATABASE_TYPE_INT64 = 0,
  DATABASE_TYPE_DOUBLE,
  DATABASE_TYPE_STRING,
  DATABASE_TYPE_BLOB
} database_value_type_t;

/**
 * Open an existing database. The database must exist and be valid. The
 * function returns an error if this is not the case..
 *
 * @param[out] handle Pointer to the database handle that should be used for
 *   this database connection.
 * @param[in] path Path to a valid sqlite database.
 *
 * @return @ref ERROR_OK on success,
 * @return @ref ERROR_DATABASE_OPEN The database does not exist or is not a
 *  regular file.
 * @return @ref ERROR_DATABASE_INVALID The database is invalid, i.e a table does
 *  not exist or a column doesn't match the specification or the blob-path is
 *  not an existing directory.
 * @return @ref ERROR_MEMORY Out of memory.
 * @return @ref ERROR_INVALID_ARGUMENTS Invalid arguments have been passed.
 * @return @ref ERROR_UNKNOWN An unspecified error occurred.
 */
int database_open(database_handle_t** handle, const char* path);

/**
 * Close database.
 *
 * @param[in] handle Database handle to be freed, non-NULL.
 *
 * @return @ref ERROR_OK on success.
 * @return @ref ERROR_INVALID_ARGUMENTS Invalid arguments have been passed.
 * @return @ref ERROR_UNKNOWN An unspecified error occurred.
 */
int database_close(database_handle_t* handle);

/**
 * Query the value's type associated to a domain and key.
 *
 * @param[in] handle Database handle
 * @param[in] domain The domain of the keys
 * @param[in] key The key
 * @param[out] type The type of the key
 *
 * @return @ref ERROR_OK on success,
 * @return @ref ERROR_DATABASE_INVALID The database is invalid, i.e one of the
 *  queries failed.
 * @return @ref ERROR_DATABASE_TYPE_UNKNOWN The associated type is not Int64,
 *  Double, String or Blob.
 * @return @ref ERROR_DATABASE_NO_SUCH_KEY The domain, key pair does not exist.
 * @return @ref ERROR_INVALID_ARGUMENTS Invalid arguments have been passed.
 * @return @ref ERROR_MEMORY Out of memory.
 * @return @ref ERROR_UNKNOWN An unspecified error occurred.
 */
int database_get_type(database_handle_t* handle, const char* domain,
    const char* key, database_value_type_t* type);

/** Enumerate keys.
 *
 * The keys are returned in one large string that is separated by \c 0s. The SQL
 * statement @a GLOB is used to enumerate the keys. So @a pattern can be
 * anything that is valid as argument to @a GLOB. The result is sorted
 * alphabetically.
 *
 * The caller is responsible to free up the memory pointed to by keys.
 *
 * @param[in] handle A valid database handle.
 * @param[in] domain The domain of the keys.
 * @param[in] pattern The key pattern.
 * @param[out] count Number of enumerated keys.
 * @param[out] size Size of keys.
 * @param[out] keys The enumerated keys.
 *
 * @return @ref ERROR_OK on success,
 * @return @ref ERROR_DATABASE_INVALID The database is invalid, i.e one of the
 *  queries failed.
 * @return @ref ERROR_MEMORY Out of memory.
 * @return @ref ERROR_INVALID_ARGUMENTS Invalid arguments have been passed.
 * @return @ref ERROR_UNKNOWN An unspecified error occurred.
 */
int database_enum_keys(database_handle_t* handle, const char* domain,
    const char* pattern, size_t* count, size_t* size, char** keys);

/**
 * Retrieve the value associated to the domain and key.
 *
 * @param[in] handle A valid database handle.
 * @param[in] domain The domain of the keys.
 * @param[in] key The key.
 * @param[out] value The value
 *
 * @return @ref ERROR_OK on success,
 * @return @ref ERROR_INVALID_ARGUMENTS Invalid arguments have been passed
 * @return @ref ERROR_DATABASE_INVALID The database is invalid, i.e one of the
 *  queries failed.
 * @return @ref ERROR_DATABASE_NO_SUCH_KEY The domain, key pair does not exist.
 * @return @ref ERROR_DATABASE_TYPE_MISMATCH The value associated to the domain,
 *  key pair is not of the correct type.
 * @return @ref ERROR_MEMORY Out of memory.
 * @return @ref ERROR_UNKNOWN An unspecified error occurred.
 */
int database_get_int64(database_handle_t* handle, const char* domain,
    const char* key, int64_t* value);

/**
 * Set the value associated to the domain and key.
 *
 * @param[in] handle A valid database handle.
 * @param[in] domain The domain of the keys.
 * @param[in] key The key.
 * @param[in] value The value
 *
 * @return @ref ERROR_OK on success,
 * @return @ref ERROR_INVALID_ARGUMENTS Invalid arguments have been passed
 * @return @ref ERROR_DATABASE_INVALID The database is invalid, i.e one of the
 *  queries failed.
 * @return @ref ERROR_MEMORY Out of memory.
 * @return @ref ERROR_UNKNOWN An unspecified error occurred.
 */
int database_set_int64(database_handle_t* handle, const char* domain,
    const char* key, int64_t value);

/**
 * Retrieve the value associated to the domain and key.
 *
 * @param[in] handle A valid database handle.
 * @param[in] domain The domain of the keys.
 * @param[in] key The key.
 * @param[out] value The value (not NAN)
 *
 * @return @ref ERROR_OK on success,
 * @return @ref ERROR_INVALID_ARGUMENTS Invalid arguments have been passed
 * @return @ref ERROR_DATABASE_INVALID The database is invalid, i.e one of the
 *  queries failed.
 * @return @ref ERROR_DATABASE_NO_SUCH_KEY The domain, key pair does not exist.
 * @return @ref ERROR_DATABASE_TYPE_MISMATCH The value associated to the domain,
 *  key pair is not of the correct type.
 * @return @ref ERROR_MEMORY Out of memory.
 * @return @ref ERROR_UNKNOWN An unspecified error occurred.
 */
int database_get_double(database_handle_t* handle, const char* domain,
    const char* key, double* value);

/**
 * Set the value associated to the domain and key.
 *
 * @param[in] handle A valid database handle.
 * @param[in] domain The domain of the keys.
 * @param[in] key The key.
 * @param[in] value The value
 *
 * @return @ref ERROR_OK on success,
 * @return @ref ERROR_INVALID_ARGUMENTS Invalid arguments have been passed
 * @return @ref ERROR_DATABASE_INVALID The database is invalid, i.e one of the
 *  queries failed.
 * @return @ref ERROR_MEMORY Out of memory.
 * @return @ref ERROR_UNKNOWN An unspecified error occurred.
 */
int database_set_double(database_handle_t* handle, const char* domain,
    const char* key, double value);

/**
 * Retrieve the value associated to the domain and key.
 *
 * @param[in] handle A valid database handle.
 * @param[in] domain The domain of the keys.
 * @param[in] key The key.
 * @param[out] value The value
 *
 * @return @ref ERROR_OK on success,
 * @return @ref ERROR_INVALID_ARGUMENTS Invalid arguments have been passed
 * @return @ref ERROR_DATABASE_INVALID The database is invalid, i.e one of the
 *  queries failed.
 * @return @ref ERROR_DATABASE_NO_SUCH_KEY The domain, key pair does not exist.
 * @return @ref ERROR_DATABASE_TYPE_MISMATCH The value associated to the domain,
 *  key pair is not of the correct type.
 * @return @ref ERROR_MEMORY Out of memory.
 * @return @ref ERROR_UNKNOWN An unspecified error occurred.
 */
int database_get_string(database_handle_t* handle, const char* domain,
    const char* key, char** value);

/**
 * Set the value associated to the domain and key.
 *
 * @param[in] handle A valid database handle.
 * @param[in] domain The domain of the keys.
 * @param[in] key The key.
 * @param[in] value The value
 *
 * @return @ref ERROR_OK on success,
 * @return @ref ERROR_INVALID_ARGUMENTS Invalid arguments have been passed
 * @return @ref ERROR_DATABASE_INVALID The database is invalid, i.e one of the
 *  queries failed.
 * @return @ref ERROR_MEMORY Out of memory.
 */
int database_set_string(database_handle_t* handle, const char* domain,
    const char* key, const char* value);

/**
 * Retrieve the value associated to the domain and key.
 *
 * @param[in] handle A valid database handle.
 * @param[in] domain The domain of the keys.
 * @param[in] key The key.
 * @param[out] value The value
 * @param[out] size The size of value
 *
 * @return @ref ERROR_OK on success,
 * @return @ref ERROR_INVALID_ARGUMENTS Invalid arguments have been passed
 * @return @ref ERROR_DATABASE_INVALID The database is invalid, i.e one of the
 *  queries failed or the referenced file is not a regular file or doesn't exist
 *  or isn't located in the blob-path or any of its subdirectories.
 * @return @ref ERROR_DATABASE_NO_SUCH_KEY The domain, key pair does not exist.
 * @return @ref ERROR_DATABASE_TYPE_MISMATCH The value associated to the domain,
 *  key pair is not of the correct type.
 * @return @ref ERROR_DATABASE_IO Reading from the referenced file failed.
 * @return @ref ERROR_MEMORY Out of memory.
 * @return @ref ERROR_UNKNOWN An unspecified error occurred.
 */
int database_get_blob(database_handle_t* handle, const char* domain,
    const char* key, unsigned char** value, size_t* size);

/**
 * Set the value associated to the domain and key.
 *
 * @param[in] handle A valid database handle.
 * @param[in] domain The domain of the keys.
 * @param[in] key The key.
 * @param[in] value The value
 * @param[in] size The size of value
 *
 * @return @ref ERROR_OK on success,
 * @return @ref ERROR_INVALID_ARGUMENTS Invalid arguments have been passed
 * @return @ref ERROR_DATABASE_INVALID The database is invalid, i.e one of the
 *  queries failed.
 * @return @ref ERROR_DATABASE_IO Writing to the blob file failed.
 * @return @ref ERROR_MEMORY Out of memory.
 */
int database_set_blob(database_handle_t* handle, const char* domain,
    const char* key, const unsigned char* value, size_t size);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // DATABASE_H
