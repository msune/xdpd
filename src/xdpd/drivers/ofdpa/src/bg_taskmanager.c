#include "bg_taskmanager.h"

#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <rofl/common/utils/c_logger.h>
#include "config.h"

//OFDPA includes
#include <ofdpa_api.h>

//Local static variable for background manager thread
static pthread_t bg_thread;
static bool cont_exec = true;

void* ofdpa_bg_loop(void* param){

	unsigned int i;
	struct timeval timeout;
	ofdpaFlowEvent_t flow_event;
	ofdpaPortEvent_t port_event;

	ROFL_DEBUG(DRIVER_NAME"[bg] Starting monitoring routine...\n");

	//Prepare timeout struct
	timeout.tv_sec = BG_OFDPA_EVENT_TIMEOUT_MS/1000;
	timeout.tv_usec = (BG_OFDPA_EVENT_TIMEOUT_MS%1000)*1000;

	while(cont_exec){
		if(ofdpaEventReceive(&timeout) != OFDPA_E_NONE)
			continue;

		//Check flow events (for all tables)
		for(i=0; i<OFDPA_MAX_TABLE_LOOP; i++){
			memset(&flow_event, 0, sizeof(flow_event));
			flow_event.flowMatch.tableId = ofdpa_table_set[i];
			while(ofdpaFlowEventNextGet(&flow_event) == OFDPA_E_NONE){
				//TODO: notify this change upwards to the CMM
			}
		}

		//Check port events
		memset(&port_event, 0, sizeof(port_event));
		while(ofdpaPortEventNextGet(&port_event) == OFDPA_E_NONE){
			//TODO: notify this change upwards to the CMM
		}
	}

	return NULL;
}

rofl_result_t launch_bgmanager(){
	//Set flag
	cont_exec = true;

	if(pthread_create(&bg_thread, NULL, ofdpa_bg_loop,NULL)<0){
		ROFL_ERR(DRIVER_NAME"[bg] pthread_create failed, errno(%d): %s\n", errno, strerror(errno));
		return ROFL_FAILURE;
	}

	return ROFL_SUCCESS;
}

rofl_result_t stop_bgmanager(){
	cont_exec = false;
	pthread_join(bg_thread,NULL);
	return ROFL_SUCCESS;
}


