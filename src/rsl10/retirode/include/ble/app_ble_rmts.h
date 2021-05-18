/*
 * app_ble_RMTS.h
 *
 *  Created on: Mar 31, 2021
 *      Author: Patrik Smolar
 */

#ifndef APP_BLE_RMTS_H
#define APP_BLE_RMTS_H


#ifdef __cplusplus
extern "C"
{
#endif    /* ifdef __cplusplus */

/* ----------------------------------------------------------------------------
 * Include files
 * --------------------------------------------------------------------------*/
#include <rsl10_ke.h>
#include <gattc_task.h>

/* ----------------------------------------------------------------------------
 * Defines
 * --------------------------------------------------------------------------*/

/* 128-bit UUID for the Range Measurement Transfer Service  */
#define RMTS_SVC_UUID \
    { 0x03, 0x00, 0x13, 0xAC, 0x42, 0x02, \
      0xCD, 0x8D, \
      0xEB, 0x11, \
      0xE6, 0x8C, \
      0x0A, 0xDB, 0x77, 0x51 }

/* 128-bit UUID for the Range Measurement Transfer Service - Control Point Characteristic  */
#define RMTS_CHAR_CONTROL_POINT_UUID \
    { 0x03, 0x00, 0x13, 0xAC, 0x42, 0x02, \
      0xCD, 0x8D, \
      0xEB, 0x11, \
      0xE6, 0x8C, \
      0x8E, 0xDE, 0x77, 0x51 }

/* 128-bit UUID for the Range Measurement Transfer Service - Time Of Flight Data Characteristic  */
#define RMTS_CHAR_TOFD_UUID \
    { 0x03, 0x00, 0x13, 0xAC, 0x42, 0x02, \
      0xCD, 0x8D, \
      0xEB, 0x11, \
      0xE6, 0x8C, \
      0x8A, 0xDD, 0x77, 0x51 }

/* 128-bit UUID for the Range Measurement Transfer Service - Info Characteristic  */
#define RMTS_CHAR_INFO_UUID \
	{ 0x03, 0x00, 0x13, 0xAC, 0x42, 0x02, \
		 0xCD, 0x8D, \
		 0xEB, 0x11, \
		 0xE6, 0x8C, \
		 0x8F, 0xDF, 0x77, 0x51 }

#define RMTS_TOFD_MAX_SIZE         (GAPM_DEFAULT_MTU_MAX - 7)

typedef enum RMTS_ApiError_t
{
	RMTS_OK = 0,
	RMTS_ERR = -1,
	RMTS_ERR_NOT_PERMITTED = -2,
	RMTS_ERR_INSUFFICIENT_ATT_DB_SIZE = -3,
} RMTS_ApiError_t;


/* ----------------------------------------------------------------------------
 * Attributes Table
 * --------------------------------------------------------------------------*/
typedef enum RMTS_AttIdx_t
{
    /* Range Finder Service 0 */
    ATT_RMTS_SERVICE_0,

    /* Range Finder Control Point Characteristic */
	ATT_RMTS_CONTROL_POINT_CHAR_0,
	ATT_RMTS_CONTROL_POINT_VAL_0,
	ATT_RMTS_CONTROL_POINT_DESC_0,

	/* Picture Transfer Info Characteristic */
	ATT_RMTS_INFO_CHAR_0,
	ATT_RMTS_INFO_VAL_0,
	ATT_RMTS_INFO_CCC_0,
	ATT_RMTS_INFO_DESC_0,

    /* Range Finder TOFD Characteristic */
	ATT_RMTS_TOFD_CHAR_0,
	ATT_RMTS_TOFD_VAL_0,
	ATT_RMTS_TOFD_CCC_0,
	ATT_RMTS_TOFD_DESC_0,

    /* Total number of all custom attributes of RMTS. */
    ATT_RMTS_COUNT,
} RMTS_AttIdx_t;


/* ----------------------------------------------------------------------------
 * Attributes Table
 * --------------------------------------------------------------------------*/
typedef enum RMTS_ControlPointOpCode_t
{
	/**
     * Start Command - Range Finder Start measurement and inform app when ready
     */
    RMTS_OP_START_REQ = 0x01,

    /**
     * Stop Command - Data transfer canceled by connected peer
     */
    RMTS_OP_CANCEL_REQ = 0x02,

	/**
	 * Data Transfer Command - Connected peer received data size and start data transfer
	 */
	RMTS_OP_DATA_TRANSFER_REQ = 0x03,

	/**
	 * Generated during data transfer to inform application that ESTS is
	 * ready to accept more data.
	 */
   RMTS_OP_DATA_SPACE_AVAIL_IND = 0x04,


   /**
	* Generated when all data were successfully transferred.
	*/
   RMTS_OP_DATA_TRANSFER_COMPLETED = 0x05,

} RMTS_ControlPointOpCode_t;

typedef enum RMTS_InfoErrorCode_t
{
    RMTS_INFO_ERR_ABORTED_BY_SERVER = 0x00,
    RMTS_INFO_ERR_ABORTED_BY_CLIENT = 0x01,
} RMTS_InfoErrorCode_t;

typedef void (*RMTS_ControlHandler)(RMTS_ControlPointOpCode_t opcode,
        const void *p_param);


/* ----------------------------------------------------------------------------
 * Function prototype definitions
 * --------------------------------------------------------------------------*/


/**
 *
 * @pre
 * BLE stack was initialized using @ref APP_BLE_PeripheralServerInitialize
 * with att_db size of at least ATT_RMTS_COUNT
 *
 * @param control_event_handler
 * Application callback function that informs application of any RMTS related
 * events.
 *
 * @return
 */
int32_t RMTS_Initialize(RMTS_ControlHandler control_event_handler);

/**
 *
 * @pre
 * ESTS was initialized and there is active connection.
 *
 * @param tofd_size
 * Total size of data in bytes.
 */
int32_t RMTS_Start_TOFD_Transfer(uint32_t tofd_size);

/**
 * Inform client that range measurement or transfer operation was aborted with given
 * error code.
 *
 * @param errcode
 * Reason for aborting of the operation.
 *
 * @return
 * RMTS_OK - Abort notification was transmitted. <br>
 * RMTS_ERR_NOT_PERMITTED - There is either no connection established
 */
int32_t RMTS_Abort_TOFD_Transfer(RMTS_InfoErrorCode_t errcode);

/** @return
 * Max size to push
 */
int32_t RMTS_GetMax_TOFD_PushSize(void);


/**
 * Queue data for transmission over BLE.
 *
 * The service automatically merges data to as little packets as possible
 * depending on the currently negotiated MTU for the active connection.
 *
 * @pre
 * Data info was provided using @ref RMTS_Start_TOFD_Transfer before
 * This is necessary in order to determine whether all data were
 * transmitted or not to send last shorter data packet.
 *
 * @param p_tofd
 * Time of flight data
 *
 * @param data_len
 * Number of bytes to queue for transmission.
 *
 * @return
 * Actual number of bytes queued for transmission or 0 if unable to queue any
 * additional bytes.
 * Negative error code on error.
 */
int32_t RMTS_TOFD_Push(const uint8_t* p_tofd, const int32_t data_len);


#ifdef __cplusplus
}
#endif    /* ifdef __cplusplus */

#endif    /* APP_BLE_RMTS_H */

