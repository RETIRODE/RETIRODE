/*
 * app_ble_ESTS.h
 *
 *  Created on: Mar 31, 2021
 *      Author: patos
 */

#ifndef APP_BLE_ESTS_H
#define APP_BLE_ESTS_H


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

#define ESTS_SVC_UUID \
    { 0x03, 0x00, 0x13, 0xac, 0x42, 0x24, \
      0xb3, 0xa8, \
      0xeb, 0x11, \
      0x7b, 0x9d, \
      0xf8, 0x3f, 0x5c, 0x0b }

/** 128-bit UUID for the External Sensor Trigger Service - Range finder send command */
#define ESTS_RFSC_UUID \
    { 0x03, 0x00, 0x13, 0xac, 0x42, 0x24, \
      0xb3, 0xa8, \
      0xeb, 0x11, \
      0x7b, 0x9d, \
      0x78, 0x42, 0x5c, 0x0b }

/** 128-bit UUID for the External Sensor Trigger Service - Range finder send query */
#define ESTS_RFSQ_UUID \
    { 0x03, 0x00, 0x13, 0xac, 0x42, 0x24, \
      0xb3, 0xa8, \
      0xeb, 0x11, \
      0x7b, 0x9d, \
      0x72, 0x43, 0x5c, 0x0b }

/** 128-bit UUID for the External Sensor Trigger Service - Range finder recieve query */
#define ESTS_RFRQ_UUID \
    { 0x03, 0x00, 0x13, 0xac, 0x42, 0x24, \
      0xb3, 0xa8, \
      0xeb, 0x11, \
      0x7b, 0x9d, \
      0x44, 0x44, 0x5c, 0x0b }



typedef enum ESTS_InfoErrorCode_t
{
    ESTS_INFO_ERR_ABORTED_BY_SERVER = 0x00,
    ESTS_INFO_ERR_ABORTED_BY_CLIENT = 0x01,
} ESTS_InfoErrorCode_t;

typedef enum ESTS_ApiError_t
{
	ESTS_OK = 0,
	ESTS_ERR = -1,
	ESTS_ERR_NOT_PERMITTED = -2,
	ESTS_ERR_INSUFFICIENT_ATT_DB_SIZE = -3,
} ESTS_ApiError_t;

typedef enum ESTS_AttIdx_t
{
    /* External Sensor Trigger Service 0 */
    ATT_ESTS_SERVICE_0,

    /* External Sensor Trigger Service - Range Finder Send Command */
	ATT_ESTS_RANGE_FINDER_SEND_COMMAND_CHAR_0,
	ATT_ESTS_RANGE_FINDER_SEND_COMMAND_VAL_0,
	ATT_ESTS_RANGE_FINDER_SEND_COMMAND_DESC_0,

 /* External Sensor Trigger Service - Range Finder Send Query */
	ATT_ESTS_RANGE_FINDER_SEND_QUERY_CHAR_0,
	ATT_ESTS_RANGE_FINDER_SEND_QUERY_VAL_0,
	ATT_ESTS_RANGE_FINDER_SEND_QUERY_DESC_0,

	/* External Sensor Trigger Service - Range Finder Receive Query */
	ATT_ESTS_RANGE_FINDER_RECEIVE_QUERY_CHAR_0,
	ATT_ESTS_RANGE_FINDER_RECEIVE_QUERY_VAL_0,
	ATT_ESTS_RANGE_FINDER_RECEIVE_QUERY_CCC_0,
	ATT_ESTS_RANGE_FINDER_RECEIVE_QUERY_DESC_0,

    /* Total number of all custom attributes of ESTS. */
    ATT_ESTS_COUNT,
} ESTS_AttIdx_t;


typedef enum ESTS_RF_SETTING_ID_t
{

    /**
     * SW RESET
     */
    ESTS_OP_SW_RESET,

    /**
     * LASER VOLTAGE
     */
    ESTS_OP_LASER_VOLTAGE,

	/**
	 * SiPM BIAS POWER VOLTAGE
	 */
	ESTS_OP_S_BIAS_POWER_VOLTAGE,

	/**
	 * SiPM CALIBRATE
	 */
	ESTS_OP_CALIBRATE,

	/**
	 * PULSE COUNT
	 */
	ESTS_OP_PULSE_COUNT,

	ESTS_RF_SETTING_ID_COUNT

} ESTS_RF_SETTING_ID_t;


typedef enum ESTS_LASER_VOLTAGE_TYPE_t
{
	/**
	 * ACTUAL
	 */
	ESTS_OP_LASER_VOLTAGE_ACTUAL = 1,

	/**
	 * TARGET
	 */
	ESTS_OP_LASER_VOLTAGE_TARGET,

} ESTS_LASER_VOLTAGE_TYPE_t;

typedef enum ESTS_S_BIAS_POWER_VOLTAGE_TYPE_t
{
	/**
	 * ACTUAL
	 */
	ESTS_OP_S_BIAS_POWER_VOLTAGE_ACTUAL = 1,

	/**
	 * TARGET
	 */
	ESTS_OP_S_BIAS_POWER_VOLTAGE_TARGET,

} ESTS_S_BIAS_POWER_VOLTAGE_TYPE_t;

typedef enum ESTS_CALIBRATE_TYPE_t
{
	/**
	 * 0,0 ns
	 */
	ESTS_OP_CALIBRATE_FIRST = 1,

	/**
	 * 62,5 ns
	 */
	ESTS_OP_CALIBRATE_SECOND = 1,

	/**
	 * 125,0 ns
	 */
	ESTS_OP_CALIBRATE_THIRD = 1,



} ESTS_CALIBRATE_TYPE_t;

typedef enum ESTS_PULSE_COUNT_TYPE_t
{
	/**
	 * VALUE
	 */
	ESTS_OP_PULSE_COUNT_VALUE = 1,

} ESTS_PULSE_COUNT_TYPE_t;

typedef struct ESTS_OP_SW_RESET_params_t
{
    /** Condition if came from query characteristic */
    bool is_query;

    /** Type of SW RESET
     *      *
     * Only 0 is allowed
     *
     *  */
    uint8_t type;

    /** Query response value or command value*/
    uint8_t value;

} ESTS_OP_SW_RESET_params_t;


typedef struct ETSS_LASER_VOLTAGE_params_t
{
    /** Condition if came from query characteristic */
    bool is_query;

    /** Type of LASER VOLTAGE
     *      *
     * ESTS_OP_LASER_VOLTAGE_ACTUAL = 1
     * ESTS_OP_LASER_VOLTAGE_TARGET = 2
     *
     *  */
    uint8_t type;

    /** Query response value or command value*/
    uint8_t value;

} ETSS_LASER_VOLTAGE_params_t;

typedef struct ETSS_S_BIAS_POWER_VOLTAGE_params_t
{
    /** Condition if came from query characteristic */
    bool is_query;

    /** Type of S_BIAS_POWER_VOLTAGE
     *      *
     * ESTS_OP_S_BIAS_POWER_VOLTAGE_ACTUAL = 1
     * ESTS_OP_S_BIAS_POWER_VOLTAGE_TARGET = 2
     *
     *  */
    uint8_t type;

    /** Query response value or command value*/
    uint8_t value;

} ETSS_S_BIAS_POWER_VOLTAGE_params_t;

typedef struct ETSS_CALIBRATE_params_t
{
    /** Condition if came from query characteristic */
    bool is_query;

    /** Type of CALIBRATE
     *      *
     * ESTS_OP_CALIBRATE_FIRST = 1
     * ESTS_OP_CALIBRATE_SECOND = 2
     * ESTS_OP_CALIBRATE_THIRD = 3
     *
     *  */
    uint8_t type;

    /** Query response value or command value*/
    uint8_t value;

} ETSS_CALIBRATE_params_t;

typedef struct ETSS_PULSE_COUNT_params_t
{
    /** Condition if came from query characteristic */
    bool is_query;

    /** Type of PULSE_COUNT
     *      *
     * STS_OP_PULSE_COUNT_VALUE = 1
     *
     *  */
    uint8_t type;

    /** Query response value or command value*/
    uint8_t value;

} ETSS_PULSE_COUNT_params_t;


typedef void (*ESTS_ControlHandler)(ESTS_RF_SETTING_ID_t sidx,
        const void *p_param);


/* ----------------------------------------------------------------------------
 * Function prototype definitions
 * --------------------------------------------------------------------------*/

int32_t ESTS_Initialize(ESTS_ControlHandler control_event_handler);
int32_t ESTS_NOTIFY_QUERY_RESPONSE(const uint8_t *p_param);


#ifdef __cplusplus
}
#endif    /* ifdef __cplusplus */

#endif    /* APP_BLE_ESTS_H */

