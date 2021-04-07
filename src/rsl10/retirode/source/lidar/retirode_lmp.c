/*
 * retirode_lmp.c
 *
 *  Created on: Apr 4, 2021
 *      Author: marti
 */


#include <retirode_lmp.h>
#include <rsl10.h>

#define QUERY_CHAR                 		"??"
#define ACK_CHAR                   		"\r"
#define COMMAND(reg, value)				BuildCommand(reg,value, 4)
#define QUERY(reg)						BuildCommand(reg,QUERY_CHAR, 4)


static void BuildCommand(const char reg, const char value, char *command, uint32_t len);
/**
 * Array of internal flags about the state of library and which application
 * issued commands are pending.
 */
struct RETIRODE_LMP_EnvironmentFlags_t
{
    /** Command to start power up of the LMP was received. */
    bool cmd_power_on;

    /** Command to power off LMP was received. */
    bool cmd_power_off;

    /** Command to measure n samples*/
    uint32_t cmd_measure;

    /** Flag to indicate that LMP is not in shutdown mode. */
    bool is_powered;

    /** Flag to indicate that LMP is ready to start next UART transaction. */
    bool is_uart_ready;
};

/**
 * Holds all information about current state of LMP
 */
struct RETIRODE_LMP_Eviroment_t
{
	 /** Pointer to UART CMSIS-Driver used to communicate with the LIDAR. */
	ARM_DRIVER_USART *uart;

	/**
	 * Flag set from UART Event Handler used to determine status of UART
	 * transaction.
	 *
	 * Some error flags are exposed only using the shared Event Handler,
	 * therefore we store the value given in UART interrupt and process it in
	 * then read / write routines.
	 */
	volatile uint32_t uart_event;

	/**
	 * Current state of LMP operation.
	 *
	 * Used to call appropriate state handler in main loop.
	 */
	RETIRODE_LMP_State_t state;

	/** Flags about LMP state and application issued commands. */
	volatile struct RETIRODE_LMP_EnvironmentFlags_t flag;

	/**
	 * Application assigned event handler used to notify application on
	 * certain events.
	 */
	RETIRODE_LMP_EventHandler_t p_evt_handler;
};

/** @internal */
typedef void (*RETIRODE_LMP_StateHandler_t)(void);


static void RETIRODE_LMP_ShutdownStateHandler(void);
static void RETIRODE_LMP_PowerUpStateHandler(void);
static void RETIRODE_LMP_IdleStateHandler(void);
static void RETIRODE_LMP_ConfigurationStateHandler(void);
static void RETIRODE_LMP_MeasuringStateHandler(void);
static void RETIRODE_LMP_DataReadyStateHandler(void);


static struct RETIRODE_LMP_Eviroment_t lmp_env;

static const RETIRODE_LMP_StateHandler_t retirode_lmp_state_handler[RETIRODE_LMP_STATE_MAX] =
{
	[RETIRODE_LMP_STATE_SHUTDOWN] = RETIRODE_LMP_ShutdownStateHandler,
	[RETIRODE_LMP_STATE_POWER_UP] = RETIRODE_LMP_PowerUpStateHandler,
	[RETIRODE_LMP_STATE_IDLE] = RETIRODE_LMP_IdleStateHandler,
	[RETIRODE_LMP_STATE_CONFIGURATION] = RETIRODE_LMP_ConfigurationStateHandler,
	[RETIRODE_LMP_STATE_MEASURING] = RETIRODE_LMP_MeasuringStateHandler,
	[RETIRODE_LMP_STATE_DATA_READY] = RETIRODE_LMP_DataReadyStateHandler
};


/**
 * Function to build command/queries which will be send to LIDAR.
 *
 */



/**
 * Wrapper function for setting of new ISP state.
 *
 * Allows to add state change debug messages here if necessary.
 */
static void RETIRODE_LMP_SetState(const RETIRODE_LMP_State_t new_state)
{
    lmp_env.state = new_state;
}

/**
 * Issue a UART write command to LIDAR.
 *
 * These commands are used to issue commands to the LIDAR to either start a
 * specific operation or to query data.
 *
 * @param cmd
 * Command as defined in @ref SMARTSHOT_ISP_COMMANDS_GRP.
 *
 * @return
 * ARM_DRIVER_OK - On success. <br>
 * ARM_DRIVER_ERROR - On failure.
 */
static int32_t RETIRODE_LMP_WriteCommand(uint8_t cmd)
{

    return 1;
}


static void RETIRODE_LMP_ShutdownStateHandler(void)
{
	if(lmp_env.flag.cmd_power_on)
	{
		//TODO: Check if lmp is initialized
		lmp_env.flag.is_powered = true;

		RETIRODE_LMP_SetState(RETIRODE_LMP_STATE_POWER_UP);
	}
}


static void RETIRODE_LMP_PowerUpStateHandler(void)
{
	if(lmp_env.flag.cmd_power_on)
	{
		//TODO: Check if lmp is initialized
		lmp_env.flag.is_powered = true;

		RETIRODE_LMP_SetState(RETIRODE_LMP_STATE_POWER_UP);
	}
}
