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
#include <rsl10.h>
#include <USART_RSLxx.h>
#include "app_circbuf.h"
#include <app_ble_rmts.h>
#include <app_ble_ests.h>
#include "app_ble_peripheral_server.h"
#include "calibration.h"
#include <ble_gap.h>
#include <ble_gatt.h>
#include <retirode_rmp.h>
#include <rwble_hl_error.h>

/* ----------------------------------------------------------------------------
 * Defines
 * --------------------------------------------------------------------------*/
/* Buffer offset */
#define BUFFER_OFFSET                   5

/**
 * Size of measurement to be done before sending data over BLE
 */
#define MEASURE_SIZE   					4
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
     * Circular buffer for temporary storage of image data before it is
     * transmitted over BLE.
     */
    CIRCBUF_t data_cache;

    /**
     * True if measurement is in progress.
     */
    bool measurement_in_progress;

    /**
     * True if stop measurement command was received.
     */
    bool stop_measurement_command;

    /** Store measured data size on application level.
     *
     * Used to approximate image data transfer speed over BLE.
     */
    uint32_t data_size;
} APP_Environemnt_t;

/* ---------------------------------------------------------------------------
* Function prototype definitions
* --------------------------------------------------------------------------*/
void Usart_EventCallBack(uint32_t event);
void APP_RMTS_EventHandler(RMTS_ControlPointOpCode_t opcode,
        const void *p_param);
void APP_ESTS_EventHandler(ESTS_RF_SETTING_ID_t sidx,
        const void *p_param);
void RETIRODE_RMP_Handler(RETIRODE_RMP_Event_t event,
        						const void *p_param);
void RMP_Initialize(void);
void Device_Initialize(void);

/* ----------------------------------------------------------------------------
 * Close the 'extern "C"' block
 * ------------------------------------------------------------------------- */
#ifdef __cplusplus
}
#endif    /* ifdef __cplusplus */

#endif    /* APP_H */
