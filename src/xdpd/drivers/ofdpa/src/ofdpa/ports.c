#include "ports.h"

#include <assert.h>
#include <pthread.h>
#include <rofl/common/utils/c_logger.h>
#include <rofl/datapath/pipeline/physical_switch.h>

//OFDPA includes
#include <ofdpa_api.h>

//Own includes
#include "../config.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/**
* Port additional OFDPA state
*/
typedef struct ofdpa_port_state{
	//OFDPA port id
	uint32_t id;
}ofdpa_port_state_t;

//Fill in the port capabilities
static void port_update_state(switch_port_t* port,  ofdpa_port_state_t* ps){

	OFDPA_ERROR_t err;
	ofdpaPortFeature_t features;
	OFDPA_PORT_CONFIG_t admin_state;

	//First set administrative state
	err = ofdpaPortConfigGet(ps->id, &admin_state);
	if(err != OFDPA_E_NONE){
		ROFL_WARN(DRIVER_NAME "[ports] WARNING: unable to recover admin state for port '%s' via ofdpaPortConfigGet() call. Skipping admin port update...\n", port->name);
	}else{
		port->up = !(admin_state == OFDPA_PORT_CONFIG_DOWN);
	}

	//Retrieve the features
	err = ofdpaPortFeatureGet(ps->id, &features);
	if(err != OFDPA_E_NONE){
		ROFL_WARN(DRIVER_NAME "[ports] WARNING: unable to recover features for port '%s' via ofdpaPortFeatureGet() call. Skipping features port update...\n", port->name);
	}else{
		//TODO: FIXME
	}
}

//Set port queues
static rofl_result_t port_set_queues(switch_port_t* port, ofdpa_port_state_t* ps){

	OFDPA_ERROR_t err;
	uint32_t n_queues, i;
	ofdpaPortQueueStats_t stats;
	char queue_name[PORT_QUEUE_MAX_LEN_NAME];

	err = ofdpaNumQueuesGet(ps->id, &n_queues);
	if(err != OFDPA_E_NONE){
		ROFL_ERR(DRIVER_NAME "[ports] ERROR: unable to retrieve the number of HW queues for port '%s'\n",
												port->name);
		assert(0);
		return ROFL_FAILURE;
	}

	//Fill in queue info in the pipeline
	for(i=0; i<n_queues; i++){
		//Get queue stats
		err = ofdpaQueueStatsGet(ps->id, i, &stats);
		if(err != OFDPA_E_NONE){
			ROFL_ERR(DRIVER_NAME "[ports] ERROR: unable to retrieve information for HW queue '%u' in port '%s'\n",
												i,
												port->name);
			assert(0);
			return ROFL_FAILURE;
		}

		//Add to the pipeline
		snprintf(queue_name, PORT_QUEUE_MAX_LEN_NAME, "%s%d", "queue", i);
		if(switch_port_add_queue(port, i, (char*)&queue_name, 0, 0, 0) != ROFL_SUCCESS){
			ROFL_ERR(DRIVER_NAME "[ports] ERROR: unable to set queue '%u' information for port '%s'\n",
												i,
												port->name);
			assert(0);
			return ROFL_FAILURE;
		}
	}

	return ROFL_SUCCESS;
}

//Create new representation in the rofl_pipeline data-model
//for the OFDPA port
static switch_port_t* create_port(uint32_t id){

	switch_port_t* port;
	ofdpa_port_state_t* ps;
	char tmp[OFDPA_PORT_NAME_STRING_SIZE];
	ofdpa_buffdesc buf;

	buf.size = OFDPA_PORT_NAME_STRING_SIZE;
	buf.pstart = tmp;

	//Allocate space
	if(ofdpaPortNameGet(id, &buf) != OFDPA_E_NONE){
		assert(0);
		ROFL_ERR(DRIVER_NAME "[ports] ERROR: unable to retrieve port name for OFDPA id '%u'\n", id);
		return NULL;
	}

	port = switch_port_init(tmp,
					true,
					PORT_TYPE_PHYSICAL,
					PORT_STATE_NONE);

	//Add the OFDPA specifics
	ps = (ofdpa_port_state_t*)malloc(sizeof(ofdpa_port_state_t));
	if(!port->platform_port_state){
		assert(0);
		ROFL_ERR(DRIVER_NAME "[ports] Unable to allocate driver's internal-state memory for port '%s'; out of memory?\n", tmp);
		return NULL;
	}
	port->platform_port_state = ps;

	//Set port queues (pipeline state)
	if(port_set_queues(port, ps) != ROFL_SUCCESS){
		assert(0);
		return NULL;
	}

	//Fill-in capabilities and update state
	port_update_state(port, ps);

	//Return the pointer
	return port;
}

rofl_result_t ofdpa_discover_ports(void){

	OFDPA_ERROR_t err;
	uint32_t port = 0, next_port;
	switch_port_t* sw_port;

	ROFL_DEBUG(DRIVER_NAME "[ports] Starting port discovery...\n");

	err = ofdpaPortNextGet(port, &next_port);
	while(err == OFDPA_E_NONE){

		//Create port
		sw_port = create_port(port);
		if(!sw_port){
			assert(0);
			return ROFL_FAILURE;
		}
		ROFL_INFO(DRIVER_NAME"[ports] Discovered interface '%s' mac_addr %02X:%02X:%02X:%02X:%02X:%02X \n",
												sw_port->name);
		//Add to the pipeline
		if(physical_switch_add_port(sw_port) != ROFL_SUCCESS ){
			ROFL_ERR(DRIVER_NAME "[ports] ERROR: all physical port slots are occupied\n");
			assert(0);
			return ROFL_FAILURE;
		}


		//Continue
		port = next_port;
	}

	return ROFL_SUCCESS;
}

rofl_result_t ofdpa_bring_port_up(switch_port_t* port){

	rofl_result_t res = ROFL_SUCCESS;
	OFDPA_PORT_CONFIG_t admin_state;
	ofdpa_port_state_t* ps;
	OFDPA_ERROR_t error;

	if(!port)
		return ROFL_FAILURE;

	ps = (ofdpa_port_state_t*) port->platform_port_state;
	if(!ps){
		assert(0); //can NEVER happen
		return ROFL_FAILURE;
	}

	pthread_mutex_lock(&mutex);

	//Now retrieve the current state
	error = ofdpaPortConfigGet(ps->id, &admin_state);
	if(error != OFDPA_E_NONE){
		ROFL_ERR(DRIVER_NAME "[ports] Unable to recover admin state for port '%s' via ofdpaPortConfigGet() call. OFDPA error: '%s'\n", port->name, (error == OFDPA_E_PARAM)? "invalid parameters" : "Unknown");
		res = ROFL_FAILURE;
		goto UP_END;
	}

	if(admin_state == OFDPA_PORT_CONFIG_DOWN){
		admin_state = !OFDPA_PORT_CONFIG_DOWN;
		error = ofdpaPortConfigSet(ps->id, admin_state);
		if(error != OFDPA_E_NONE){
			ROFL_ERR(DRIVER_NAME "[ports] Unable to bring up admin state for port '%s' via ofdpaPortConfigGet() call. OFDPA error: '%s'\n", port->name, (error == OFDPA_E_PARAM)? "invalid parameters" : "Unknown");
		res = ROFL_FAILURE;
		}
	}

UP_END:
	pthread_mutex_unlock(&mutex);

	return res;
}

rofl_result_t ofdpa_bring_port_down(switch_port_t* port){

	rofl_result_t res = ROFL_SUCCESS;
	OFDPA_PORT_CONFIG_t admin_state;
	ofdpa_port_state_t* ps;
	OFDPA_ERROR_t error;

	if(!port)
		return ROFL_FAILURE;

	ps = (ofdpa_port_state_t*) port->platform_port_state;
	if(!ps){
		assert(0); //can NEVER happen
		return ROFL_FAILURE;
	}

	pthread_mutex_lock(&mutex);

	//Now retrieve the current state
	error = ofdpaPortConfigGet(ps->id, &admin_state);
	if(error != OFDPA_E_NONE){
		ROFL_ERR(DRIVER_NAME "[ports] Unable to recover admin state for port '%s' via ofdpaPortConfigGet() call. OFDPA error: '%s'\n", port->name, (error == OFDPA_E_PARAM)? "invalid parameters" : "Unknown");
		res = ROFL_FAILURE;
		goto DOWN_END;
	}

	if(admin_state != OFDPA_PORT_CONFIG_DOWN){
		admin_state = OFDPA_PORT_CONFIG_DOWN;
		error = ofdpaPortConfigSet(ps->id, admin_state);
		if(error != OFDPA_E_NONE){
			ROFL_ERR(DRIVER_NAME "[ports] Unable to bring down admin state for port '%s' via ofdpaPortConfigGet() call. OFDPA error: '%s'\n", port->name, (error == OFDPA_E_PARAM)? "invalid parameters" : "Unknown");
		res = ROFL_FAILURE;
		}
	}

DOWN_END:
	pthread_mutex_unlock(&mutex);

	return res;
}
