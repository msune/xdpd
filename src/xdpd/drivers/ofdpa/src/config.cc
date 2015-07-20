#include "config.h"
#include "util/compiler_assert.h"

/*
* Validation of config values (at compile time)
*/


/*
* Helper array to efficently loop over OFDPA tables
*/
OFDPA_FLOW_TABLE_ID_t ofdpa_table_set[OFDPA_MAX_TABLE_LOOP] = {
	OFDPA_FLOW_TABLE_ID_INGRESS_PORT,
	OFDPA_FLOW_TABLE_ID_VLAN,
	OFDPA_FLOW_TABLE_ID_TERMINATION_MAC,
	OFDPA_FLOW_TABLE_ID_UNICAST_ROUTING,
	OFDPA_FLOW_TABLE_ID_MULTICAST_ROUTING,
	OFDPA_FLOW_TABLE_ID_BRIDGING,
	OFDPA_FLOW_TABLE_ID_ACL_POLICY
};
