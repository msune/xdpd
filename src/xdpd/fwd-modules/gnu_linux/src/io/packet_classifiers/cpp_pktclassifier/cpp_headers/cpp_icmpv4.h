/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _CPP_ICMPV4_H_
#define _CPP_ICMPV4_H_

#include <stdint.h>

uint8_t get_icmp_type(void *hdr);

void set_icmp_type(void *hdr, uint8_t type);

uint8_t get_icmp_code(void *hdr);

void set_icmp_code(void *hdr, uint8_t code);

uint16_t get_checksum(void *hdr);

void icmpv4_calc_checksum(void * hdr,uint16_t length);

#endif //_CPP_ICMPV4_H_
