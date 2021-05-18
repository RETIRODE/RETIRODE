/*
 * app_ble_ETSS_int.h
 *
 *  Created on: Mar 31, 2021
 *      Author: Patrik Smolar
 */

#ifndef APP_BLE_ETSS_INT_H
#define APP_BLE_ETSS_INT_H

#ifdef __cplusplus
extern "C"
{
#endif    /* ifdef __cplusplus */



/* ----------------------------------------------------------------------------
 * Include files
 * --------------------------------------------------------------------------*/
#include <app_ble_ests.h>

#include <ble_gap.h>
#include <ble_gatt.h>

/* ----------------------------------------------------------------------------
 * Defines
 * --------------------------------------------------------------------------*/
#define ESTS_CHAR_COMMAND_VALUE_MAX_SIZE         (6)
#define ESTS_CHAR_COMMAND_VALUE_MIN_SIZE         (3)
#define ESTS_CHAR_QUERY_VALUE_SIZE         		 (2)


#define ETSS_MIN_TX_OCTETS              (27)
#define ETSS_NTF_PDU_HEADER_LENGTH      (2)
#define ETSS_NTF_L2CAP_HEADER_LENGTH    (4)
#define ETSS_NTF_ATT_HEADER_LENGTH      (3)
#define ETSS_INFO_OFFSET_LENGTH         (4)


#define ETSS_RFSC_USER_DESC   "Range Finder Send Command"
#define ETSS_RFSQ_USER_DESC        "Range Finder Send Query"
#define ETSS_RFRQ_USER_DESC        "Range Finder Receive Query"



/** List of application specific BLE error codes that can be send in
 * ATT_ERROR_RSP PDUs.
 */
typedef enum ETSS_AttErr_t
{
	ETSS_ATT_ERR_NTF_DISABLED = 0x80
} ETSS_AttErr_t;


/** List of service states
 */
typedef enum ETSS_ServiceState_t
{
	/** Module was not initialized yet. */
	    ESTS_STATE_INIT,

	    /** Module is initialized and ready to accept trigger values. */
	    ESTS_STATE_IDLE,

	    /** Device is connected to a client and able to transmit notifications. */
	    ESTS_STATE_CONNECTED,
} ETSS_ServiceState_t;

/** Stores values required for Send Command Characteristic  */
typedef struct ETSS_RFSC_Characteristic_t
{
    /** Application Write callback. */
	ESTS_ControlHandler callback;

    uint8_t value[ESTS_CHAR_COMMAND_VALUE_MAX_SIZE];


} ETSS_RFSC_Characteristic_t;

/** Stores values required for Send Query Characteristic  */
typedef struct ETSS_RFSQ_Characteristic_t
{
    /** Application Write callback. */
	ESTS_ControlHandler callback;

    uint8_t value[ESTS_CHAR_QUERY_VALUE_SIZE];


} ETSS_RFSQ_Characteristic_t;


/** Stores values required for Receive Query Characteristic  */
typedef struct ETSS_RFRQ_Characteristic_t
{
    /** Client Characteristic Configuration Descriptor Value */
    uint8_t ccc[2];

    uint8_t value[ESTS_CHAR_COMMAND_VALUE_MAX_SIZE];
} ETSS_RFRQ_Characteristic_t;


/**
 * Collects all attribute database related variables of ESTS
 * Service.
 */
typedef struct ETSS_AttDb_t
{
    /**
     * Offset of the attidx of ETSS service attributes.
     *
     * This can change depending on number and order of registeres custom
     * services to shared attribute database.
     */
    uint16_t attidx_offset;

    ETSS_RFSC_Characteristic_t send_command;
    ETSS_RFSQ_Characteristic_t send_query;
    ETSS_RFRQ_Characteristic_t receive_query;
} ETSS_AttDb_t;



typedef struct ESTS_Environment_t
{
    /**
     * Stores all attribute database related variables.
     */
    ETSS_AttDb_t att;

    /**
     * Stores state.
     */
    ETSS_ServiceState_t state;

    /**
	 * Message ID of the first registered message type in kernel.
	 */
     uint16_t msg_id_offset;

} ESTS_Environment_t;

/* ----------------------------------------------------------------------------
 * Global variables and types
 * --------------------------------------------------------------------------*/


#ifdef __cplusplus
}
#endif    /* ifdef __cplusplus */

#endif    /* APP_BLE_ETSS_INT_H */
