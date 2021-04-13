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
 * ble_l2c.h
 * - BLE L2C layer abstraction header
 * ------------------------------------------------------------------------- */
#ifndef BLE_L2C_H
#define BLE_L2C_H

#if RTE_BLE_L2CC_ENABLE
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
#include <rsl10_ke.h>
#include <l2cc.h>

/* ----------------------------------------------------------------------------
 * Defines
 * ------------------------------------------------------------------------- */
#ifndef L2C_CONNECTION_MAX
#define L2C_CONNECTION_MAX             1
#endif

typedef struct
{
    uint8_t conidx;
    uint16_t local_cid;
    uint16_t peer_cid;
    uint16_t le_psm;
} L2C_ConnectionInfo_t;

typedef struct
{
    uint16_t connectionCount;
    L2C_ConnectionInfo_t connectionInfo[L2C_CONNECTION_MAX];
} L2C_Env_t;

/* ----------------------------------------------------------------------------
 * L2C Functions
 * --------------------------------------------------------------------------*/
const L2C_Env_t* L2C_GetEnv(void);

void L2CC_Initialize(void);

void L2CC_LecbConnectCmd(uint8_t conidx, const struct l2cc_lecb_connect_cmd *param);

void L2CC_LecbConnectCfm(uint8_t conidx, const struct l2cc_lecb_connect_cfm *param);

void L2CC_LecbDisconnectCmd(uint8_t conidx, uint16_t peer_cid);

void L2CC_LecbAddCmd(uint8_t conidx, uint16_t local_cid, uint16_t credit);

void L2CC_LecbSduSendCmd(uint8_t conidx, uint16_t cid, uint16_t length, const uint8_t* data);

void L2CC_MsgHandler(ke_msg_id_t const msg_id, void const *param,
                     ke_task_id_t const dest_id, ke_task_id_t const src_id);

/* ----------------------------------------------------------------------------
 * Close the 'extern "C"' block
 * ------------------------------------------------------------------------- */
#ifdef __cplusplus
}
#endif    /* ifdef __cplusplus */

#endif /* RTE_BLE_L2CC_ENABLE */

#endif    /* BLE_L2C_H */
