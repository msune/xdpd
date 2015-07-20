/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef OFDPA_PORTS
#define OFDPA_PORTS

#include <rofl_datapath.h>
#include <rofl/datapath/pipeline/switch_port.h>

/**
* @file ports.h
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief OFDPA port management routines
*
*/
/**
* Discover physical ports in the Broadcom switch
*/
rofl_result_t discover_ports(void);

/**
* Bring Broadcom switch port up
*/
rofl_result_t bring_port_up(switch_port_t* port);

/**
* Bring Broadcom switch port down
*/
rofl_result_t bring_port_down(switch_port_t* port);

#endif /* OFDPA_PORTS */
