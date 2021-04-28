/*
 * app_ble_RMTS.h
 *
 *  Created on: Mar 31, 2021
 *      Author: patos
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

#define RMTS_SVC_UUID \
    { 0x03, 0x00, 0x13, 0xAC, 0x42, 0x02, \
      0xCD, 0x8D, \
      0xEB, 0x11, \
      0xE6, 0x8C, \
      0x0A, 0xDB, 0x77, 0x51 }

#define RMTS_CHAR_CONTROL_POINT_UUID \
    { 0x03, 0x00, 0x13, 0xAC, 0x42, 0x02, \
      0xCD, 0x8D, \
      0xEB, 0x11, \
      0xE6, 0x8C, \
      0x8E, 0xDE, 0x77, 0x51 }

#define RMTS_CHAR_TOFD_UUID \
    { 0x03, 0x00, 0x13, 0xAC, 0x42, 0x02, \
      0xCD, 0x8D, \
      0xEB, 0x11, \
      0xE6, 0x8C, \
      0x8A, 0xDD, 0x77, 0x51 }

#define RMTS_CHAR_INFO_UUID \
		{  0x03, 0x00, 0x13, 0xAC, 0x42, 0x02, \
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

typedef enum RMTS_ControlPointOpCode_t
{

    /**
     * Generated when an start command is received over BLE.
     */
    RMTS_OP_START_REQ = 1,

    /**
     * Generated when cancel command is received over BLE from peer
     * device.
     */
    RMTS_OP_CANCEL_REQ,


	RMTS_OP_DATA_TRANSFER_REQ,


    /**
     * Generated during MR_DATA transfer to inform application that RMTS is
     * ready to accept more MR_DATA.
     */
   RMTS_OP_DATA_SPACE_AVAIL_IND,
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

int32_t RMTS_Initialize(RMTS_ControlHandler control_event_handler);

int32_t RMTS_Start_TOFD_Transfer(uint32_t tofd_size);

int32_t RMTS_Abort_TOFD_Transfer(RMTS_InfoErrorCode_t errcode);

int32_t RMTS_GetMax_TOFD_PushSize(void);

int32_t RMTS_TOFD_Push(const uint8_t* p_tofd, const int32_t data_len);


#ifdef __cplusplus
}
#endif    /* ifdef __cplusplus */

#endif    /* APP_BLE_RMTS_H */

