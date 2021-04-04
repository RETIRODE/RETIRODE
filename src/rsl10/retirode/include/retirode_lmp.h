/*
 * retirode_lmp.h
 *
 *  Created on: Apr 4, 2021
 *      Author: marti
 */

#ifndef INCLUDE_RETIRODE_LMP_H_
#define INCLUDE_RETIRODE_LMP_H_

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


/* ----------------------------------------------------------------------------
 * Define & Type declaration
 * ------------------------------------------------------------------------- */
#define R_REGISTER                 "R"
#define C_REGISTER                 "C"
#define W_REGISTER                 "W"
#define P_REGISTER                 "P"
#define p_REGISTER                 "p"
#define N_REGISTER                 "N"
#define D_REGISTER                 "D"
#define L_REGISTER                 "L"
#define B_REGISTER                 "B"
#define I_REGISTER                 "I"
#define b_REGISTER                 "b"


typedef enum RETIRODE_LMP_State_t
{
    /**
     * LIDAR is completely powered off.
     * State transitions:
     * * SMARTSHOT_ISP_STATE_POWER_UP - After receiving power up command.
     */
	RETIRODE_LMP_STATE_SHUTDOWN,

    /**
     * LIDAR is powering up.
     * Turning ON both power supplies
     * State transitions:
     * * SMARTSHOT_ISP_STATE_IDLE - After successful power-up operation.
     */
	RETIRODE_LMP_STATE_POWER_UP,

	/**
	 * LIDAR is ready and waiting for application commands.
	 *
	 * State transitions:
	 * * SMARTSHOT_ISP_STATE_CONFIGURATION - After receiving CONFIGURE command.
	 * * SMARTSHOT_ISP_STATE_MEASURING - After receiving MEASURE command.
	 */
	RETIRODE_LMP_STATE_IDLE,

	/**
	 * Send received configuration command to LIDAR
	 *
	 * State transitions:
	 * * SMARTSHOT_ISP_STATE_IDLE - After processing configuration command.
	 */
	RETIRODE_LMP_STATE_CONFIGURATION,

	/**
	 * Send command to LIDAR to start measurement
	 *
	 * State transitions:
	 * * SMARTSHOT_ISP_STATE_DATA_READY - After RECEIVING data from LIDAR.
	 */
	RETIRODE_LMP_STATE_MEASURING,

	/**
	 * Notifies application about received data
	 *
	 * State transitions:
	 * * SMARTSHOT_ISP_STATE_DATA_READY - After RECEIVING data from LIDAR.
	 */
	RETIRODE_LMP_STATE_DATA_READY,

	RETIRODE_LMP_STATE_MAX
} RETIRODE_LMP_State_t;


/**
 * List of event generated by RETIRODE_LMP library that need to be processed
 * by the application
 */
typedef enum RETIRODE_LMP_Event_t
{
	RETIRODE_LMP_EVENT_ERROR,
	RETIRODE_LMP_EVENT_READY,
	RETIRODE_LMP_EVENT_MEASUREMENT_DATA_READY,
	RETIRODE_LMP_EVENT_QUERY_RESPONSE_READY
}RETIRODE_LMP_Event_t;

/** Function prototype for RETIRODE_LMP library event handler.
 *
 * @param event
 * Event type to be processed.
 *
 * @param p_param
 * Pointer to optional event specific parameter structure.
 */
typedef void (*RETIRODE_LMP_EventHandler_t)(RETIRODE_LMP_Event_t event,
        const void *p_param);

/* ----------------------------------------------------------------------------
 * Close the 'extern "C"' block
 * ------------------------------------------------------------------------- */
#ifdef __cplusplus
}
#endif    /* ifdef __cplusplus */

#endif /* INCLUDE_RETIRODE_LMP_H_ */