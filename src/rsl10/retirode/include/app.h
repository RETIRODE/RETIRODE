/* ----------------------------------------------------------------------------
 * Copyright (c) 2019 Semiconductor Components Industries, LLC (d/b/a
 * ON Semiconductor), All Rights Reserved
 *
 * This code is the property of ON Semiconductor and may not be redistributed
 * in any form without prior written permission from ON Semiconductor.
 * The terms of use and warranty for this code are covered by contractual
 * agreements between ON Semiconductor and the licensee.
 *
 * This is Reusable Code.
 * ----------------------------------------------------------------------------
 * app.h
 * - Application header file for the uart_cmsis_driver sample
 *   application
 * ------------------------------------------------------------------------- */

#ifndef APP_H
#define APP_H

/* ----------------------------------------------------------------------------
 * If building with a C++ compiler, make all of the definitions in this header
 * have a C binding.
 * ------------------------------------------------------------------------- */
#ifdef __cplusplus
extern "C"
{
#endif    /* ifdef __cplusplus */

/* ----------------------------------------------------------------------------
 * Include files
 * --------------------------------------------------------------------------*/

#include <RTE_Device.h>
#include <GPIO_RSLxx.h>
#include <rsl10.h>
#include <string.h>
#include <USART_RSLxx.h>
#include "app_circbuf.h"
#include <app_ble_rmts.h>
#include <app_ble_ests.h>
#include "app_ble_peripheral_server.h"
#include "calibration.h"
#include <ble_gap.h>
#include <ble_gatt.h>

/* ----------------------------------------------------------------------------
 * Defines
 * --------------------------------------------------------------------------*/
#if !RTE_USART
    #error "Please configure USART0 in RTE_Device.h"
#endif    /* if !RTE_USART0 */


/* DIO number that is used for easy re-flashing (recovery mode) */
#define RECOVERY_DIO                    12

/* GPIO definitions */
#define LED_DIO                         6
#define BUTTON_DIO                      5

/* Buffer offset */
#define BUFFER_OFFSET                   5


/**
 * Size of buffer for storing data before it gets transmitted over BLE.
 *
 * Used mainly to accumulate enough data to send biggest allowed packet size
 * to minimize number of transmitted packets.
 */
#define APP_DATA_CACHE_SIZE             (4 * 64)


/** Structure holding all data managed on application level. */
typedef struct APP_Environemnt_t
{
    /**
     * Flag to indicate if SPI transfer for reading of image data is in
     * progress.
     *
     * Makes sure the data are read only in single chunks to not overwhelm
     * limited buffer storage.
     */
    bool isp_read_in_progress;

    /**
     * Circular buffer for temporary storage of image data before it is
     * transmitted over BLE.
     */
    CIRCBUF_t data_cache;

    /**
     * Timestamp of when application received image capture request from peer
     * device.
     *
     * Used to calculate ISP power up time.
     */
    uint32_t time_capture_req;

    /**
     * Used to calculate time it took ISP to capture image.
     *
     * The capture may be variable due to auto exposure settings of ISP.
     */
    uint32_t time_capture_start;

    /**
     * Used to calculate time of transmission of image data over BLE.
     *
     * Time from receiving of data transfer request to last data packet
     * transmission.
     */
    uint32_t time_transfer_start;

    /** Store captured image size on application level.
     *
     * Used to approximate image data transfer speed over BLE.
     */
    uint32_t data_size;
} APP_Environemnt_t;



/* ----------------------------------------------------------------------------
 * Global variables and types
 * --------------------------------------------------------------------------*/
extern ARM_DRIVER_USART Driver_USART0;
extern DRIVER_GPIO_t Driver_GPIO;

/* ---------------------------------------------------------------------------
* Function prototype definitions
* --------------------------------------------------------------------------*/
void Usart_EventCallBack(uint32_t event);
void Button_EventCallback(uint32_t event);
void Initialize(void);
void ToggleLed(uint32_t n, uint32_t delay_ms);
void APP_RMTS_EventHandler(RMTS_ControlPointOpCode_t opcode,
        const void *p_param);
void APP_ESTS_EventHandler(ESTS_RF_SETTING_ID_t sidx,
        const void *p_param);
void Device_Initialize(void);

/* ----------------------------------------------------------------------------
 * Close the 'extern "C"' block
 * ------------------------------------------------------------------------- */
#ifdef __cplusplus
}
#endif    /* ifdef __cplusplus */

#endif    /* APP_H */
