/*
 * app_ble_RMTS_int.h
 *
 *  Created on: Mar 31, 2021
 *      Author: Patrik Smolar
 */

#ifndef APP_BLE_RMTS_INT_H
#define APP_BLE_RMTS_INT_H

#ifdef __cplusplus
extern "C"
{
#endif    /* ifdef __cplusplus */



/* ----------------------------------------------------------------------------
 * Include files
 * --------------------------------------------------------------------------*/
#include <app_ble_rmts.h>

#include <ble_gap.h>
#include <ble_gatt.h>

/* ----------------------------------------------------------------------------
 * Defines
 * --------------------------------------------------------------------------*/

#define RMTS_CONTROL_POINT_VALUE_LENGTH (1)
#define RMTS_TOFD_CHAR_VALUE_LENGTH GAPM_DEFAULT_TX_OCT_MAX

#define RMTS_MIN_TX_OCTETS              (27)
#define RMTS_NTF_PDU_HEADER_LENGTH      (2)
#define RMTS_NTF_L2CAP_HEADER_LENGTH    (4)
#define RMTS_NTF_ATT_HEADER_LENGTH      (3)
#define RMTS_INFO_OFFSET_LENGTH         (4)


#define RMTS_CONTROL_POINT_USER_DESC   "Control Point"
#define RMTS_TOFD_INFO_USER_DESC        "Info"
#define RMTS_TOFD_USER_DESC        "Time of flight data"

#define RMTS_CONTROL_POINT_OPCODE_START_REQ          (0x01)
#define RMTS_CONTROL_POINT_OPCODE_CANCEL_REQ        (0x02)
#define RMTS_CONTROL_POINT_OPCODE_TOFD_TRANSFER_REQ (0x03)

#define RMTS_INFO_OPCODE_ERROR_IND        (0x00)
#define RMTS_INFO_OPCODE_TOFD_CAPTURED_IND (0x01)

#define RMTS_INFO_ERR                     (0x00)
#define RMTS_INFO_ERR_CANCELLED           (0x01)
#define RMTS_INFO_TOFD_CAPTURED_LENGTH     (5)

#define RMTS_TOFD_SIZE_OFFSET (0)
#define RMTS_TOFD_SIZE_LENGTH (4)
#define RMTS_TOFD_OFFSET      (4)

#define RMTS_MAX_PENDING_PACKET_COUNT  (5)

/** List of application specific BLE error codes that can be send in
 * ATT_ERROR_RSP PDUs.
 */
typedef enum RMTS_AttErr_t
{
	RMTS_ATT_ERR_NTF_DISABLED              = 0x80,
	RMTS_ATT_ERR_PROC_IN_PROGRESS          = 0x81,
	RMTS_ATT_ERR_TOFD_TRANSFER_DISALLOWED   = 0x82,
} RMTS_AttErr_t;

typedef enum RMTS_TransferState_t
{
	RMTS_STATE_IDLE,
	RMTS_STATE_CONNECTED,
	RMTS_STATE_START_REQUEST,
	RMTS_STATE_TOFD_INFO_PROVIDED,
	RMTS_STATE_TOFD_TRANSMISSION,
} RMTS_State_t;

/** Stores values required for Control Point Characteristic attributes. */
typedef struct RMTS_ControlPointCharacteristic_t
{
    /** Application Write callback. */
    RMTS_ControlHandler callback;

    uint8_t value[RMTS_CONTROL_POINT_VALUE_LENGTH];


} RMTS_ControlPointAttribute_t;

/** Stores values required for Info Characteristic attributes. */
typedef struct RMTS_InfoCharacteristic_t
{
    /** Client Characteristic Configuration Descriptor Value */
    uint8_t ccc[2];
} RMTS_InfoCharacteristic_t;

/** Stores values required for TOFD Info Characteristic attributes. */
typedef struct RMTS_TOFDCharacteristic_t
{
    /**
     * Number of bytes currently stored in #value that are queued for
     * transmission.
     */
    uint32_t value_length;

    uint8_t value[RMTS_TOFD_CHAR_VALUE_LENGTH];

    /** Client Characteristic Configuration Descriptor Value */
    uint8_t ccc[2];
} RMTS_TOFDCharacteristic_t;

/**
 * Collects all attribute database related variables of Picture Transfer
 * Service.
 */
typedef struct RMTS_AttDb_t
{
    /**
     * Offset of the attidx of RMTS service attributes.
     *
     * This can change depending on number and order of registeres custom
     * services to shared attribute database.
     */
    uint16_t attidx_offset;

    RMTS_ControlPointAttribute_t cp;
    RMTS_InfoCharacteristic_t info;
    RMTS_TOFDCharacteristic_t TOFD;
} RMTS_AttDb_t;

/**
 * Retains all information required to execute complete measured range data transfers to
 * connected peer device.
 */
typedef struct RMTS_TOFDTransferControl_t
{
    /** Current status of tofd transfer procedure. */
    RMTS_State_t state;

    /** Total size of the tofd that needs to be transfered. */
    uint32_t bytes_total;

    /** */
    uint32_t bytes_queued;

    /** */
    uint8_t packets_pending;
} RMTS_TOFDTransferControl_t;

typedef struct RMTS_Environment_t
{
    /**
     * Stores all attribute database related variables.
     */
    RMTS_AttDb_t att;

    /**
     * Stores progress information of ongoing tofd transfers.
     */
    RMTS_TOFDTransferControl_t transfer;

    /**
     * Maximum PDU size negotiated using Data Length Extension (DLE) for
     * current connection.
     *
     * Speeds up data transfers.
     * Available on on Bluetooth 4.2 and newer devices.
     * On 4.0 devices the value always remains at PTSS_MIN_TX_OCTETS .
     */
    uint16_t max_tx_octets;
} RMTS_Environment_t;

/* ----------------------------------------------------------------------------
 * Global variables and types
 * --------------------------------------------------------------------------*/


#ifdef __cplusplus
}
#endif    /* ifdef __cplusplus */

#endif    /* APP_BLE_RMTS_INT_H */
