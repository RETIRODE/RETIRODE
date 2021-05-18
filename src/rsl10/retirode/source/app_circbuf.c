/* ----------------------------------------------------------------------------
 * Copyright (c) 2020 Semiconductor Components Industries, LLC (d/b/a
 * ON Semiconductor), All Rights Reserved
 *
 * This code is the property of ON Semiconductor and may not be redistributed
 * in any form without prior written permission from ON Semiconductor.
 * The terms of use and warranty for this code are covered by contractual
 * agreements between ON Semiconductor and the licensee.
 *
 * This is Reusable Code.
 *
 * ------------------------------------------------------------------------- */

/**
 * @file app_circbuf.c
 *
 * Implements fixed size circular buffer FIFO for storing of byte arrays.
 */


/* ----------------------------------------------------------------------------
 * Include files
 * --------------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>

#include <app_circbuf.h>


/* ----------------------------------------------------------------------------
 * Defines
 * --------------------------------------------------------------------------*/

/* ----------------------------------------------------------------------------
 * Function Declarations
 * --------------------------------------------------------------------------*/

/* ----------------------------------------------------------------------------
 * Types
 * --------------------------------------------------------------------------*/

/* ----------------------------------------------------------------------------
 * Global Variables
 * --------------------------------------------------------------------------*/

/* Stores file name when assertions are enabled. */
;

/* ----------------------------------------------------------------------------
 * Function Definitions
 * --------------------------------------------------------------------------*/

void CIRCBUF_Initialize(uint8_t *p_buf, size_t buf_size, CIRCBUF_t *obj)
{

    obj->p_buf = p_buf;
    obj->size = buf_size;

    obj->head = 0;
    obj->tail = 0;
    obj->is_full = false;


}

bool CIRCBUF_IsEmpty(const CIRCBUF_t *obj)
{


    bool is_empty = (obj->head == obj->tail) && (obj->is_full == false);

    return is_empty;
}

bool CIRCBUF_IsFull(const CIRCBUF_t *obj)
{


    return obj->is_full;
}

size_t CIRCBUF_GetFree(const CIRCBUF_t *obj)
{


    size_t avail = obj->size - CIRCBUF_GetUsed(obj);



    return avail;
}

size_t CIRCBUF_GetUsed(const CIRCBUF_t *obj)
{


    size_t count;

    if (CIRCBUF_IsEmpty(obj) == true)
    {
        count = 0;
    }
    else if (obj->is_full == true)
    {
        count = obj->size;
    }
    else if (obj->tail > obj->head)
    {
        count = obj->tail - obj->head;
    }
    else
    {
        count = obj->size - obj->head;

        if (obj->tail > 0)
        {
            count += (obj->tail - 1);
        }
    }


    return count;
}

int32_t CIRCBUF_PushBack(uint8_t *p_data, size_t data_size, CIRCBUF_t *obj)
{

    if (data_size > CIRCBUF_GetFree(obj))
    {
        /* Not enough space to insert new elements. */
        return -1;
    }

    while (data_size--)
    {
        /* Add new element to tail. */
        obj->p_buf[obj->tail++] = *p_data++;

        /* Rotate tail if end of buffer was reached. */
        if (obj->tail == obj->size)
        {
            obj->tail = 0;
        }
    }

    /* Check if this push operation completely filled up the buffer. */
    if (obj->tail == obj->head)
    {
        obj->is_full = true;
    }

    return 0;
}

int32_t CIRCBUF_PopFront(uint8_t *p_data, size_t data_size, CIRCBUF_t *obj)
{


    if (data_size > CIRCBUF_GetUsed(obj))
    {
        /* Asked for more elements than available. */
        return -1;
    }

    while (data_size--)
    {
        /* Read element from head. */
        *p_data++ = obj->p_buf[obj->head++];

        /* Rotate head if end of buffer was reached. */
        if (obj->head == obj->size)
        {
            obj->head = 0;
        }
    }

    /* At least one element was extracted. */
    obj->is_full = false;


    return 0;
}
