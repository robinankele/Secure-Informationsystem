#ifndef SHA1_H
#define SHA1_H

/**
 * @brief SHA-1
 *
 * @file sha1.h
 */

#include <stddef.h>
#include <stdint.h>

/**
 * Size of the hash in bytes.
 */
#define SHA1_BLOCKSIZE 20

/**
 * This function calculates the SHA-1 hash sum for the given input @a data
 * and its given length @a len. The 160 bits result will be written
 * into the first 20 bytes of @a res, a buffer that has to be allocated by the
 * caller of this function.
 *
 * If and only if the calculation of the SHA-1 hash sum succeeded, this function
 * will return @ref ERROR_OK, otherwise @ref ERROR_UNKNOWN or @ref
 * ERROR_INVALID_ARGUMENTS (Invalid buffer pointers or a passed length of 0)
 * will be returned.
 *
 * See http://www.ietf.org/rfc/rfc3174.txt for the defined standard and an
 * example C implementation.
 *
 * @param[in] data The input data
 * @param[in] len The length of the input data @a data
 * @param[out] res The buffer where the SHA-1 hash sum will be written to
 *
 * @return @ref ERROR_OK on success
 * @return @ref ERROR_UNKNOWN on failure
 * @return @ref ERROR_INVALID_ARGUMENTS If invalid arguments have been passed
 */
int sha1(const unsigned char* data, size_t len, uint8_t *res);

#endif
