#pragma once

#include <machine/nic.h>

class UDPNIC : public NIC<Ethernet>
{
	// TCB - implementar todas essas funções.

	virtual int send(const Address & dst, const Protocol & prot, const void * data, unsigned int size) { return 0; }
	virtual int receive(Address * src, Protocol * prot, void * data, unsigned int size) { return 0; }

	virtual Buffer * alloc(const Address & dst, const Protocol & prot, unsigned int once, unsigned int always, unsigned int payload) { return 0; }
	virtual int send(Buffer * buf) { return 0; }
	virtual void free(Buffer * buf) { }

	virtual const Address & address() { return *(new Address()); }
	virtual void address(const Address &) { }

	virtual bool reconfigure(const Configuration * c = 0) { return false; }
	virtual const Configuration & configuration() { return *(new Configuration()); }

	virtual const Statistics & statistics() { return *(new Statistics()); }
};