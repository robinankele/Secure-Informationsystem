#ifndef HMAC_H
#define HMAC_H

/**
 * @brief HMAC-SHA-1
 *
 * @file hmac.h
 */

#include <stddef.h>

/**
 * This function calculates a hash-based message authentication code for a given
 * message based on the HMAC-SHA-1. The @a key of length @a keysize is used to
 * calculate the HMAC-SHA-1 of @a message of length @a messagesize. The result
 * is saved in the buffer @a hmac that has the size of @ref SHA1_BLOCKSIZE.
 *
 * The function returns @ref ERROR_INVALID_ARGUMENTS if any of the given
 * parameters is invalid (invalid buffer pointers, a key or message size of
 * zero). If the function for some reason runs out of memory, @ref ERROR_MEMORY
 * will be returned. If the calculation of the HMAC succeded, @ref ERROR_OK will
 * be returned and if the function fails for any other reason, @ref
 * ERROR_UNKNOWN will be returned.
 *
 * See http://www.ietf.org/rfc/rfc2104.txt for the definition and analysis of
 * the HMAC construction.
 *
 * @param[in] key The used key
 * @param[in] keysize The length of the key
 * @param[in] message The input message
 * @param[in] messagesize The length of the input message
 * @param[out] hmac Location where the calculated HMAC-SHA-1 value is stored.
 *  The caller has to make sure that the buffer fits at least @ref SHA1_BLOCKSIZE
 *  bytes
 *
 * @return @ref ERROR_OK No error occurred
 * @return @ref ERROR_MEMORY Out of memory
 * @return @ref ERROR_INVALID_ARGUMENTS Invalid arguments have been passed
 * @return @ref ERROR_UNKNOWN Any other error occurred
 */
int hmac(const unsigned char* key, size_t keysize,
    const unsigned char* message, size_t messagesize,
    unsigned char* hmac);

/**
 * This functon is used to verify the given HMAC-SHA-1 @a hmac for the given @a
 * message of length @a messagesize and the @a key of length @a keysize.
 *
 * The function returns @ref ERROR_INVALID_ARGUMENTS if any of the given
 * parameters is invalid (invalid buffer pointers, a key or message of
 * length zero). If the given HMAC-SHA-1 value @a hmac is correct, the function
 * returns @ref ERROR_OK, otherwise @ref ERROR_HMAC_VERIFICATION_FAILED will be
 * returned. If any other unspecified error occurs, this function returns @ref
 * ERROR_UNKNOWN.
 *
 * @param[in] key The used key
 * @param[in] keysize The length of the used key
 * @param[in] message The input message
 * @param[in] messagesize The length of the input message
 * @param[in] hmac The calculated HMAC-SHA-1 value
 *
 * @return @ref ERROR_OK No error occurred and the HMAC-SHA-1 value is correct
 * @return @ref ERROR_HMAC_VERIFICATION_FAILED The passed HMAC-SHA-1 value does not
 *  match for the given input
 * @return @ref ERROR_INVALID_ARGUMENTS Invalid arguments have been passed
 * @return @ref ERROR_UNKNOWN Any other error occurred
 */
int hmac_verify(const unsigned char* key, size_t keysize,
    const unsigned char* message, size_t messagesize,
    const unsigned char* hmac);

#endif
