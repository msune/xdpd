#!/bin/bash

# check for veth0
ip link show veth0 >/dev/null 2>&1
if [ 0 -ne $? ]; then
	# create veth0/veth1
	sudo ip link add type veth
fi

set -e

##
#List of tests
##

#Mockup
sudo ./test_portmockup
sudo ./test_portmockup_multiport

if [ $DEBUG ];then 
	sudo ./test_portmockup_matchesmockup
	sudo ./test_portmockup_matchesmockup_multiport
	#test expirations of buffers in datapacket_storage by background task manager
	sudo ./test_storage_packets_expiration
fi

if [ ! $TRAVIS_CI ];then 
#MMAP
sudo ./test_portmmap

#test for the hcl notifications for port events (add, delete & status change)
sudo ./test_port_status
fi
