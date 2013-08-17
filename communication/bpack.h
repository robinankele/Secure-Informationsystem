#ifndef BPACK_H
#define BPACK_H

#include <stdint.h>
#include <stddef.h>

#include "datastore.h"

/** @brief Data packing and unpacking
 *
 * bpack is used to pack a bunch of data into some sort of byte-based data
 * store. bunpack unpacks the data from the date store.
 *
 * bpack and unpack use format strings to describe the data. The format string
 * may consist of the following characters:
 *
 *  - l: a 64 bit signed integer
 *  - <: switch integer representation to little-endian
 *  - >: switch integer representation to big-endian
 *  - d: a double precission floating point
 *  - s: a zero-terminated string
 *  - b: a binary blob of some length
 *
 * Any other character is invalid and bpack and bunpack shall fail with
 * ERROR_BPACK_INVALID_FORMAT_STRING in that case.
 *
 * The data is written to the data store in the following way:
 *  - integers: Integers are stored either in their little-endian or big-endian
 *    representation. Per default they are stored in their little-endian
 *    representation.
 *  - doubles: Doubles are stored in the following way: S[EEMMMM]. S shall be
 *    known as sign byte and stores the following information:
 *    - S & 0x01 is true if the sign bit of the double is set.
 *    - S & 0x02 is true if the double is NaN or NaN.
 *    - S & 0x04 is true if the double is Inf or -Inf.
 *    - S & 0x08 is true if the double is 0 or -0.
 *    - all other values for S are invalid.
 *    If S contains any valid value but 0x00, or 0x01, then only only the sign
 *    byte is stored. EE is the 16 bit integer containing the (11 bit) exponent
 *    of the double value. MMMM is the 64 bit integer containing the (52 bit)
 *    normalized mantissa of the absolute vale of the double. The normalized
 *    mantissa is in the internval [0.5,1). If the double is not NaN, Inf or 0
 *    the following relation holds: value == (-1)^(S & 0x01) * MMMM * 2^EE,
 *    where ^ denotes the exponentiation. Please also note that only one of the
 *    following flags can be set at the same time: 0x02, 0x04 and 0x08, so a
 *    number cannot be e.g. -Inf and 0 at the same time.
 *  - strings and blobs: strings and blobs are stored byte by byte and
 *    prefixed by their length as 64-bit unsigned integer.
 *
 * @file bpack.h
 */

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * Packs a bunch of data passed by @a fmt and the following parameters
 * into the given @a datastore.
 *
 * For every character in the format string the correct amount of arguments in
 * the variable arguments list have to be passed. The arguments must match the
 * following requirements for each element in the format string:
 * - l: a int64_t
 * - d: a double
 * - s: a non-NULL char*
 * - b: a pair of size_t and a non-NULL unsigned char*. The first argument gives
 *   the size of the blob.
 *
 * If the above conditions are not met @a bpack returns @ref
 * ERROR_INVALID_ARGUMENTS and packs the arguments up to that point.
 *
 * Any other character than "ldsb<>" is invalid and bpack shall fail
 * with ERROR_BPACK_INVALID_FORMAT_STRING in that case.
 *
 * If writing to the datastore failed, @a bpack returns @ref ERROR_BPACK_WRITE.
 *
 * @param[in] datastore Target data store.
 * @param[in] fmt Packing format.
 *
 * @return @ref ERROR_OK on success
 * @return @ref ERROR_BPACK_INVALID_FORMAT_STRING Invalid format string
 * @return @ref ERROR_BPACK_WRITE Could not write data
 * @return @ref ERROR_INVALID_ARGUMENTS Invalid arguments.
 */
int bpack(data_store_t* datastore, const char* fmt, ...);

/**
 * Unpacks data specified by @a fmt from the given @a datastore
 * into the additional passed parameters.
 *
 * For every character in the format string the correct amount of arguments in
 * the variable arguments list have to be passed. The arguments must match the
 * following requirements for each element in the format string:
 * - l: a non-NULL int64_t*
 * - d: a non-NULL double*
 * - s: a non-NULL char**
 * - b: a pair of non-NULL size_t* and a non-NULL unsigned char**.
 *
 * If the above conditions are not met @a bpack returns @ref
 * ERROR_INVALID_ARGUMENTS and unpacks the arguments up to that point.
 *
 * Any other character than "ldsb<>" is invalid and bunpack shall fail
 * with ERROR_BPACK_INVALID_FORMAT_STRING in that case.
 *
 * If reading from the datastore fails, @a bunpack returns @ref
 * ERROR_BPACK_READ.
 *
 * If the data to be unpacked as double does not meet the
 * the format requirements, @a bunpack returns @ref ERROR_BUNPACK_INVALID_DATA.
 *
 * @a bunpack allocates the memory necessary to hold the \0 terminated strings
 * and blobs.  The caller is responsible to free this memory after use. In case
 * of an error, only the memory allocated for the current position in the format
 * string will be de-allocated by bunpack. The memory for all the previous
 * strings and blobs that were unpacked successfully, has to be deallocated by
 * the caller.
 *
 * The targets of the pointers are only changed after successfully unpacking the
 * value.
 *
 * @param[in] datastore Source data store.
 * @param[in] fmt Packing format.
 *
 * @return @ref ERROR_OK on success
 * @return @ref ERROR_BPACK_INVALID_FORMAT_STRING Invalid format string
 * @return @ref ERROR_BPACK_READ Could not read data
 * @return @ref ERROR_BUNPACK_INVALID_DATA double could not be unpacked
 * @return @ref ERROR_MEMORY Memory allocation failed.
 * @return @ref ERROR_INVALID_ARGUMENTS Invalid arguments.
 */
int bunpack(data_store_t* datastore, const char* fmt, ...);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // BPACK_H
