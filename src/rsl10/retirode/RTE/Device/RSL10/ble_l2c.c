/* ----------------------------------------------------------------------------
 * Copyright (c) 2018 Semiconductor Components Industries, LLC (d/b/a
 * ON Semiconductor), All Rights Reserved
 *
 * Copyright (C) RivieraWaves 2009-2018
 *
 * This module is derived in part from example code provided by RivieraWaves
 * and as such the underlying code is the property of RivieraWaves [a member
 * of the CEVA, Inc. group of companies], together with additional code which
 * is the property of ON Semiconductor. The code (in whole or any part) may not
 * be redistributed in any form without prior written permission from
 * ON Semiconductor.
 *
 * The terms of use and warranty for this code are covered by contractual
 * agreements between ON Semiconductor and the licensee.
 *
 * This is Reusable Code.
 *
 * ----------------------------------------------------------------------------
 * ble_l2c.c
 * - BLE L2C layer abstraction source
 * ------------------------------------------------------------------------- */
#if RTE_BLE_L2CC_ENABLE

#include <ble_l2c.h>
#include <msg_handler.h>

/* L2C Environment Structure */
static L2C_Env_t l2c_env;

/* ----------------------------------------------------------------------------
 * Function      : const L2C_Env_t* L2C_GetEnv(void)
 * ----------------------------------------------------------------------------
 * Description   : Return a reference to the internal environment structure.
 * Inputs        : None
 * Outputs       : A constant pointer to L2C_Env_t
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
const L2C_Env_t* L2C_GetEnv(void)
{
    return &l2c_env;
}

/* ----------------------------------------------------------------------------
 * Function      : void L2CC_Initialize(void)
 * ----------------------------------------------------------------------------
 * Description   : Initialize the L2C environment
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void L2CC_Initialize(void)
{
    memset(&l2c_env, 0, sizeof(L2C_Env_t));
}

/* ----------------------------------------------------------------------------
 * Function      : void L2CC_LecbConnectCmd(uint8_t conidx,
 *                                   const struct l2cc_lecb_connect_cmd *param)
 * ----------------------------------------------------------------------------
 * Description   : LE credit based connection request
 * Inputs        : - conidx       - Connection index
 *                 - param        - Connection command parameters
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void L2CC_LecbConnectCmd(uint8_t conidx, const struct l2cc_lecb_connect_cmd *param)
{
    struct l2cc_lecb_connect_cmd *cmd;

    cmd = KE_MSG_ALLOC(L2CC_LECB_CONNECT_CMD,
                       KE_BUILD_ID(TASK_L2CC, conidx),
                       TASK_APP, l2cc_lecb_connect_cmd);

    memcpy(cmd, param, sizeof(struct l2cc_lecb_connect_cmd));

    ke_msg_send(cmd);
}

/* ----------------------------------------------------------------------------
 * Function      : void L2CC_LecbConnectCfm(uint8_t conidx,
 *                                   const struct l2cc_lecb_connect_cfm *param)
 * ----------------------------------------------------------------------------
 * Description   : Send connection confirmation
 * Inputs        : - conidx       - Connection index
 *                 - param        - Connection confirmation parameters
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void L2CC_LecbConnectCfm(uint8_t conidx, const struct l2cc_lecb_connect_cfm *param)
{
    struct l2cc_lecb_connect_cfm *cfm;

    cfm = KE_MSG_ALLOC(L2CC_LECB_CONNECT_CFM,
                       KE_BUILD_ID(TASK_L2CC, conidx),
                       TASK_APP, l2cc_lecb_connect_cfm);

    memcpy(cfm, param, sizeof(struct l2cc_lecb_connect_cfm));

    ke_msg_send(cfm);
}

/* ----------------------------------------------------------------------------
 * Function      : void L2CC_LecbAddCmd(uint8_t conidx, uint16_t local_cid,
 *                                      uint16_t credit)
 * ----------------------------------------------------------------------------
 * Description   : LE credit based credit addition
 * Inputs        : - conidx       - Connection index
 *                 - param        - Local Channel identifier
 *                 - credit       - Credit added locally for channel identifier
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void L2CC_LecbAddCmd(uint8_t conidx, uint16_t local_cid, uint16_t credit)
{
    struct l2cc_lecb_add_cmd *cmd;

    cmd = KE_MSG_ALLOC(L2CC_LECB_ADD_CMD,
                       KE_BUILD_ID(TASK_L2CC, conidx),
                       TASK_APP, l2cc_lecb_add_cmd);

    cmd->operation = L2CC_LECB_CREDIT_ADD;
    cmd->local_cid = local_cid;
    cmd->credit = credit;

    ke_msg_send(cmd);
}

/* ----------------------------------------------------------------------------
 * Function      : void L2CC_LecbSduSendCmd(uint8_t conidx, uint16_t cid,
 *                                        uint16_t length, const uint8_t* data)
 * ----------------------------------------------------------------------------
 * Description   : Send data over an LE Credit Based Connection
 * Inputs        : - conidx       - Connection index
 *                 - cid          - Channel identifier
 *                 - length       - Length of data (in bytes)
 *                 - data         - Data to be sent
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void L2CC_LecbSduSendCmd(uint8_t conidx, uint16_t cid, uint16_t length, const uint8_t* data)
{
    struct l2cc_lecb_sdu_send_cmd *cmd;

    cmd = KE_MSG_ALLOC_DYN(L2CC_LECB_SDU_SEND_CMD,
                       KE_BUILD_ID(TASK_L2CC, conidx),
                       TASK_APP, l2cc_lecb_sdu_send_cmd, length);

    cmd->operation = L2CC_LECB_SDU_SEND;
    cmd->sdu.cid = cid;
    cmd->sdu.length = length;
    memcpy(cmd->sdu.data, data, length);

    ke_msg_send(cmd);
}

/* ----------------------------------------------------------------------------
 * Function      : void L2CC_LecbDisconnectCmd(uint8_t conidx,
 *                                             uint16_t peer_cid)
 * ----------------------------------------------------------------------------
 * Description   : Disconnect an LE Credit Based Connection
 * Inputs        : - conidx       - Connection index
 *                 - peer_cid     - Peer Channel identifier
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void L2CC_LecbDisconnectCmd(uint8_t conidx, uint16_t peer_cid)
{
    struct l2cc_lecb_disconnect_cmd *cmd;

    cmd = KE_MSG_ALLOC(L2CC_LECB_DISCONNECT_CMD,
                       KE_BUILD_ID(TASK_L2CC, conidx),
                       TASK_APP, l2cc_lecb_disconnect_cmd);

    cmd->operation = L2CC_LECB_DISCONNECT;
    cmd->peer_cid = peer_cid;
    ke_msg_send(cmd);
}

void L2CC_MsgHandler(ke_msg_id_t const msg_id, void const *param,
                     ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    switch(msg_id)
    {
        case L2CC_LECB_DISCONNECT_IND:
        {
            const struct l2cc_lecb_disconnect_ind* ind = param;

            /* Search for connection info in l2c_env.connectionInfo[] and
             * remove disconnected element (shift array left with memmove) */
            for(uint16_t i = 0; i < l2c_env.connectionCount; i++)
            {
                if((ind->le_psm == l2c_env.connectionInfo[i].le_psm) &&
                   (ind->local_cid == l2c_env.connectionInfo[i].local_cid) &&
                   (ind->peer_cid == l2c_env.connectionInfo[i].peer_cid) &&
                   (i < (l2c_env.connectionCount - 1)))
                {
                    uint16_t shift = l2c_env.connectionCount - i - 1;
                    memmove(&l2c_env.connectionInfo[i], &l2c_env.connectionInfo[i + 1],
                            shift * (sizeof(L2C_ConnectionInfo_t)));
                    break;
                }
            }
            l2c_env.connectionCount--;
        }
        break;

        case L2CC_LECB_CONNECT_REQ_IND:
        {
            const struct l2cc_lecb_connect_req_ind* ind = param;
            l2c_env.connectionInfo[l2c_env.connectionCount].le_psm = ind->le_psm;
            l2c_env.connectionInfo[l2c_env.connectionCount].peer_cid = ind->peer_cid;
            l2c_env.connectionCount++;
        }
        break;

        case L2CC_LECB_CONNECT_IND:
        {
            const struct l2cc_lecb_connect_ind* ind = param;
            for (uint16_t i = 0; i < l2c_env.connectionCount; i++)
            {
                if((ind->le_psm == l2c_env.connectionInfo[i].le_psm) &&
                   (ind->peer_cid == l2c_env.connectionInfo[i].peer_cid))
                {
                    l2c_env.connectionInfo[i].local_cid = ind->local_cid;
                    break;
                }
            }

        }
        break;

        case L2CC_CMP_EVT:
        {

        }
        break;
    }
}

#endif /* RTE_BLE_L2CC_ENABLE */
