#include "communication/bpack.h"
#include "errors.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <stdint.h>
#include <errno.h>

#define DOUBLE_NAN 0x02
#define DOUBLE_INF 0x04
#define DOUBLE_NEG 0x01
#define DOUBLE_ZERO 0x08

/* we are on a little endian platform so these are no-ops */

static uint64_t
htole64(uint64_t v)
{
  return v;
}

static uint64_t
le64toh(uint64_t v)
{
  return v;
}

static uint16_t
htole16(uint16_t v)
{
  return v;
}

static uint16_t
le16toh(uint16_t v)
{
  return v;
}

/* here we have to do something */

static uint64_t
htobe64(uint64_t v)
{
  return ((v & 0x00000000000000FFULL) << 56) |
    ((v & 0x000000000000FF00ULL) << 40) |
    ((v & 0x0000000000FF0000ULL) << 24) |
    ((v & 0x00000000FF000000ULL) << 8) |
    ((v & 0x000000FF00000000ULL) >> 8) |
    ((v & 0x0000FF0000000000ULL) >> 24) |
    ((v & 0x00FF000000000000ULL) >> 40) |
    ((v & 0xFF00000000000000ULL) >> 56);
}

static uint64_t
be64toh(uint64_t v)
{
  /* just do it again */
  return htobe64(v);
}

static uint16_t
htobe16(uint16_t v)
{
  return ((v & 0x00FF) << 8) |
    ((v & 0xFF00) >> 8);
}

static uint16_t
be16toh(uint16_t v)
{
  return htobe16(v);
}

static int
bpack_v(data_store_t* datastore, const char* fmt, va_list ap)
{
  bool little_endian = true;

  while (*fmt) {
    switch (*fmt) {
      case 'l':
        {
          int64_t val = va_arg(ap, int64_t);
          val = little_endian ? (int64_t)htole64(val) : (int64_t)htobe64(val);
          const unsigned char* begin = (const unsigned char*) &val;
          for (size_t s = 0; s != sizeof(int64_t); ++s) {
            if (data_store_write_byte(datastore, begin[s]) != ERROR_OK) {
              return ERROR_BPACK_WRITE;
            }
          }
          break;
        }

      case '<':
        little_endian = true;
        break;

      case '>':
        little_endian = false;
        break;

      case 'd':
        {
          const double val = va_arg(ap, double);

          unsigned char sign = signbit(val) ? DOUBLE_NEG : 0;
          if (isnan(val)) {
            // there is no -NaN
            sign = DOUBLE_NAN;
          } else if (isinf(val)) {
            sign |= DOUBLE_INF;
          } else if (fabs(val) == 0.0) {
            sign |= DOUBLE_ZERO;
          }
          if (data_store_write_byte(datastore, sign) != ERROR_OK) {
            return ERROR_BPACK_WRITE;
          }
          if (sign & DOUBLE_NAN || sign & DOUBLE_INF || sign & DOUBLE_ZERO) {
            break;
          }

          int exponent = 0;
          const double normalized_mantissa = frexp(fabs(val), &exponent);
          const int16_t exp = little_endian ? htole16(exponent) : htobe16(exponent);
          int64_t mantissa = 0;
          memcpy(&mantissa, &normalized_mantissa, sizeof(double));

          const unsigned char* begin = (const unsigned char*) &exp;
          for (size_t s = 0; s != sizeof(int16_t); ++s) {
            if (data_store_write_byte(datastore, begin[s]) != ERROR_OK) {
              return ERROR_BPACK_WRITE;
            }
          }

          int ret = bpack(datastore, little_endian ? "<l" : ">l", mantissa & 0xfffffffffffffL);
          if (ret != ERROR_OK) {
            return ret;
          }

          break;
        }

      case 's':
        {
          const char* val = va_arg(ap, const char*);
          if (!val) {
            return ERROR_INVALID_ARGUMENTS;
          }
          size_t size = strlen(val);
          int ret = bpack(datastore, little_endian ? "<b" : ">b", size, val);
          if (ret != ERROR_OK) {
            return ret;
          }

          break;
        }

      case 'b':
        {
          const size_t size = va_arg(ap, const size_t);
          const unsigned char* val = va_arg(ap, const unsigned char*);
          if (!val) {
            return ERROR_INVALID_ARGUMENTS;
          }

          int ret = bpack(datastore, little_endian ? "<l" : ">l", (int64_t)size);
          if (ret != ERROR_OK) {
            return ret;
          }

          for (size_t s = 0; s != size; ++s) {
            if (data_store_write_byte(datastore, val[s]) != ERROR_OK) {
              return ERROR_BPACK_WRITE;
            }
          }
          break;
        }

      default:
        return ERROR_BPACK_INVALID_FORMAT_STRING;
    }
    ++fmt;
  }

  return 0;
}

static int
bunpack_v(data_store_t* datastore, const char* fmt, va_list ap)
{
  bool little_endian = true;

  while (*fmt) {
    switch (*fmt) {
      case 'l':
        {
          int64_t* val = va_arg(ap, int64_t*);
          if (!val) {
            return ERROR_INVALID_ARGUMENTS;
          }

          int64_t tmp;
          unsigned char* begin = (unsigned char*)&tmp;
          for (size_t s = 0; s != sizeof(int64_t); ++s) {
            if (data_store_read_byte(datastore, &begin[s]) != ERROR_OK) {
              return ERROR_BPACK_READ;
            }
          }
          *val = little_endian ? (int64_t)le64toh(tmp) : (int64_t)be64toh(tmp);
          break;
        }

      case '<':
        little_endian = true;
        break;

      case '>':
        little_endian = false;
        break;

      case 'd':
        {
          double* val = va_arg(ap, double*);
          if (!val) {
            return ERROR_INVALID_ARGUMENTS;
          }

          unsigned char signbyte = 0;
          if (data_store_read_byte(datastore, &signbyte) != ERROR_OK) {
            return ERROR_BPACK_READ;
          }
          const unsigned char signno = signbyte & ~DOUBLE_NEG;
          if (signno == DOUBLE_NAN) {
            /* there is no -NaN */
            if (signbyte & DOUBLE_NEG) {
              return ERROR_BUNPACK_INVALID_DATA;
            }
            *val = NAN;
            break;
          } else if (signno == DOUBLE_INF) {
            *val = signbyte & DOUBLE_NEG ? -INFINITY : INFINITY;
            break;
          } else if (signno == DOUBLE_ZERO) {
            *val = signbyte & DOUBLE_NEG ? -0.0 : 0.0;
            break;
          } else if (signno != 0) {
            return ERROR_BUNPACK_INVALID_DATA;
          }

          int16_t exponent = 0;
          unsigned char* begin = (unsigned char*)&exponent;
          for (size_t s = 0; s != sizeof(int16_t); ++s) {
            if (data_store_read_byte(datastore, &begin[s]) != ERROR_OK) {
              return ERROR_BPACK_READ;
            }
          }
          exponent = little_endian ? le16toh(exponent) : be16toh(exponent);

          int64_t mantissa = 0;
          int ret = bunpack(datastore, little_endian ? "<l" : ">l", &mantissa);
          if (ret != ERROR_OK) {
            return ret;
          }
          if (mantissa & 0xfff0000000000000L) {
            // mantissa to large
            return ERROR_BUNPACK_INVALID_DATA;
          }
          mantissa |= 0x3fe0000000000000L;

          double normalized_mantissa = 0;
          memcpy(&normalized_mantissa, &mantissa, sizeof(double));

          errno = 0;
          double value = ldexp(normalized_mantissa, exponent);
          if (errno != 0) {
            return ERROR_BUNPACK_INVALID_DATA;
          }

          *val = (signbyte & DOUBLE_NEG) ? -value : value;
          break;
        }

      case 's':
        {
          char** val = va_arg(ap, char**);
          if (!val) {
            return ERROR_INVALID_ARGUMENTS;
          }

          size_t size = 0;
          char* tmp = NULL;
          int ret = bunpack(datastore, little_endian ? "<b" : ">b", &size, &tmp);
          if (ret != ERROR_OK) {
            return ret;
          }

          if (size == SIZE_MAX) {
            // can't allocate that much memory
            free(tmp);
            return ERROR_MEMORY;
          }

          char* tmp2 = realloc(tmp, size + 1);
          if (!tmp2) {
            free(tmp);
            return ERROR_MEMORY;
          }

          tmp2[size] = '\0';
          *val = tmp2;
          break;
        }

      case 'b':
        {
          size_t* size = va_arg(ap, size_t*);
          unsigned char** val = va_arg(ap, unsigned char**);
          if (!val || !size) {
            return ERROR_INVALID_ARGUMENTS;
          }

          uint64_t rs = 0;
          int ret = bunpack(datastore, little_endian ? "<l" : ">l", &rs);
          if (ret != ERROR_OK) {
            return ret;
          }

          if (rs > SIZE_MAX) {
            // can't allocate that much memory
            return ERROR_MEMORY;
          }

          const size_t s = rs;
          unsigned char* tmp = malloc(s);
          if (!tmp && s) {
            return ERROR_MEMORY;
          }

          for (size_t i = 0; i != s; ++i) {
            if (data_store_read_byte(datastore, tmp + i) != ERROR_OK) {
              free(tmp);
              return ERROR_BPACK_READ;
            }
          }
          *val = tmp;
          *size = s;

          break;
        }

      default:
        return ERROR_BPACK_INVALID_FORMAT_STRING;
    }
    ++fmt;
  }

  return 0;
}

int
bpack(data_store_t* datastore, const char* fmt, ...)
{
  if (!datastore || !datastore->write_byte || !fmt) {
    return ERROR_INVALID_ARGUMENTS;
  }

  va_list ap;
  va_start(ap, fmt);
  const int ret = bpack_v(datastore, fmt, ap);
  va_end(ap);

  return ret;
}

int
bunpack(data_store_t* datastore, const char* fmt, ...)
{
  if (!datastore || !datastore->read_byte || !fmt) {
    return ERROR_INVALID_ARGUMENTS;
  }

  va_list ap;
  va_start(ap, fmt);
  const int ret = bunpack_v(datastore, fmt, ap);
  va_end(ap);

  return ret;
}
