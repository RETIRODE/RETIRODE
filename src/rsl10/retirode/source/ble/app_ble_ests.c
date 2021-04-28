//-----------------------------------------------------------------------------
// Copyright (c) 2019 Semiconductor Components Industries LLC
// (d/b/a "ON Semiconductor").  All rights reserved.
// This software and/or documentation is licensed by ON Semiconductor under
// limited terms and conditions.  The terms and conditions pertaining to the
// software and/or documentation are available at
// http://www.onsemi.com/site/pdf/ONSEMI_T&C.pdf ("ON Semiconductor Standard
// Terms and Conditions of Sale, Section 8 Software") and if applicable the
// software license agreement.  Do not use this software and/or documentation
// unless you have carefully read and you agree to the limited terms and
// conditions.  By using this software and/or documentation, you agree to the
// limited terms and conditions.
//-----------------------------------------------------------------------------

#include <app_ble_ESTS_int.h>
#include <app_ble_peripheral_server.h>
#include <msg_handler.h>


static uint8_t ESTS_Send_Command_Handler(uint8_t conidx, uint16_t atsidx,
        uint16_t handle, uint8_t *to, const uint8_t *from, uint16_t length,
        uint16_t operation);

static uint8_t ESTS_Send_Query_Handler(uint8_t conidx, uint16_t atsidx,
        uint16_t handle, uint8_t *to, const uint8_t *from, uint16_t length,
        uint16_t operation);


static ESTS_MSG_ID_COUNT = 0;

static ESTS_Environment_t ESTS_env = { 0 };

const struct att_db_desc ESTS_att_db[ATT_ESTS_COUNT] =
{
    /* External Sensor Transfer Service 0 */

    CS_SERVICE_UUID_128(
            ATT_ESTS_SERVICE_0, /* atsidx */
            ESTS_SVC_UUID),     /* uuid */


    /* External Sensor Trigger Service - Range Finder Send Command */

    CS_CHAR_UUID_128(
    		ATT_ESTS_RANGE_FINDER_SEND_COMMAND_CHAR_0,              /* atsidx_char */
			ATT_ESTS_RANGE_FINDER_SEND_COMMAND_VAL_0,               /* atsidx_val */
			ESTS_RFSC_UUID,                          				/* uuid */
            PERM(WRITE_REQ, ENABLE) | PERM(WRITE_COMMAND, ENABLE),  /* perm */
            sizeof(ESTS_env.att.send_command.value),                /* length */
            ESTS_env.att.send_command.value,                        /* data */
			ESTS_Send_Command_Handler),                         /* callback */

    CS_CHAR_USER_DESC(
    		ATT_ESTS_RANGE_FINDER_SEND_COMMAND_DESC_0,              /* atsidx */
            (sizeof(ETSS_RFSC_USER_DESC) - 1), /* length */
			ETSS_RFSC_USER_DESC,               /* data */
            NULL),                                      /* callback */


		/* External Sensor Trigger Service - Range Finder Send Command */

	CS_CHAR_UUID_128(
			ATT_ESTS_RANGE_FINDER_SEND_QUERY_CHAR_0,              /* atsidx_char */
			ATT_ESTS_RANGE_FINDER_SEND_QUERY_VAL_0,               /* atsidx_val */
			ESTS_RFSQ_UUID,                          				/* uuid */
			PERM(WRITE_REQ, ENABLE) | PERM(WRITE_COMMAND, ENABLE),  /* perm */
			sizeof(ESTS_env.att.send_query.value),                /* length */
			ESTS_env.att.send_query.value,                        /* data */
			ESTS_Send_Query_Handler),                         /* callback */

	CS_CHAR_USER_DESC(
			ATT_ESTS_RANGE_FINDER_SEND_QUERY_DESC_0,              /* atsidx */
			(sizeof(ETSS_RFSQ_USER_DESC) - 1), /* length */
			ETSS_RFSQ_USER_DESC,               /* data */
			NULL),                                      /* callback */


    /* External Sensor Trigger Service - Range Finder Receive Query */

    CS_CHAR_UUID_128(
    		ATT_ESTS_RANGE_FINDER_RECEIVE_QUERY_CHAR_0, /* atsidx_char */
			ATT_ESTS_RANGE_FINDER_RECEIVE_QUERY_VAL_0,           /* atsidx_val */
			ESTS_RFRQ_UUID,            /* uuid */
            PERM(NTF, ENABLE),                   /* perm */
            0,                                   /* length */
            NULL,                                /* data */
            NULL),                               /* callback */

    CS_CHAR_CCC(
    		ATT_ESTS_RANGE_FINDER_RECEIVE_QUERY_CCC_0,       /* atsidx */
            ESTS_env.att.receive_query.ccc,       /* data */
            NULL),                           /* callback */

    CS_CHAR_USER_DESC(
    		ATT_ESTS_RANGE_FINDER_RECEIVE_QUERY_DESC_0,            /* atsidx */
            (sizeof(ETSS_RFRQ_USER_DESC) - 1), /* length*/
			ETSS_RFRQ_USER_DESC,               /* data */
            NULL),                                 /* callback */
};

static void ESTS_AppMsgHandler(ke_msg_id_t const msg_id, void const *param,
        ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    if ((msg_id >= ESTS_env.msg_id_offset)
        && (msg_id < (ESTS_env.msg_id_offset + ESTS_MSG_ID_COUNT)))
    {
        uint8_t sidx = msg_id - ESTS_env.msg_id_offset;

        /* It is possible the client disconnected since the message was
         * scheduled.
         */
        if (ESTS_env.state == ESTS_STATE_CONNECTED)
        {
            /* It is possible that the client disabled this trigger in the
             * meantime.
             */

        }
    }
}

static void ESTS_BleMsgHandler(ke_msg_id_t const msg_id, void const *param,
        ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    switch (msg_id)
    {
        case GAPC_CONNECTION_REQ_IND:
        {


            ESTS_env.state = ESTS_STATE_CONNECTED;

            /* Make sure all connection related variables are reset.*/
            for (ESTS_RF_SETTING_ID_t tidx = 0; tidx < ESTS_RF_SETTING_ID_COUNT; ++tidx)
            {
                //ESTS_env.att.receive_query[tidx].ccc[0] = 0;
               // ESTS_env.att.receive_query[tidx].ccc[1] = 0;
            }


            break;
        }

        case GAPC_DISCONNECT_IND:
        {


            ESTS_env.state = ESTS_STATE_IDLE;

            /* Disable any active triggers upon disconnection. */
            for (ESTS_RF_SETTING_ID_t tidx = 0; tidx < ESTS_RF_SETTING_ID_COUNT; ++tidx)
            {
                //ESTS_ConfigureTriggerSource(tidx, false);
            }


            break;
        }

        default:
            break;
    }
}

int32_t ESTS_Initialize(ESTS_ControlHandler control_event_handler)
{
    int32_t status;

    if (control_event_handler == NULL)
    {
        return ESTS_ERR;
    }

    ESTS_env.att.attidx_offset = 0;
    ESTS_env.att.send_command.callback = control_event_handler;
    ESTS_env.att.send_query.callback = control_event_handler;

    ESTS_env.att.receive_query.ccc[0] = 0x00;
    ESTS_env.att.receive_query.ccc[1] = 0x00;

    ESTS_env.state = ESTS_STATE_IDLE;


    /* Add custom attributes into the attribute database. */
    status = APP_BLE_PeripheralServerAddCustomService(ESTS_att_db, ATT_ESTS_COUNT,
            &ESTS_env.att.attidx_offset);
    if (status != 0)
    {
        return ESTS_ERR_INSUFFICIENT_ATT_DB_SIZE;
    }

    ESTS_env.msg_id_offset = APP_BLE_PeripheralServerRegisterKernelMsgIds(
               ESTS_MSG_ID_COUNT);

       ESTS_env.state = ESTS_STATE_IDLE;

       /* Listen for specific BLE kernel messages. */
       MsgHandler_Add(TASK_ID_APP, ESTS_AppMsgHandler);
       MsgHandler_Add(GAPC_DISCONNECT_IND, ESTS_BleMsgHandler);
       MsgHandler_Add(GAPC_CONNECTION_REQ_IND, ESTS_BleMsgHandler);

    return ESTS_OK;
}

static uint8_t ESTS_Send_Command_Handler(uint8_t conidx, uint16_t atsidx,
        uint16_t handle, uint8_t *to, const uint8_t *from, uint16_t length,
        uint16_t operation)
{
	return 0;
}


static uint8_t ESTS_Send_Query_Handler(uint8_t conidx, uint16_t atsidx,
        uint16_t handle, uint8_t *to, const uint8_t *from, uint16_t length,
        uint16_t operation)
{
	return 0;
}
