/* ----------------------------------------------------------------------------
 * Copyright (c) 2020 Semiconductor Components Industries, LLC (d/b/a
 * ON Semiconductor), All Rights Reserved
 *
 * This code is the property of ON Semiconductor and may not be redistributed
 * in any form without prior written permission from ON Semiconductor.
 * The terms of use and warranty for this code are covered by contractual
 * agreements between ON Semiconductor and the licensee.
 * ------------------------------------------------------------------------- */

/**
 * @file app_circbuf.h
 *
 * Implements fixed size circular buffer FIFO for storing of byte arrays.
 */

#ifndef APP_CIRCBUF_H
#define APP_CIRCBUF_H

/* ----------------------------------------------------------------------------
 * If building with a C++ compiler, make all of the definitions in this header
 * have a C binding.
 * ------------------------------------------------------------------------- */
#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

/* ----------------------------------------------------------------------------
 * Include files
 * --------------------------------------------------------------------------*/

#include <stdbool.h>
#include <stdint.h>

/* ----------------------------------------------------------------------------
 * Defines
 * --------------------------------------------------------------------------*/

/** Object structure that holds all information about a circular buffer. */
typedef struct CIRCBUF_t
{
    /**
     * Pointer to underlying byte array that stores data of the circular buffer.
     *
     * Provided during initialization to allow for static allocation.
     */
    uint8_t *p_buf;

    /**
     * ::p_buf array index pointing to next element that will be read on next
     * pop operation.
     */
    size_t head;

    /**
     * ::p_buf array index pointing to next element that will be written on next
     * push operation.
     */
    size_t tail;

    /**
     * Total capacity of the circular buffer in bytes.
     * Equall to the size of the ::p_buf array.
     */
    size_t size;

    /**
     * Flag to indicate if buffer is full or empty when ::head index is equal to
     * ::tail index.
     */
    bool is_full;
} CIRCBUF_t;

/* ----------------------------------------------------------------------------
 * Function declarations
 * --------------------------------------------------------------------------*/

/**
 * Initialize circular buffer with given buffer array.
 *
 * Can be also used to reinitialize existing buffer structures.
 *
 * @pre
 * Following requirements must be met:
 *
 * - `REQUIRE(p_buf != NULL)`
 * - `REQUIRE(buf_size > 0)`
 * - `REQUIRE(obj != NULL)`
 *
 * @post
 * Provides empty circular buffer that uses @p b_buf for storage and @p buf_size
 * is the maximum number of elements that can be stored in the circular
 * buffer.
 *
 * - `ENSURE(obj->p_buf == p_buf)`
 * - `ENSURE(obj->size == buf_size)`
 * - `ENSURE(obj->head == 0)`
 * - `ENSURE(obj->tail == 0)`
 * - `ENSURE(obj->is_full == false)`
 *
 * @param p_buf
 * Pointer to byte array to use for storing of data.
 *
 * @param buf_size
 * Size of byte array @p p_buf in bytes.
 *
 * @param obj
 * Circular buffer object to initialize.
 */
void CIRCBUF_Initialize(uint8_t *p_buf, size_t buf_size, CIRCBUF_t *obj);

/**
 * Checks if circular buffer is empty.
 *
 * @pre
 * Following requirements must be met:
 *
 * - `REQUIRE(obj != NULL)`
 * - @p obj was already initialized using @ref CIRCBUF_Initialize
 *
 * @post
 * - The Circular buffer @p obj is not modified.
 *
 * @param obj
 * Circular buffer object to check.
 *
 * @return
 * true  - Circular buffer @p obj is empty. <br>
 * false - Circular buffer @p obj is not empty.
 */
bool CIRCBUF_IsEmpty(const CIRCBUF_t *obj);

/**
 * Checks if circular buffer is full.
 *
 * @pre
 * Following requirements must be met:
 *
 * - `REQUIRE(obj != NULL)`
 * - @p obj was already initialized using @ref CIRCBUF_Initialize
 *
 * @post
 * - The Circular buffer @p obj is not modified.
 *
 * @param obj
 * Circular buffer object to check.
 *
 * @return
 * true  - Circular buffer @p obj is full. <br>
 * false - Circular buffer @p obj is not full.
 */
bool CIRCBUF_IsFull(const CIRCBUF_t *obj);

/**
 * Returns number of bytes that can be pushed (written) into the circular
 * buffer.
 *
 * @pre
 * Following requirements must be met:
 *
 * - `REQUIRE(obj != NULL)`
 * - @p obj was already initialized using @ref CIRCBUF_Initialize
 *
 * @post
 * - Returned value is not larger than circular buffer size.
 * - The circular buffer @p obj is not modified.
 *
 * @param obj
 * Circular buffer object to check.
 *
 * @return
 * Number of bytes that can be written (pushed) into the circular buffer @p obj.
 */
size_t CIRCBUF_GetFree(const CIRCBUF_t *obj);

/**
 * Returns number of bytes that can be read (pop) from the circular buffer.
 *
 * @pre
 * Following requirements must be met:
 *
 * - `REQUIRE(obj != NULL)`
 * - @p obj was already initialized using @ref CIRCBUF_Initialize
 *
 * @post
 * - Returned value is not larger than circular buffer size.
 * - The circular buffer @p obj is not modified.
 *
 * @param obj
 * Circular buffer object to check.
 *
 * @return
 * Number of bytes that can be read (pop) from the circular buffer @p obj.
 */
size_t CIRCBUF_GetUsed(const CIRCBUF_t *obj);

/**
 * Appends given amount of bytes to the back of the circular buffer.
 *
 * @pre
 * Following requirements must be met:
 *
 * - `REQUIRE(p_data != NULL)`
 * - `REQUIRE(data_size > 0)`
 * - `REQUIRE(obj != NULL)`
 * - @p obj was already initialized using @ref CIRCBUF_Initialize
 *
 * @post
 * - The circular buffer @p obj is modified only if it is able to write
 *   @p data_size bytes.
 *
 * @param p_data
 * Pointer to byte array to be added to the circular buffer.
 *
 * @param data_size
 * Size of @p p_data array in bytes.
 *
 * @param obj
 * Circular buffer object where data will be written.
 *
 * @return
 * 0  - On success. <br>
 * -1 - On failure. If @p data_size is larger than available space in the
 *      buffer.
 */
int32_t CIRCBUF_PushBack(uint8_t *p_data, size_t data_size, CIRCBUF_t *obj);

/**
 * Retrieves given amount of bytes from the front of the circular buffer.
 *
 * @pre
 * Following requirements must be met:
 *
 * - `REQUIRE(p_data != NULL)`
 * - `REQUIRE(data_size > 0)`
 * - `REQUIRE(obj != NULL)`
 * - @p obj was already initialized using @ref CIRCBUF_Initialize
 *
 * @post
 * - The circular buffer @p obj is modified only if it contains at least
 *   @p data_size number of bytes.
 *
 * @param p_data
 * Pointer to byte array to store data read from the circular buffer @p obj .
 *
 * @param data_size
 * Number of bytes to pop from the circullar buffer.
 * @p p_data array must be at least @p data_size bytes long.
 *
 * @param obj
 * Circular buffer object where data will be written.
 *
 * @return
 * 0  - On success. <br>
 * -1 - On failure. If requested more data that is available in the circular
 *      buffer.
 */
int32_t CIRCBUF_PopFront(uint8_t *p_data, size_t data_size, CIRCBUF_t *obj);

/* ----------------------------------------------------------------------------
 * Close the 'extern "C"' block
 * ------------------------------------------------------------------------- */
#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif /* APP_CIRCBUF_H */
