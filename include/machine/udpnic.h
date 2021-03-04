#pragma once
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <machine/nic.h>
#include <utility/debug.h>

extern char* globalIPAddress;

class UDPNIC : public NIC<Ethernet>
{
private:
	int _socket = -1;
	fd_set _fd;
	sockaddr_in _remoteAddress;

public:
	UDPNIC()
	{
		db<UDPNIC>(TRC) << "UDPNIC()" << endl;

		_socket = socket(AF_INET, SOCK_DGRAM, 0);

		struct sockaddr_in servaddr;
		memset(&servaddr, '\0', sizeof(sockaddr));
		servaddr.sin_family = AF_INET;
		servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
		servaddr.sin_port = htons(5000);
		bind(_socket, (struct sockaddr *)&servaddr, sizeof(sockaddr));

		FD_ZERO(&_fd);
		FD_SET(_socket, &_fd);

		_remoteAddress.sin_family = AF_INET;
		_remoteAddress.sin_port = htons(5000);
		_remoteAddress.sin_addr.s_addr = inet_addr(globalIPAddress);
	}

#define NULL 0

	virtual int send(const Address & dst, const Protocol & prot, const void * data, unsigned int size)
	{
		db<UDPNIC>(TRC) << "UDPNIC::send(s=" << address() << ",d=" << dst << ",p=" << hex << prot << dec << ",d=" << data << ",s=" << size << ")" << endl;

		fd_set writefd;
		memcpy(&writefd, &_fd, sizeof(fd_set));
		struct timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;
		if (select(_socket + 1, NULL, &writefd, NULL, &timeout) > 0)
		{
			sendto(_socket, data, size, 0, (struct sockaddr *)&_remoteAddress,
				sizeof(_remoteAddress));
			return size;
		}

		return 0;
	}
	virtual int receive(Address * src, Protocol * prot, void * data, unsigned int size)
	{
		db<UDPNIC>(TRC) << "UDPNIC::receive(s=" << *src << ",p=" << hex << *prot << dec << ",d=" << data << ",s=" << size << ") => " << endl;

		struct timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;
		fd_set readfd;
		memcpy(&readfd, &_fd, sizeof(fd_set));
		if (select(_socket + 1, &readfd, NULL, NULL, &timeout) > 0)
			return recvfrom(_socket, data, size, 0, NULL, NULL);

		return 0;
	}

	virtual Buffer * alloc(const Address & dst, const Protocol & prot, unsigned int once, unsigned int always, unsigned int payload)
	{
		db<UDPNIC>(TRC) << "UDPNIC::alloc(s=" << address() << ",d=" << dst << ",p=" << hex << prot << dec << ",on=" << once << ",al=" << always << ",ld=" << payload << ")" << endl;

		Buffer* buf = new Buffer(this, 0);
		buf->size(once + always + payload);
		
		// TCB - do we need it?
		// MAC::marshal(buf);

		return buf;
	}
	virtual int send(Buffer * buf)
	{
		db<UDPNIC>(TRC) << "UDPNIC::send(buf=" << buf << ",frame=" << buf->frame() << " => " << *(buf->frame()) << endl;
		return send(address(), NULL, buf->link(), buf->size());
	}

	virtual void free(Buffer * buf)
	{
		db<UDPNIC>(TRC) << "UDPNIC::free(buf=" << buf << ")" << endl;
		delete buf;
	}

	virtual const Address& address()
	{
		db<UDPNIC>(TRC) << "UDPNIC::address()" << endl;
		return *(new Address());
	}
	
	virtual void address(const Address& addr)
	{
		db<UDPNIC>(TRC) << "UDPNIC::address(addr=" << addr << ")" << endl;
	}

	virtual bool reconfigure(const Configuration * c = 0)
	{ 
		db<UDPNIC>(TRC) << "UDPNIC::reconfigure(c=" << c << ")" << endl;
		return true;
	}

	virtual const Configuration & configuration()
	{
		db<UDPNIC>(TRC) << "UDPNIC::configuration()" << endl;
		return *(new Configuration());
	}

	virtual const Statistics& statistics()
	{
		db<UDPNIC>(TRC) << "UDPNIC::statistics()" << endl;
		return *(new Statistics());
	}

#undef NULL
};