/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef XDPD_OFDPA_CONFIG_H
#define XDPD_OFDPA_CONFIG_H

/**
* @file config.h
*
* @author Marc Sune<marc.sune (at) bisdn.de>
* @brief OFDPA configuration
*/

//OFDPA includes
#include <ofdpa_api.h>

//Driver name
#define DRIVER_NAME "[xdpd][ofdpa]"

//Number of tables needed by OFDPA (max table)
#define OFDPA_NUM_OF_TABLES OFDPA_FLOW_TABLE_ID_ACL_POLICY

//Use to efficently loop over the OFDPA table set
#define OFDPA_MAX_TABLE_LOOP 7

//Timeout for the BG task manager
#define BG_OFDPA_EVENT_TIMEOUT_MS 250

//Array
extern OFDPA_FLOW_TABLE_ID_t ofdpa_table_set[OFDPA_MAX_TABLE_LOOP];

#endif //XDPD_OFDPA_CONFIG_H
