#include "node_orchestrator.h"
#include <rofl/common/utils/c_logger.h>

using namespace xdpd;

//FIXME: protect this variable with a mutex?
uint64_t NodeOrchestrator::nextDpid = 0x1;

NodeOrchestrator::NodeOrchestrator() :
		socket(NULL)
{
	socket = rofl::csocket::csocket_factory(rofl::csocket::SOCKET_TYPE_PLAIN, this);

	socket_params = rofl::csocket::get_default_params(rofl::csocket::SOCKET_TYPE_PLAIN);

	socket_params.set_param(rofl::csocket::PARAM_KEY_LOCAL_HOSTNAME).set_string(MGMT_ADDR);
	socket_params.set_param(rofl::csocket::PARAM_KEY_LOCAL_PORT).set_string(MGMT_PORT);
	socket_params.set_param(rofl::csocket::PARAM_KEY_DOMAIN).set_string("inet");
	socket_params.set_param(rofl::csocket::PARAM_KEY_TYPE).set_string("stream");
	socket_params.set_param(rofl::csocket::PARAM_KEY_PROTOCOL).set_string(MGMT_PROTOCOL);
}

NodeOrchestrator::~NodeOrchestrator()
{
	socket->close();
}

void NodeOrchestrator::init()
{

	ROFL_INFO("\n\n[xdpd]["PLUGIN_NAME"] **************************\n");	
	ROFL_INFO("[xdpd]["PLUGIN_NAME"] Plugin receiving commands from the node orchestrator.\n");
	ROFL_INFO("[xdpd]["PLUGIN_NAME"] **************************\n\n");
	
	socket->listen(socket_params);
};

vector<rofl::coption> NodeOrchestrator::get_options(void)
{
		vector<rofl::coption> vec;
		return vec;
};

void NodeOrchestrator::handle_listen(rofl::csocket& socket, int newsd)
{	
	rofl::csocket *worker_socket = rofl::csocket::csocket_factory(rofl::csocket::SOCKET_TYPE_PLAIN, this);	
	worker_socket->accept(rofl::csocket::get_default_params(rofl::csocket::SOCKET_TYPE_PLAIN),newsd);
}


void NodeOrchestrator::handle_accepted(csocket& socket)
{	
	return handle_read(socket);
}

void NodeOrchestrator::handle_write(rofl::csocket& socket)
{
	socket.close();
}

void NodeOrchestrator::handle_read(rofl::csocket& socket)
{		
	cmemory mem(BUFFER_SIZE);
		
	int ReadBytes;
	
	try
	{
		ReadBytes = socket.recv(mem.somem(), mem.memlen());
	}catch(...)
	{
		return;
	}

	if (ReadBytes <= 0) 
	{
		// socket closed
		ROFL_INFO("[xdpd]["PLUGIN_NAME"] Reading socket failed, errno: %d (%s)\n",errno,strerror(errno));
		return;
	}
	
	char DataBuffer[BUFFER_SIZE];
	memcpy(DataBuffer,(char*)mem.somem(),ReadBytes);
	DataBuffer[ReadBytes] = '\0';

	ROFL_INFO("[xdpd]["PLUGIN_NAME"] Data received (%d bytes):\n",ReadBytes);
	ROFL_INFO("[xdpd]["PLUGIN_NAME"] %s\n",DataBuffer);

	string message = MessageHandler::processCommand(string(DataBuffer));
	const char *answer = message.c_str();
	
	ROFL_INFO("[xdpd]["PLUGIN_NAME"] Answer to be sent: %s\n",answer);

	rofl::csockaddr const& raddr = socket.get_raddr();

	cmemory *mem2 = new cmemory((uint8_t*)answer,message.length());
	
	socket.send(mem2,raddr);
	
};


LSI NodeOrchestrator::createLSI(list<string> phyPorts, string controllerAddress, string controllerPort)
{
	map<string,unsigned int> ports;

	uint64_t dpid = nextDpid;	
	nextDpid++;
		
	stringstream lsiName_ss;
	lsiName_ss << dpid;
	string lsiName = lsiName_ss.str();

	rofl::cparams socket_params = rofl::csocket::get_default_params(rofl::csocket::SOCKET_TYPE_PLAIN);
	socket_params.set_param(rofl::csocket::PARAM_KEY_REMOTE_HOSTNAME) = controllerAddress;
	socket_params.set_param(rofl::csocket::PARAM_KEY_REMOTE_PORT) = controllerPort; 
	socket_params.set_param(rofl::csocket::PARAM_KEY_DOMAIN) = string("inet"); 
	int ma_list[OF1X_MAX_FLOWTABLES] = { 0 };
	
	try
	{
		switch_manager::create_switch(OFVERSION, dpid,lsiName,NUM_TABLES,ma_list,RECONNECT_TIME,rofl::csocket::SOCKET_TYPE_PLAIN,socket_params);
	} catch (...) {
		ROFL_ERR("[xdpd]["PLUGIN_NAME"] Unable to create the LSI\n");
		throw;
	}	

	//Attach ports
	list<string>::iterator port_it;
	unsigned int i;
	for(port_it = phyPorts.begin(), i=1; port_it != phyPorts.end(); ++port_it, ++i)
	{		
		//Ignore empty ports	
		if(*port_it == "")
			continue;
	
		try{
			//Attach
			port_manager::attach_port_to_switch(dpid, *port_it, &i);
			
			//Bring up
			port_manager::bring_up(*port_it);
		}catch(...){	
			ROFL_ERR("[xdpd]["PLUGIN_NAME"] Unable to attach port '%s'",(*port_it).c_str());
			throw;
		}
		ports[*port_it] = i;
	}
	
	return LSI(dpid,ports);
}

set<string> NodeOrchestrator::discoverPhyPorts()
{
	set<string> availablePorts =  port_manager::list_available_port_names();
	set<string>::iterator port = availablePorts.begin();
	ROFL_INFO("[xdpd]["PLUGIN_NAME"] Number of ports available: %d\n", availablePorts.size());
	for(; port != availablePorts.end(); port++)
	{
		ROFL_INFO("[xdpd]["PLUGIN_NAME"] %s\n",(*port).c_str());	
	}
	
	return availablePorts;
}

unsigned int NodeOrchestrator::attachPhyPort(uint64_t dpid, string port)
{
	unsigned int portID;
	try{
		//Attach
		port_manager::attach_port_to_switch(dpid, port, &portID);
		
		//Bring up
		port_manager::bring_up(port);
	}catch(...){	
		ROFL_ERR("[xdpd]["PLUGIN_NAME"] Unable to attach port '%s'",port.c_str());
		throw;
	}
	
	return portID;
}

pair<unsigned int, unsigned int> NodeOrchestrator::createVirtualLink(uint64_t dpid_a,uint64_t dpid_b)
{
	string name_port_a;
	string name_port_b;
	unsigned int port_a = 0, port_b = 0;
	
	try
	{
		port_manager::connect_switches(dpid_a, &port_a, name_port_a, dpid_b, &port_b, name_port_b);
	}catch(...)
	{
		ROFL_ERR("[xdpd]["PLUGIN_NAME"] Unable to create a virtual link between the switch '%d' and '%d'\n",dpid_a,dpid_b);
		throw;
	}
	
	ROFL_INFO("[xdpd]["PLUGIN_NAME"] Virtual link created - %x:%s <-> %x:%s\n", dpid_a,name_port_a.c_str(),dpid_b,name_port_b.c_str());
	
	xdpd::port_snapshot port_snapshot_a;
	port_manager::get_port_info(name_port_a,port_snapshot_a);
	
	
	xdpd::port_snapshot port_snapshot_b;
	port_manager::get_port_info(name_port_b,port_snapshot_b);
	
				
	return make_pair(port_a,port_b);
}

unsigned int NodeOrchestrator::createNfPort(uint64_t dpid, string NfName, string NfPortName, port_type_t type)
{
	unsigned int port_number = 0;

	try
	{
		nf_port_manager::create_nf_port(NfName, NfPortName,type);	
	}catch(...)
	{
		ROFL_ERR("[xdpd]["PLUGIN_NAME"] Unable to create the NF port %s",NfPortName.c_str());
		throw;
	}
	
	try
	{
		port_manager::attach_port_to_switch(dpid, NfPortName, &port_number);
	}catch(...)
	{
		nf_port_manager::destroy_nf_port(NfPortName);
		ROFL_ERR("[xdpd]["PLUGIN_NAME"] Unable to attach the NF port %s to the switch %d",NfPortName.c_str(),dpid);
		throw;
	}
	
	port_manager::bring_up(NfPortName);
	
	ROFL_INFO("[xdpd]["PLUGIN_NAME"] Port '%s' attached to port %d of LSI '%x'\n",NfPortName.c_str(),port_number,dpid);	
	
	return port_number;
}

void NodeOrchestrator::destroyLSI(uint64_t dpid)
{	
	ROFL_ERR("[xdpd]["PLUGIN_NAME"] Trying to destroy the LSI: %d\n",dpid);
	try
	{
		switch_manager::destroy_switch(dpid);
	} catch (...) {
		ROFL_ERR("[xdpd]["PLUGIN_NAME"] Unable to destroy LSI\n");
		throw;
	}	
	ROFL_ERR("[xdpd]["PLUGIN_NAME"] LSI %d destroyed\n",dpid);
}

bool NodeOrchestrator::destroyNfPort(uint64_t dpid, string NfPortName, bool detach)
{
	if(detach)
	{
		//Bring down the port
		port_manager::bring_down(NfPortName);
		try
		{
			//Detatch the port from the LSI
			port_manager::detach_port_from_switch(dpid, NfPortName);
		}catch(...)
		{	
			ROFL_ERR("[xdpd]["PLUGIN_NAME"] Unable to detatch port '%s' from LSI '%s'. Unknown error.\n", NfPortName.c_str(),dpid);
			return false;
		}
	}
	
	try
	{
		//Destroy the port
		nf_port_manager::destroy_nf_port(NfPortName);
	}catch(...)
	{
		ROFL_ERR("[xdpd]["PLUGIN_NAME"] Unable to destroy port '%s'. Unknown error.\n", NfPortName.c_str());
		return false;
	}
	
	ROFL_ERR("[xdpd]["PLUGIN_NAME"] Port %s destroyed\n",NfPortName.c_str());
	return true;
}

bool NodeOrchestrator::detachPort(uint64_t dpid, uint64_t portID, bool vlink)
{
	//TODO: check if it is a vlink. This must be done through the switch snapshot

	try
	{
		port_manager::detach_port_from_switch_by_num(dpid, portID);
	}catch(...)
	{
		ROFL_ERR("[xdpd]["PLUGIN_NAME"] Unable to detach port '%d' from switch %d. Unknown error.\n",portID,dpid);
	}
	
	ROFL_ERR("[xdpd]["PLUGIN_NAME"] Port %d detached from switch %d\n",portID,dpid);	
	return true;
}

bool NodeOrchestrator::detachPort(uint64_t dpid, string port, bool vlink)
{
	if(vlink && !port_manager::is_vlink(port))
	{
		ROFL_ERR("[xdpd]["PLUGIN_NAME"] The port '%s' is not a virtual link!\n",port.c_str());
		return false;
	}
	else if(!vlink && port_manager::is_vlink(port))
	{
		ROFL_ERR("[xdpd]["PLUGIN_NAME"] The port '%s' is a virtual link!\n",port.c_str());
		return false;
	}

	try
	{
		port_manager::detach_port_from_switch(dpid, port);
	}catch(...)
	{
		ROFL_ERR("[xdpd]["PLUGIN_NAME"] Unable to detach port '%s' from switch %d. Unknown error.\n",port.c_str(),dpid);
	}
	
	ROFL_ERR("[xdpd]["PLUGIN_NAME"] Port %s detached from switch %d\n",port.c_str(),dpid);	
	return true;
}

