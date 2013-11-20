/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _CPC_ETERNET_H_
#define _CPC_ETERNET_H_

#include "../cpc_utils.h"

#define DEFAULT_ETHER_FRAME_SIZE 1518
#define CPC_ETH_ALEN 6

static inline
uint64_t mac_addr_to_u64(uint8_t *mac){
	uint32_t *p32 = (uint32_t*)mac;
	uint16_t *p16 = (uint16_t*)(&mac[4]);
	return (uint64_t)*p32 + (((uint64_t)*p16)<<32);
};

static inline
void u64_to_mac_ptr(uint8_t *mac, uint64_t val){
	uint32_t *p32 = (uint32_t*) mac;
	uint16_t *p16 = (uint16_t*) (&mac[4]);
	
	*p32 = (uint32_t)(val&0x00000000ffffffff);
	*p16 = (uint16_t)((val&0x0000ffff00000000)>>32);
};

/* Ethernet constants and definitions */

// Ethernet II header
struct cpc_eth_hdr {
	uint8_t dl_dst[CPC_ETH_ALEN];
	uint8_t dl_src[CPC_ETH_ALEN];
	uint16_t dl_type;
	uint8_t data[0];
}__attribute__((packed));

typedef struct cpc_eth_hdr cpc_eth_hdr_t;

inline static
uint64_t get_dl_eth_dst(void *hdr){
	return mac_addr_to_u64(((cpc_eth_hdr_t*)hdr)->dl_dst);
};

inline static
void set_dl_eth_dst(void* hdr, uint64_t dl_dst){
	u64_to_mac_ptr( ((cpc_eth_hdr_t*)hdr)->dl_dst, dl_dst);
	//TODO is the mac also swapped to host byte order?
};

inline static
uint64_t get_dl_eth_src(void* hdr){
	return mac_addr_to_u64(((cpc_eth_hdr_t*)hdr)->dl_src);
};

inline static
void set_dl_eth_src(void* hdr, uint64_t dl_src){
	u64_to_mac_ptr( ((cpc_eth_hdr_t*)hdr)->dl_src, dl_src);
	//TODO is the mac also swapped to host byte order?
};

inline static
uint16_t get_dl_eth_type(void* hdr){
	return CPC_BE16TOH(((cpc_eth_hdr_t *)hdr)->dl_type);
};

inline static
void set_dl_eth_type(void* hdr, uint16_t dl_type){
	((cpc_eth_hdr_t *)hdr)->dl_type = CPC_HTOBE16(dl_type);
};

#endif //_CPC_ETERNET_H_
