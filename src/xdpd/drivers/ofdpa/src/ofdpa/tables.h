/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef OFDPA_TABLES
#define OFDPA_TABLES

#include <rofl_datapath.h>
#include <stdbool.h>

/**
* @file ofdpa.h
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief OFDPA table utils
*
*/

//C++ extern C
ROFL_BEGIN_DECLS

/**
* Checks if the table id is a valid one
*/
bool ofdpa_is_valid_table_id(const unsigned int tid);

//C++ extern C
ROFL_END_DECLS

#endif /* OFDPA_TABLES */
