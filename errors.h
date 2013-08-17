#ifndef ERRORS_H
#define ERRORS_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

enum {
  ERROR_OK = 0,
  ERROR_UNKNOWN,
  ERROR_MEMORY,
  ERROR_INVALID_ARGUMENTS,
  ERROR_EOF,

  ERROR_BPACK_INVALID_FORMAT_STRING,
  ERROR_BPACK_WRITE,
  ERROR_BPACK_READ,
  ERROR_BUNPACK_INVALID_DATA,

  ERROR_CHANNEL_BUSY,
  ERROR_CHANNEL_FAILED,

  ERROR_REGISTRY_NO_SUCH_KEY,
  ERROR_REGISTRY_UNKNOWN_IDENTIFIER,
  ERROR_REGISTRY_INVALID_STATE,

  ERROR_DATABASE_OPEN,
  ERROR_DATABASE_INVALID,
  ERROR_DATABASE_NO_SUCH_KEY,
  ERROR_DATABASE_IO,
  ERROR_DATABASE_TYPE_MISMATCH,
  ERROR_DATABASE_TYPE_UNKNOWN,

  ERROR_SERVER_INIT,
  ERROR_SERVER_SHUTDOWN,
  ERROR_SERVER_PROCESS,

  ERROR_HMAC_VERIFICATION_FAILED
};

typedef enum packet_type_e {
  PACKET_INVALID,
  PACKET_OK,
  PACKET_ERROR,
  PACKET_INT,
  PACKET_GET_INT,
  PACKET_SET_INT,
  PACKET_DOUBLE,
  PACKET_GET_DOUBLE,
  PACKET_SET_DOUBLE,
  PACKET_STRING,
  PACKET_GET_STRING,
  PACKET_SET_STRING,
  PACKET_BLOB,
  PACKET_GET_BLOB,
  PACKET_SET_BLOB,
  PACKET_ENUM,
  PACKET_GET_ENUM,
  PACKET_TYPE,
  PACKET_GET_VALUE_TYPE,
  PACKET_SHUTDOWN
} packet_type_t;

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // ERRORS_H
