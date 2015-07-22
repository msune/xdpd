#include <assert.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/of1x_async_events_hooks.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/of1x_switch.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/pipeline/of1x_flow_table.h>
#include <rofl/datapath/hal/openflow/openflow1x/of1x_cmm.h>
#include <rofl/common/utils/c_logger.h>

//Own includes
#include "../config.h"

//OFDPA includes
#include <ofdpa_api.h>

/*
* Hooks for configuration of the switch
*/
rofl_result_t platform_post_init_of1x_switch(of1x_switch_t* sw){

	unsigned int i;

	for(i=0; i<sw->pipeline.num_of_tables; i++){
		of1x_flow_table_config_t *config = &(sw->pipeline.tables[i].config);
		of1x_group_table_config_t *group_config = &(sw->pipeline.groups->config);

		//Clear everything
		bitmap128_clean(&config->match);
		bitmap128_clean(&config->wildcards);
		bitmap128_clean(&config->apply_actions);
		bitmap128_clean(&config->write_actions);
		config->metadata_match = 0x0ULL;
		config->metadata_write = 0x0ULL;
		config->instructions = 0x0;
		config->table_miss_config = 0x0;
		bitmap128_clean(&group_config->supported_actions);

		switch(i){
			case OFDPA_FLOW_TABLE_ID_INGRESS_PORT:
				//Matches
				bitmap128_set(&config->match, OF1X_MATCH_IN_PORT);
				bitmap128_set(&config->match, OF1X_MATCH_ETH_TYPE);
				bitmap128_set(&config->match, OF1X_MATCH_TUNNEL_ID);
				//TODO extensions

				//Apply actions
				//TODO extensions

				//Instructions
				config->instructions = 	(1 << OF1X_IT_APPLY_ACTIONS) |
							(1 << OF1X_IT_GOTO_TABLE);

				break;
			case OFDPA_FLOW_TABLE_ID_VLAN:
				//Matches
				bitmap128_set(&config->match, OF1X_MATCH_IN_PORT);
				bitmap128_set(&config->match, OF1X_MATCH_ETH_DST);
				bitmap128_set(&config->match, OF1X_MATCH_VLAN_VID);
				bitmap128_set(&config->match, OF1X_MATCH_ETH_TYPE);
				bitmap128_set(&config->match, OF1X_MATCH_TUNNEL_ID);
				//TODO extensions

				//Apply actions
				bitmap128_set(&config->apply_actions, OF1X_AT_SET_FIELD_VLAN_VID);
				bitmap128_set(&config->apply_actions, OF1X_AT_PUSH_VLAN);
				bitmap128_set(&config->apply_actions, OF1X_AT_COPY_TTL_IN);
				bitmap128_set(&config->apply_actions, OF1X_AT_SET_FIELD_TUNNEL_ID);
				//TODO extensions

				//Write actions
				bitmap128_set(&config->write_actions, OF1X_AT_OUTPUT);
				bitmap128_set(&config->write_actions, OF1X_AT_GROUP);

				//Instructions
				config->instructions = 	(1 << OF1X_IT_APPLY_ACTIONS) |
							(1 << OF1X_IT_WRITE_ACTIONS) |
							(1 << OF1X_IT_GOTO_TABLE);


				break;
			case OFDPA_FLOW_TABLE_ID_TERMINATION_MAC:
				break;
			case OFDPA_FLOW_TABLE_ID_UNICAST_ROUTING:
				break;
			case OFDPA_FLOW_TABLE_ID_MULTICAST_ROUTING:
				break;
			case OFDPA_FLOW_TABLE_ID_BRIDGING:
				break;
			case OFDPA_FLOW_TABLE_ID_ACL_POLICY:
				break;
			default:
				//Do nothing
				break;
		}


	}

	
	ROFL_INFO(DRIVER_NAME " calling %s()\n",__FUNCTION__);

	return ROFL_SUCCESS;
}

rofl_result_t platform_pre_destroy_of1x_switch(of1x_switch_t* sw){

	ROFL_INFO(DRIVER_NAME " calling %s()\n",__FUNCTION__);

	return ROFL_SUCCESS;
}


//Async notifications

/*
* Packet in
*/
void platform_of1x_packet_in(const of1x_switch_t* sw, uint8_t table_id, datapacket_t* pkt, uint16_t send_len, of_packet_in_reason_t reason)
{
	ROFL_INFO(DRIVER_NAME " calling %s()\n",__FUNCTION__);
}

//Flow removed
void platform_of1x_notify_flow_removed(const of1x_switch_t* sw,
						of1x_flow_remove_reason_t reason,
						of1x_flow_entry_t* removed_flow_entry){

	ROFL_INFO(DRIVER_NAME " calling %s()\n",__FUNCTION__);

}


void plaftorm_of1x_add_entry_hook(of1x_flow_entry_t* new_entry){

}

void platform_of1x_modify_entry_hook(of1x_flow_entry_t* old_entry, of1x_flow_entry_t* mod, int reset_count){

}

void platform_of1x_remove_entry_hook(of1x_flow_entry_t* entry){

}

void platform_of1x_update_stats_hook(of1x_flow_entry_t* entry) {

}
