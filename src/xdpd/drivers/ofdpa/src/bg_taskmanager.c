#include "bg_taskmanager.h"

#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <rofl/common/utils/c_logger.h>
#include "config.h"

//Local static variable for background manager thread
static pthread_t bg_thread;
static bool bg_continue_execution = true;

void* ofdpa_bg_loop(void* param){
	ROFL_DEBUG(DRIVER_NAME"[bg] Starting monitoring routine...\n");

	//TODO: FIXME
	return NULL;
}

rofl_result_t launch_bgmanager(){
	//Set flag
	bg_continue_execution = true;

	if(pthread_create(&bg_thread, NULL, ofdpa_bg_loop,NULL)<0){
		ROFL_ERR(DRIVER_NAME"[bg] pthread_create failed, errno(%d): %s\n", errno, strerror(errno));
		return ROFL_FAILURE;
	}

	return ROFL_SUCCESS;
}

rofl_result_t stop_bgmanager(){
	bg_continue_execution = false;
	pthread_join(bg_thread,NULL);
	return ROFL_SUCCESS;
}


