#include "pm_ext.h"
#include <assert.h>
#include "../port_manager.h"
#include "../system_manager.h"

using namespace rofl;
using namespace xdpd;

#define MODULE_NAME "[xdpd][port_manager_extensions]"

//
// NF creation/destruction
//
void port_manager_extensions::create_nf_port(std::string& nf_name, std::string& nf_port_name, port_type_t nf_type)
{

	hal_extension_ops_t* ops = system_manager::__get_driver_hal_extension_ops();

	if(!ops->nf_ports.create_nf_port)
		throw ePmExtNotsupportedByDriver();

	if(nf_type == PORT_TYPE_NF_NATIVE)
	{
		ROFL_ERR("%s ERROR: only SHMEM or EXTERNAL types are currently implemented\n", MODULE_NAME);
		throw ePmExtInvalidNF();
	}

	// Serialize
	pthread_mutex_lock(&port_manager::mutex);

	// The NF port must not exist
	if(port_manager::exists(nf_port_name))
	{
		pthread_mutex_unlock(&port_manager::mutex);
		ROFL_ERR("%s ERROR: Attempting to create an existent NF port %s\n", MODULE_NAME, nf_port_name.c_str());
		throw ePmExtInvalidNF();
	}

	if((*ops->nf_ports.create_nf_port)(nf_name.c_str(), nf_port_name.c_str(), nf_type) != HAL_SUCCESS)
	{
		pthread_mutex_unlock(&port_manager::mutex);
		assert(0);
		ROFL_ERR("%s ERROR: Driver was unable to create the NF port\n", MODULE_NAME, nf_port_name.c_str());
		throw ePmExtUnknownError();
	}

	ROFL_INFO("%s Successfully created NF port %s of type %s, {%s}\n", MODULE_NAME, nf_port_name.c_str(), (nf_type == PORT_TYPE_NF_SHMEM)? "SHMEM": "EXTERNAL", nf_name.c_str() );

	//TODO: notify the creation to the plugins?

	//Release mutex
	pthread_mutex_unlock(&port_manager::mutex);
}

void port_manager_extensions::destroy_nf_port(std::string& nf_port_name)
{

	hal_extension_ops_t* ops = system_manager::__get_driver_hal_extension_ops();

	if(!ops->nf_ports.destroy_nf_port)
		throw ePmExtNotsupportedByDriver();
	// Serialize
	pthread_mutex_lock(&port_manager::mutex);

	// The NF must exist
	if(!port_manager::exists(nf_port_name))
	{
		pthread_mutex_unlock(&port_manager::mutex);
		ROFL_ERR("%s ERROR: Attempting to destroy a non-existent NF port %s\n", MODULE_NAME, nf_port_name.c_str());
		throw ePmExtInvalidNF();
	}

	if((*ops->nf_ports.destroy_nf_port)(nf_port_name.c_str()) != HAL_SUCCESS)
	{
		pthread_mutex_unlock(&port_manager::mutex);
		assert(0);
		ROFL_ERR("%s ERROR: Driver was unable to destroy the NF port %s\n", MODULE_NAME, nf_port_name.c_str());
		throw ePmExtUnknownError();
	}

	ROFL_INFO("%s NF port %s destroyed\n", MODULE_NAME, nf_port_name.c_str());

	//TODO: notify the destruction to the plugins?

	//Release mutex
	pthread_mutex_unlock(&port_manager::mutex);
}

