/*
 * app_ble_ESTS.h
 *
 *  Created on: Mar 31, 2021
 *      Author: Patrik Smolar
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
 * UUID Defines
 * --------------------------------------------------------------------------*/

/* 128-bit UUID for the External Sensor Trigger Service  */
#define ESTS_SVC_UUID \
    { 0x03, 0x00, 0x13, 0xac, 0x42, 0x02, \
      0xb3, 0xa8, \
      0xeb, 0x11, \
      0x7b, 0x9d, \
      0xf8, 0x3f, 0x5c, 0x0b }

/* 128-bit UUID for the External Sensor Trigger Service - Range finder send command Characteristic */
#define ESTS_RFSC_UUID \
    { 0x03, 0x00, 0x13, 0xac, 0x42, 0x02, \
      0xb3, 0xa8, \
      0xeb, 0x11, \
      0x7b, 0x9d, \
      0x78, 0x42, 0x5c, 0x0b }

/* 128-bit UUID for the External Sensor Trigger Service - Range finder send query Characteristic*/
#define ESTS_RFSQ_UUID \
    { 0x03, 0x00, 0x13, 0xac, 0x42, 0x02, \
      0xb3, 0xa8, \
      0xeb, 0x11, \
      0x7b, 0x9d, \
      0x72, 0x43, 0x5c, 0x0b }

/* 128-bit UUID for the External Sensor Trigger Service - Range finder recieve query Characteristic*/
#define ESTS_RFRQ_UUID \
    { 0x03, 0x00, 0x13, 0xac, 0x42, 0x02, \
      0xb3, 0xa8, \
      0xeb, 0x11, \
      0x7b, 0x9d, \
      0x44, 0x44, 0x5c, 0x0b }


/* ----------------------------------------------------------------------------
 * Error codes
 * --------------------------------------------------------------------------*/
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


/* ----------------------------------------------------------------------------
 * Attributes Table
 * --------------------------------------------------------------------------*/
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


/* ----------------------------------------------------------------------------
 * Main Settings Enum
 * --------------------------------------------------------------------------*/
typedef enum ESTS_RF_SETTING_ID_t
{
    /**
     * SW RESET
     */
    ESTS_OP_SW_RESET = 0x00,

    /**
     * LASER VOLTAGE
     */
    ESTS_OP_LASER_VOLTAGE = 0x01,

	/**
	 * SiPM BIAS POWER VOLTAGE
	 */
	ESTS_OP_S_BIAS_POWER_VOLTAGE = 0x02,

	/**
	 * SiPM CALIBRATE
	 */
	ESTS_OP_CALIBRATE = 0x03,

	/**
	 * PULSE COUNT
	 */
	ESTS_OP_PULSE_COUNT = 0x04,

	/**
	 * Voltages Status
	 */
	ESTS_OP_VOLTAGES_STATUS= 0x05,


	ESTS_RF_SETTING_ID_COUNT

} ESTS_RF_SETTING_ID_t;


/* ----------------------------------------------------------------------------
 * Laser Voltage Types Enum
 * --------------------------------------------------------------------------*/
typedef enum ESTS_LASER_VOLTAGE_TYPE_t
{
	/**
	 * TARGET
	 */
	ESTS_OP_LASER_VOLTAGE_TARGET = 0x01,

	/**
	 * ACTUAL
	 */
	ESTS_OP_LASER_VOLTAGE_ACTUAL = 0x02,

	/**
	 * Turn on/off
	 */
	ESTS_OP_LASER_VOLTAGE_SWITCH = 0x03,

} ESTS_LASER_VOLTAGE_TYPE_t;


/* ----------------------------------------------------------------------------
 * SiPM Bias Power Voltage Types Enum
 * --------------------------------------------------------------------------*/
typedef enum ESTS_S_BIAS_POWER_VOLTAGE_TYPE_t
{
	/**
	 * TARGET
	 */
	ESTS_OP_S_BIAS_POWER_VOLTAGE_TARGET = 0x01,

	/**
	 * ACTUAL
	 */
	ESTS_OP_S_BIAS_POWER_VOLTAGE_ACTUAL = 0x02,

	/**
	 * Turn on/off
	 */
	ESTS_OP_S_BIAS_POWER_VOLTAGE_SWITCH = 0x03,

} ESTS_S_BIAS_POWER_VOLTAGE_TYPE_t;


/* ----------------------------------------------------------------------------
 * Calibration Types Enum
 * --------------------------------------------------------------------------*/
typedef enum ESTS_CALIBRATE_TYPE_t
{
	/**
	 * 0,0 ns
	 */
	ESTS_OP_CALIBRATE_FIRST = 0x01,

	/**
	 * 62,5 ns
	 */
	ESTS_OP_CALIBRATE_SECOND = 0x02,

	/**
	 * 125,0 ns
	 */
	ESTS_OP_CALIBRATE_THIRD = 0x03,

	/**
	* Calibration done
	*/
	ESTS_OP_CALIBRATE_DONE = 0x04

} ESTS_CALIBRATE_TYPE_t;


/* ----------------------------------------------------------------------------
 * Pulse Count Types Enum
 * --------------------------------------------------------------------------*/
typedef enum ESTS_PULSE_COUNT_TYPE_t
{
	/**
	 * VALUE
	 */
	ESTS_OP_PULSE_COUNT_VALUE = 0x01,

} ESTS_PULSE_COUNT_TYPE_t;


/* ----------------------------------------------------------------------------
 * Voltages Status Types Enum
 * --------------------------------------------------------------------------*/
typedef enum ESTS_OP_VOLTAGES_STATUS_TYPE_t
{
	/**
	 * VALUE
	 */
	ESTS_OP_VOLTAGES_STATUS_VALUE = 0x01,

} ESTS_OP_VOLTAGES_STATUS_TYPE_t;


/* ----------------------------------------------------------------------------
 * SW Reset request/response structure
 * --------------------------------------------------------------------------*/
typedef struct ESTS_OP_SW_RESET_params_t
{
    /** Condition if came from query characteristic */
    bool is_query;

    /** Type of SW RESET
     *      *
     * Only 0x00 is allowed
     *
     *  */
    uint8_t type;

    /** Query response value or command value
     *
     * Only 0x00 is allowed
     *
     * */
    uint32_t value;

} ESTS_OP_SW_RESET_params_t;


/* ----------------------------------------------------------------------------
 * Laser Voltage request/response structure
 * --------------------------------------------------------------------------*/
typedef struct ETSS_LASER_VOLTAGE_params_t
{
    /** Condition if came from query characteristic */
    bool is_query;

    /** Type of LASER VOLTAGE
     *      *
     * ESTS_OP_LASER_VOLTAGE_ACTUAL = 0x01
     * ESTS_OP_LASER_VOLTAGE_TARGET = 0x02
     * ESTS_OP_LASER_VOLTAGE_SWITCH = 0x03
     *
     *  */
    uint8_t type;

    /** Query response value or command value*/
    uint32_t value;

} ETSS_LASER_VOLTAGE_params_t;


/* ----------------------------------------------------------------------------
 * SiPM Bias Power Voltage request/response structure
 * --------------------------------------------------------------------------*/
typedef struct ETSS_S_BIAS_POWER_VOLTAGE_params_t
{
    /** Condition if came from query characteristic */
    bool is_query;

    /** Type of S_BIAS_POWER_VOLTAGE
     *      *
     * ESTS_OP_S_BIAS_POWER_VOLTAGE_ACTUAL = 0x01
     * ESTS_OP_S_BIAS_POWER_VOLTAGE_TARGET = 0x02
     * ESTS_OP_LASER_VOLTAGE_SWITCH = 0x03
     *
     *  */
    uint8_t type;

    /** Query response value or command value*/
    uint32_t value;

} ETSS_S_BIAS_POWER_VOLTAGE_params_t;


/* ----------------------------------------------------------------------------
 * Calibration request/response structure
 * --------------------------------------------------------------------------*/
typedef struct ETSS_CALIBRATE_params_t
{
    /** Condition if came from query characteristic */
    bool is_query;

    /** Type of CALIBRATE
     *      *
     * ESTS_OP_CALIBRATE_FIRST = 0x01
     * ESTS_OP_CALIBRATE_SECOND = 0x02
     * ESTS_OP_CALIBRATE_THIRD = 0x03
     * ESTS_OP_CALIBRATE_DONE = 0x04
     *
     *  */
    uint8_t type;

    /** Query response value or command value*/
    uint32_t value;

} ETSS_CALIBRATE_params_t;


/* ----------------------------------------------------------------------------
 * Pulse Count request/response structure
 * --------------------------------------------------------------------------*/
typedef struct ETSS_PULSE_COUNT_params_t
{
    /** Condition if came from query characteristic */
    bool is_query;

    /** Type of PULSE_COUNT
     *      *
     * ESTS_OP_PULSE_COUNT_VALUE = 0x01
     *
     *  */
    uint8_t type;

    /** Query response value or command value*/
    uint8_t value;

} ETSS_PULSE_COUNT_params_t;


/* ----------------------------------------------------------------------------
 * Voltages Status request/response structure
 * --------------------------------------------------------------------------*/
typedef struct ESTS_VOLTAGES_STATUS_params_t
{
    /** Condition if came from query characteristic */
    bool is_query;

    /** Type of PULSE_COUNT
	 *
	 * ESTS_OP_VOLTAGES_STATUS_VALUE = 0x01
	 *
	 *  */
    uint8_t type;

    /** Query response value or command value*/
    uint32_t value;

} ESTS_VOLTAGES_STATUS_params_t;




typedef void (*ESTS_ControlHandler)(ESTS_RF_SETTING_ID_t sidx,
        const void *p_param);


/* ----------------------------------------------------------------------------
 * Function prototype definitions
 * --------------------------------------------------------------------------*/

/**
 * Initialize the External Sensor Trigger Service.
 *
 * Sets initial values of all service related characteristics and descriptors.
 * All service related attributes are then added to attribute database.
 *
 * @pre
 * The Peripheral Server library was initialized with sufficient attribute
 * database size using APP_BLE_PeripheralServerInitialize .
 *
 * @param control_event_handler
 * Application provided callback that will be called for any request on
 * Send Command Char. or Send Query Char.
 *
 * @return
 * 0  - On success.
 * -1 - Attribute database does not have enough free space to fit all ESTSS
 *      attributes.
 */
int32_t ESTS_Initialize(ESTS_ControlHandler control_event_handler);


/**
 * Notify connected peer with response
 *
 * Peer create request on Send Query Characteristic and via this can
 * receive response on Receive Query Characteristic
 *
 * @pre
 * Peer created request on Send Query Char -> ESTS handler in application forwarded
 * request to Range Finder -> Range finder created response -> Application created p_param *
 *
 * @param p_param
 * Response Query message
 * <1byte ESTS_RF_SETTING_ID_t>, <1byte ESTS_..._TYPE_t>, <1byte/4byte value>
 *
 * @return
 * 0 If OK
 * -1 If Error
 */
int32_t ESTS_NOTIFY_QUERY_RESPONSE(const uint8_t *p_param);


#ifdef __cplusplus
}
#endif    /* ifdef __cplusplus */

#endif    /* APP_BLE_ESTS_H */

