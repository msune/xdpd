#include <stdio.h>
#include <net/if.h>
#include <rofl/datapath/hal/driver.h>
#include <rofl/common/utils/c_logger.h>
#include <rofl/datapath/hal/cmm.h>
#include <rofl/datapath/pipeline/platform/memory.h>
#include <rofl/datapath/pipeline/physical_switch.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/of1x_switch.h>

#include <stdlib.h>
#include <string.h>
#include <rofl/datapath/pipeline/openflow/of_switch.h>
#include <rofl/datapath/pipeline/common/datapacket.h>

//OFDPA includes
#include <ofdpa_api.h>

//Own includes
#include "../ofdpa/ports.h"

//Driver static info
#define EXAMPLE_CODE_NAME "ofdpa"
#define EXAMPLE_VERSION VERSION
#define EXAMPLE_DESC  "OFDPA driver.\n\nThe OFDPA driver supports Broadcom's OFDPA compliant switches."
#define EXAMPLE_USAGE  ""

//Here we are going to store the parameters that we got
//and that we were able to parse (understand) and execute
char extra_params[DRIVER_EXTRA_PARAMS_MAX_LEN];

//Static pipeline LSI (the only one)
static of_switch_t* sw=NULL;

//Number of tables needed by OFDPA (max table)
#define OFDPA_NUM_OF_TABLES OFDPA_FLOW_TABLE_ID_ACL_POLICY

/*
* @name    hal_driver_init
* @brief   Initializes driver. Before using the HAL_DRIVER routines, higher layers must allow driver to initialize itself
* @ingroup driver_management
*/
hal_result_t hal_driver_init(hal_extension_ops_t* extensions, const char* _extra_params){


	ROFL_INFO("["DRIVER_NAME"] calling hal_driver_init()\n");

	//If using ROFL-PIPELINE, the physical switch must be inited
	if(physical_switch_init() != ROFL_SUCCESS)
		return HAL_FAILURE;


	//Discover the phyiscal ports of the switch
	discover_ports();

	//Initialize some form of background task manager

	//And initialize or setup any other state your platform needs...

	ROFL_INFO("["DRIVER_NAME"] Initializing OFDPA driver...\n");

	//Call OFDPA init routine
	OFDPA_ERROR_t res = ofdpaClientInitialize("test");
	if(res != OFDPA_E_NONE){
		ROFL_ERR("["DRIVER_NAME"] Unable to initialize the OFDPA. Error: %d\n", res);
		return HAL_FAILURE;
	}

	strncpy(extra_params, _extra_params, DRIVER_EXTRA_PARAMS_MAX_LEN);

	//We don't support any HAL extension
	memset(extensions, 0, sizeof(hal_extension_ops_t));

	return HAL_SUCCESS;
}

/**
* @name    hal_driver_get_info
* @brief   Get the information of the driver (code-name, version, usage...)
* @ingroup driver_management
*/
void hal_driver_get_info(driver_info_t* info){
	//Fill-in driver_info_t
	strncpy(info->code_name, EXAMPLE_CODE_NAME, DRIVER_CODE_NAME_MAX_LEN);
	strncpy(info->version, EXAMPLE_VERSION, DRIVER_VERSION_MAX_LEN);
	strncpy(info->description, EXAMPLE_DESC, DRIVER_DESCRIPTION_MAX_LEN);
	strncpy(info->usage, EXAMPLE_USAGE, DRIVER_USAGE_MAX_LEN);
	strncpy(info->extra_params, extra_params, DRIVER_EXTRA_PARAMS_MAX_LEN);
}


/*
* @name    hal_driver_destroy
* @brief   Destroy driver state. Allows platform state to be properly released.
* @ingroup driver_management
*/
hal_result_t hal_driver_destroy(){

	//In this function you allow the platform
	//to be properly cleaning its own state

	//If using the pipeline you should call
	//physical_switch_destroy();

	ROFL_INFO("["DRIVER_NAME"] calling hal_driver_destroy()\n");

	return HAL_SUCCESS;
}

/*
* Switch management functions
*/
bool hal_driver_switch_exists(uint64_t dpid){
	return physical_switch_get_logical_switch_by_dpid(dpid) != NULL;
}

/**
* @brief   Retrieve the list of LSIs dpids
* @ingroup logical_switch_management
* @retval  List of available dpids, which MUST be deleted using dpid_list_destroy().
*/
dpid_list_t* hal_driver_get_all_lsi_dpids(void){
	return physical_switch_get_all_lsi_dpids();
}

/**
 * @name hal_driver_get_switch_snapshot_by_dpid
 * @brief Retrieves a snapshot of the current state of a switch port, if the port name is found. The snapshot MUST be deleted using switch_port_destroy_snapshot()
 * @ingroup logical_switch_management
 * @retval  Pointer to of_switch_snapshot_t instance or NULL
 */
of_switch_snapshot_t* hal_driver_get_switch_snapshot_by_dpid(uint64_t dpid){
	return physical_switch_get_logical_switch_snapshot(dpid);
}


/*
* @name    hal_driver_create_switch
* @brief   Instruct driver to create an OF logical switch
* @ingroup logical_switch_management
* @retval  Pointer to of_switch_t instance
*/
hal_result_t hal_driver_create_switch(char* name, uint64_t dpid, of_version_t of_version, unsigned int num_of_tables, int* ma_list){

	//We only accept one logical switch in this driver
	if(sw){
		ROFL_ERR("["DRIVER_NAME"] ERROR: OFDPA driver only supports 1 logical switch!\n");
		return HAL_FAILURE;
	}

	//With a certain number of tables
	if(num_of_tables != OFDPA_NUM_OF_TABLES){
		ROFL_ERR("["DRIVER_NAME"] ERROR: OFDPA driver only supports (exactly) %u table!\n", OFDPA_NUM_OF_TABLES);
		return HAL_FAILURE;
	}

	//C
	sw = (of_switch_t*)of1x_init_switch(name, of_version, dpid, num_of_tables, (enum of1x_matching_algorithm_available*) ma_list);
	if(!sw){
		ROFL_ERR("["DRIVER_NAME"] ERROR: unable to create pipeline LSI.\n");
		return HAL_FAILURE;
	}

	//Adding switch to the bank
	physical_switch_add_logical_switch(sw);

	ROFL_DEBUG("["DRIVER_NAME"] Creating LSI named: %s, num. of tables: %u, version: 0x%x\n",
											name,
											num_of_tables,
											of_version);
	return HAL_SUCCESS;
}

/*
* @name    hal_driver_destroy_switch_by_dpid
* @brief   Instructs the driver to destroy the switch with the specified dpid
* @ingroup logical_switch_management
*/
hal_result_t hal_driver_destroy_switch_by_dpid(const uint64_t dpid){

	if(!sw)
		return HAL_FAILURE;
	if(sw->dpid != dpid)
		return HAL_FAILURE;

	//FIXME: destroy rules

	//Detach ports from switch. Do not feed more packets to the switch
	if(physical_switch_detach_all_ports_from_logical_switch(sw)!=ROFL_SUCCESS)
		return HAL_FAILURE;

	//Remove switch from the switch bank
	if(physical_switch_remove_logical_switch(sw)!=ROFL_SUCCESS)
		return HAL_FAILURE;

	//Set pointer sw pointer so that it can be recreated in the future
	sw=NULL;

	return HAL_SUCCESS;
}

/*
* Port management
*/

/**
* @brief   Checks if a port with the specified name exists
* @ingroup port_management
*/
bool hal_driver_port_exists(const char *name){
	return physical_switch_get_port_by_name(name) != NULL;
}

/**
* @brief   Retrieve the list of names of the available ports of the platform. You may want to
* 	   call hal_driver_get_port_snapshot_by_name(name) to get more information of the port
* @ingroup port_management
* @retval  List of available port names, which MUST be deleted using switch_port_name_list_destroy().
*/
switch_port_name_list_t* hal_driver_get_all_port_names(void){
	return physical_switch_get_all_port_names();
}

/**
 * @name hal_driver_get_port_by_name
 * @brief Retrieves a snapshot of the current state of a switch port, if the port name is found. The snapshot MUST be deleted using switch_port_destroy_snapshot()
 * @ingroup port_management
 */
switch_port_snapshot_t* hal_driver_get_port_snapshot_by_name(const char *name){
	return physical_switch_get_port_snapshot(name);
}

/**
 * @name hal_driver_get_port_by_num
 * @brief Retrieves a snapshot of the current state of the port of the Logical Switch Instance with dpid at port_num, if exists. The snapshot MUST be deleted using switch_port_destroy_snapshot()
 * @ingroup port_management
 * @param dpid DatapathID
 * @param port_num Port number
 */
switch_port_snapshot_t* hal_driver_get_port_snapshot_by_num(uint64_t dpid, unsigned int port_num){

	of_switch_t* lsw;

	lsw = physical_switch_get_logical_switch_by_dpid(dpid);
	if(!lsw)
		return NULL;

	//Check if the port does exist.
	if(!port_num || port_num >= LOGICAL_SWITCH_MAX_LOG_PORTS || !lsw->logical_ports[port_num].port)
		return NULL;

	return physical_switch_get_port_snapshot(lsw->logical_ports[port_num].port->name);
}


/*
* @name    hal_driver_attach_physical_port_to_switch
* @brief   Attemps to attach a system's port to switch, at of_port_num if defined, otherwise in the first empty OF port number.
* @ingroup management
*
* @param dpid Datapath ID of the switch to attach the ports to
* @param name Port name (system's name)
* @param of_port_num If *of_port_num is non-zero, try to attach to of_port_num of the logical switch, otherwise try to attach to the first available port and return the result in of_port_num
*/
hal_result_t hal_driver_attach_port_to_switch(uint64_t dpid, const char* name, unsigned int* of_port_num){

	switch_port_t* port;
	of_switch_t* lsw;

	//Check switch existance
	lsw = physical_switch_get_logical_switch_by_dpid(dpid);
	if(!lsw){
		return HAL_FAILURE;
	}

	//Check if the port does exist
	port = physical_switch_get_port_by_name(name);
	if(!port)
		return HAL_FAILURE;

	//Update pipeline state
	if(*of_port_num == 0){
		//no port specified, we assign the first available
		if(physical_switch_attach_port_to_logical_switch(port,lsw,of_port_num) == ROFL_FAILURE){
			assert(0);
			return HAL_FAILURE;
		}
	}else{

		if(physical_switch_attach_port_to_logical_switch_at_port_num(port,lsw,*of_port_num) == ROFL_FAILURE){
			assert(0);
			return HAL_FAILURE;
		}
	}

	return HAL_SUCCESS;
}

/**
* @name    hal_driver_connect_switches
* @brief   Attemps to connect two logical switches via a virtual port. Driver may or may not support this functionality.
* @ingroup management
*
* @param dpid_lsi1 Datapath ID of the LSI1
* @param dpid_lsi2 Datapath ID of the LSI2
*/
hal_result_t hal_driver_connect_switches(uint64_t dpid_lsi1, unsigned int* port_num1, switch_port_snapshot_t** port1, uint64_t dpid_lsi2, unsigned int* port_num2, switch_port_snapshot_t** port2){
	ROFL_INFO("["DRIVER_NAME"] ERROR: OFDPA doesn't currently support multiple LSIs!\n");
	return HAL_FAILURE;
}

/*
* @name    hal_driver_detach_port_from_switch
* @brief   Detaches a port from the switch
* @ingroup port_management
*
* @param dpid Datapath ID of the switch to detach the ports
* @param name Port name (system's name)
*/
hal_result_t hal_driver_detach_port_from_switch(uint64_t dpid, const char* name){

	of_switch_t* lsw;
	switch_port_t* port;
	switch_port_snapshot_t *port_snapshot=NULL;

	lsw = physical_switch_get_logical_switch_by_dpid(dpid);
	if(!lsw)
		return HAL_FAILURE;

	port = physical_switch_get_port_by_name(name);

	//Check if the port does exist and is really attached to the dpid
	if( !port || !port->attached_sw || port->attached_sw->dpid != dpid)
		return HAL_FAILURE;

	//Snapshoting the port *before* it is detached
	port_snapshot = physical_switch_get_port_snapshot(port->name);

	if(!port_snapshot){
		assert(0);
		return HAL_FAILURE;
	}

	//Detach it
	if(physical_switch_detach_port_from_logical_switch(port,lsw) != ROFL_SUCCESS){
		ROFL_ERR("["DRIVER_NAME"] ERROR: error detaching port %s.\n",port->name);
		assert(0);
		goto DRIVER_DETACH_ERROR;
	}

	return HAL_SUCCESS;

DRIVER_DETACH_ERROR:
	if(port_snapshot)
		switch_port_destroy_snapshot(port_snapshot);

	return HAL_FAILURE;
}

/*
* @name    hal_driver_detach_port_from_switch_at_port_num
* @brief   Detaches port_num of the logical switch identified with dpid
* @ingroup port_management
*
* @param dpid Datapath ID of the switch to detach the ports
* @param of_port_num Number of the port (OF number)
*/
hal_result_t hal_driver_detach_port_from_switch_at_port_num(uint64_t dpid, const unsigned int of_port_num){

	of_switch_t* lsw;

	lsw = physical_switch_get_logical_switch_by_dpid(dpid);
	if(!lsw)
		return HAL_FAILURE;

	//Check if the port does exist.
	if(!of_port_num || of_port_num >= LOGICAL_SWITCH_MAX_LOG_PORTS || !lsw->logical_ports[of_port_num].port)
		return HAL_FAILURE;

	return hal_driver_detach_port_from_switch(dpid, lsw->logical_ports[of_port_num].port->name);
}


//Port admin up/down stuff

/*
* Port administrative management actions (ifconfig up/down like)
*/

/*
* @name    hal_driver_bring_port_up
* @brief   Brings up a system port. If the port is attached to an OF logical switch, this also schedules port for I/O and triggers PORTMOD message.
* @ingroup port_management
*
* @param name Port system name
*/
hal_result_t hal_driver_bring_port_up(const char* name){

	switch_port_t* port;
	switch_port_snapshot_t* port_snapshot;

	//Check if the port does exist
	port = physical_switch_get_port_by_name(name);

	if(!port || !port->platform_port_state)
		return HAL_FAILURE;

	//Bring it up
	if(bring_port_up(port) != ROFL_SUCCESS)
		return HAL_FAILURE;

	port_snapshot = physical_switch_get_port_snapshot(port->name);
	hal_cmm_notify_port_status_changed(port_snapshot);

	return HAL_SUCCESS;
}

/*
* @name    hal_driver_bring_port_down
* @brief   Shutdowns (brings down) a system port. If the port is attached to an OF logical switch, this also de-schedules port and triggers PORTMOD message.
* @ingroup port_management
*
* @param name Port system name
*/
hal_result_t hal_driver_bring_port_down(const char* name){

	switch_port_t* port;
	switch_port_snapshot_t* port_snapshot;

	//Check if the port does exist
	port = physical_switch_get_port_by_name(name);
	if(!port || !port->platform_port_state)
		return HAL_FAILURE;

	//Bring it down
	if(bring_port_down(port) != ROFL_SUCCESS)
		return HAL_FAILURE;

	port_snapshot = physical_switch_get_port_snapshot(port->name);
	hal_cmm_notify_port_status_changed(port_snapshot);

	return HAL_SUCCESS;
}


/*
* @name    hal_driver_bring_port_up_by_num
* @brief   Brings up a port from an OF logical switch (and the underlying physical interface). This function also triggers the PORTMOD message
* @ingroup port_management
*
* @param dpid DatapathID
* @param port_num OF port number
*/
hal_result_t hal_driver_bring_port_up_by_num(uint64_t dpid, unsigned int port_num){

	of_switch_t* lsw;

	lsw = physical_switch_get_logical_switch_by_dpid(dpid);
	if(!lsw)
		return HAL_FAILURE;

	//Check if the port does exist and is really attached to the dpid
	if( !lsw->logical_ports[port_num].port || lsw->logical_ports[port_num].attachment_state != LOGICAL_PORT_STATE_ATTACHED || lsw->logical_ports[port_num].port->attached_sw->dpid != dpid)
		return HAL_FAILURE;

	return hal_driver_bring_port_up(lsw->logical_ports[port_num].port->name);
}

/*
* @name    hal_driver_bring_port_down_by_num
* @brief   Brings down a port from an OF logical switch (and the underlying physical interface). This also triggers the PORTMOD message.
* @ingroup port_management
*
* @param dpid DatapathID
* @param port_num OF port number
*/
hal_result_t hal_driver_bring_port_down_by_num(uint64_t dpid, unsigned int port_num){

	of_switch_t* lsw;

	lsw = physical_switch_get_logical_switch_by_dpid(dpid);
	if(!lsw)
		return HAL_FAILURE;

	//Check if the port does exist and is really attached to the dpid
	if( !lsw->logical_ports[port_num].port || lsw->logical_ports[port_num].attachment_state != LOGICAL_PORT_STATE_ATTACHED || lsw->logical_ports[port_num].port->attached_sw->dpid != dpid)
		return HAL_FAILURE;

	return hal_driver_bring_port_down(lsw->logical_ports[port_num].port->name);
}

/**
 * @brief Retrieve a snapshot of the monitoring state. If rev is 0, or the current monitoring
 * has changed (monitoring->rev != rev), a new snapshot of the monitoring state is made. Warning: this
 * is expensive.
 * @ingroup hal_driver_management
 *
 * @param rev Last seen revision. Set to 0 to always get a new snapshot
 * @return A snapshot of the monitoring state that MUST be destroyed using monitoring_destroy_snapshot() or NULL if there have been no changes (same rev)
 */
monitoring_snapshot_state_t* hal_driver_get_monitoring_snapshot(uint64_t rev){

	monitoring_state_t* mon = physical_switch_get_monitoring();

	if( rev == 0 || monitoring_has_changed(mon, &rev) )
		return monitoring_get_snapshot(mon);

	return NULL;
}

/**
 * @brief get a list of available matching algorithms
 * @ingroup hal_driver_management
 *
 * @param of_version
 * @param name_list
 * @param count
 * @return
 */
hal_result_t hal_driver_list_matching_algorithms(of_version_t of_version, const char * const** name_list, int *count){

	if(of_get_switch_matching_algorithms(of_version, name_list, count) == ROFL_SUCCESS)
		return HAL_SUCCESS;
	return HAL_FAILURE;
}
