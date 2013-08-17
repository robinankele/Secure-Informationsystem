/**
 * @brief HMAC-SHA-1
 *
 * This file contains the implementation of the hmac function
 *
 *  Citation/Origin:
 *      Title: HMAC: Keyed-Hashing for Message Authentification
 *      Author: Hugo Krawczyk
 *      Date: 19.12.2012
 *      URL: https://www.ietf.org/rfc/rfc2104.txt
 * 
 * @file hmac.c
 */

#include <string.h>
#include <strings.h>
#include "hmac.h"
#include "sha1_impl.h"
#include "sha1.h"
#include "../../errors.h"
#include "../../memory.h"

int 
hmac(const unsigned char* key, size_t keysize, const unsigned char* message, 
     size_t messagesize, unsigned char* hmac)
{
  if(key == NULL || keysize == 0 || message == NULL || messagesize == 0 || 
     hmac == NULL)
    return ERROR_INVALID_ARGUMENTS;

  unsigned char k_ipad[65];    /* inner padding - key XORd with ipad */
  unsigned char k_opad[65];    /* outer padding - key XORd with opad */
  unsigned char tk[20];

  /* if key is longer than 64 bytes reset it to key=MD5(key) */
  if (keysize > 64) {
    if(sha1(key, keysize, tk) != ERROR_OK)
      return ERROR_UNKNOWN;

    key = tk;
    keysize = 20;
  }

  /*
   * the HMAC_SHA1 transform looks like:
   *
   * SHA1(K XOR opad, SHA1(K XOR ipad, text))
   *
   * where K is an n byte key
   * ipad is the byte 0x36 repeated 64 times
   * opad is the byte 0x5c repeated 64 times
   * and text is the data being protected
   */

  /* start out by storing key in pads */
  bzero( k_ipad, sizeof k_ipad);
  bzero( k_opad, sizeof k_opad);
  bcopy( key, k_ipad, keysize);
  bcopy( key, k_opad, keysize);

  /* XOR key with ipad and opad values */
  int i;
  for (i=0; i<64; i++) {
    k_ipad[i] ^= 0x36;
    k_opad[i] ^= 0x5c;
  }

  /*
   * perform inner SHA1
   */
  SHA1Context context;
  if(SHA1Reset(&context) != shaSuccess ||                       /* init context for 1st pass */
     SHA1Input(&context, k_ipad, 64) != shaSuccess ||           /* start with inner pad */
     SHA1Input(&context, message, messagesize) != shaSuccess || /* then text of datagram */
     SHA1Result(&context, hmac) != shaSuccess)                  /* finish up 1st pass */
    return ERROR_UNKNOWN;
      
  /*
   * perform outer SHA1
   */
  if(SHA1Reset(&context) != shaSuccess ||                       /* init context for 2nd pass */
     SHA1Input(&context, k_opad, 64) != shaSuccess ||           /* start with outer pad */
     SHA1Input(&context, hmac, 20) != shaSuccess ||             /* then results of 1st hash */
     SHA1Result(&context, hmac) != shaSuccess)                  /* finish up 2nd pass */
    return ERROR_UNKNOWN;        
  
  return ERROR_OK;
}

int
hmac_verify(const unsigned char* key, size_t keysize,
            const unsigned char* message, size_t messagesize,
            const unsigned char* hmac_value)
{
  if(key == NULL || keysize == 0 || message == NULL ||  messagesize == 0 || 
     hmac_value == NULL)
    return ERROR_INVALID_ARGUMENTS;

  unsigned char* hmac_compare_value = NULL;
  if(requestMemory((void**)&hmac_compare_value, SHA1_BLOCKSIZE * sizeof(char)) != ERROR_OK)
    return ERROR_MEMORY;

  if(hmac(key, keysize, message, messagesize, hmac_compare_value) != ERROR_OK){
    freeMemory(hmac_compare_value);
    return ERROR_HMAC_VERIFICATION_FAILED;
  }

  /* check if hmac diggest is correct*/
  uint64_t i = 0;
  for(; i < SHA1_BLOCKSIZE; i++)
    if(hmac_value[i] != hmac_compare_value[i]){
      freeMemory(hmac_compare_value); 
      return ERROR_HMAC_VERIFICATION_FAILED;
    }

  freeMemory(hmac_compare_value);
  
  return ERROR_OK;
}

