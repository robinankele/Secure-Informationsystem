/** @brief Memory Management
 *
 * This file contains the memory management for 'the registry'.
 *
 * @file memory.c
 */

#include "errors.h"
#include <stdlib.h>

/**
 * Frees the commited value and sets the pointer to NULL
 *
 * @param handle pointer to some memory
 *
 * @return @ref ERROR_OK No error occured
 */
int
freeMemory(void *handle)
{
  if(handle != NULL)
    free(handle);
  handle = NULL;
  return ERROR_OK;
}

/**
 * Requests new memory with size
 *
 * @param handle pointer to some memory
 * @param size The size on the new memory
 *
 * @return @ref ERROR_OK No error occured
 * @return @ref ERROR_MEMORY Out of memory
 */
int
requestMemory(void **handle, size_t size)
{
  void *buffer;
  buffer = malloc(size);
  if(buffer == NULL)
    return ERROR_MEMORY;
 
  *handle = buffer;
  return ERROR_OK;
}

/**
 * Resizes the memory with the new size
 *
 * @param handle pointer to some memory
 * @param size The size on the new memory
 *
 * @return @ref ERROR_OK No error occured
 * @return @ref ERROR_MEMORY Out of memory
 */
int
editMemory(void **handle, size_t size)
{
  void* buffer;
  buffer = realloc(*handle, size);
  if(buffer == NULL){
    freeMemory(*handle);
    return ERROR_MEMORY;
  }
  
  *handle = buffer;
  return ERROR_OK;
}
