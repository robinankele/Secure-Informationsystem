#ifndef MEMORY_H
#define MEMORY_H

/** @brief Memory Management
 *
 * This file contains the memory management for 'the registry'.
 *
 * @file memory.h
 */

/**
 * Frees the commited value and sets the pointer to NULL
 *
 * @param handle pointer to some memory
 *
 * @return @ref ERROR_OK No error occured
 */
int freeMemory(void *handle);

/**
 * Requests new memory with size
 *
 * @param handle pointer to some memory
 * @param size The size on the new memory
 *
 * @return @ref ERROR_OK No error occured
 * @return @ref ERROR_MEMORY Out of memory
 */
int requestMemory(void **handle, size_t size);

/**
 * Resizes the memory with the new size
 *
 * @param handle pointer to some memory
 * @param size The size on the new memory
 *
 * @return @ref ERROR_OK No error occured
 * @return @ref ERROR_MEMORY Out of memory
 */
int editMemory(void **handle, size_t size);

#endif // MEMORY_H

