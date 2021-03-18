#pragma once
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <machine/nic.h>
#include <utility/debug.h>
#include <pthread.h>
#include <unistd.h>

extern char* globalIPAddress;

#define RX_BUFS 10

class UDPNIC : public NIC<Ethernet>
{
private:
	int _socket = -1;
	fd_set _fd;
	sockaddr_in _remoteAddress;
	Configuration _configuration;
	Statistics _statistics;
	Buffer* _rx_bufs[RX_BUFS];
	unsigned int _rx_cur_consume;
	unsigned int _rx_cur_produce;


public:
	UDPNIC()
	{
		db<UDPNIC>(TRC) << "UDPNIC()" << endl;

		_rx_cur_consume = 0;
		_rx_cur_produce = 0;

		_configuration.timer_accuracy = 1;

		_configuration.unit = 1;

		// Initialize RX buffer pool
		for (unsigned int i = 0; i < RX_BUFS; i++)
			_rx_bufs[i] = new /*(SYSTEM)*/ Buffer(this, 0);

		// Set Address
		// const UUID & id = (unsigned char[]){ 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 }; //  Machine::uuid();
		_configuration.address[0] = 0;
		_configuration.address[1] = 0;
		_configuration.address[2] = 0;
		_configuration.address[3] = 0;
		_configuration.address[4] = 0;
		_configuration.address[5] = 0;
		address(_configuration.address);

		_configuration.timer_accuracy = 1; // timer_accuracy();
		_configuration.timer_frequency = 1000000; //  timer_frequency();

		// Reset
		reconfigure(&_configuration);
		// IEEE802_15_4_NIC::MAC::constructor_epilogue(); // Device is configured, let the MAC use it

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
		_remoteAddress.sin_port = htons(5001);
		_remoteAddress.sin_addr.s_addr = inet_addr(globalIPAddress);

		create_receive_thread();

		usleep(100000);
	}

#ifndef NULL
#define NULL 0
#endif

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
			db<UDPNIC>(TRC) << "sendto size=" << size << ", data=" << data << endl;
			sendto(_socket, data, size, 0, (struct sockaddr *)&_remoteAddress,
				sizeof(_remoteAddress));
			return size;
		}

		return 0;
	}
	virtual int receive(Address * src, Protocol * prot, void * data, unsigned int size)
	{
		db<UDPNIC>(TRC) << "UDPNIC::receive(s=" /*<< *src << ",p=" << hex << *prot << dec*/ << ",d=" << data << ",s=" << size << ") => " << endl;

		Buffer * buf;
		for (buf = 0; !buf; ++_rx_cur_consume %= RX_BUFS) { // _xx_cur_xxx are simple accelerators to avoid scanning the ring buffer from the beginning.
														   // Losing a write in a race condition is assumed to be harmless. The FINC + CAS alternative seems too expensive.
			unsigned int idx = _rx_cur_consume;
			if (_rx_bufs[idx]->lock()) {
				if (_rx_bufs[idx]->size() > 0)
					buf = _rx_bufs[idx];
				else
					_rx_bufs[idx]->unlock();
			}
		}

		Address dst;

		unsigned int ret = 0;

		memcpy(data, buf->frame()->data<Frame>(), (buf->size() < size ? buf->size() : size));

		free(buf);

		return ret;
	}

	virtual Buffer * alloc(const Address & dst, const Protocol & prot, unsigned int once, unsigned int always, unsigned int payload)
	{
		db<UDPNIC>(TRC) << "UDPNIC::alloc(s=" << address() << ",d=" << dst << ",p=" << hex << prot << dec << ",on=" << once << ",al=" << always << ",ld=" << payload << ")" << endl;

		Buffer* buf = new Buffer(this, 0);
		buf->size(once + always + payload);
		
		// TCB - do we need it?
		// MAC::marshal(buf);
		buf->is_microframe = false;
		buf->trusted = false;
		buf->is_new = true;
		buf->random_backoff_exponent = 0;
		buf->microframe_count = 0;
		buf->times_txed = 0;
		// buf->offset = OFFSET_GENERAL_UPPER_BOUND;
		buf->offset = 0;

		return buf;
	}
	virtual int send(Buffer * buf)
	{
		db<UDPNIC>(TRC) << "UDPNIC::send(buf=" << buf << ",frame=" << buf->frame() << " => " << *(buf->frame()) << endl;
		return send(address(), NIC::PROTO_IP, buf->link(), buf->size());
	}

	virtual void free(Buffer * buf)
	{
		db<UDPNIC>(TRC) << "UDPNIC::free(buf=" << buf << ")" << endl;
		delete buf;
	}

	virtual const Address& address()
	{
		db<UDPNIC>(TRC) << "UDPNIC::address()" << endl;
		return _configuration.address;
	}
	
	virtual void address(const Address& addr)
	{
		db<UDPNIC>(TRC) << "UDPNIC::address(addr=" << addr << ")" << endl;
		_configuration.address = addr;
		_configuration.selector = Configuration::ADDRESS;
		reconfigure(&_configuration);
	}

	virtual bool reconfigure(const Configuration * c = 0)
	{ 
		db<UDPNIC>(TRC) << "UDPNIC::reconfigure(c=" << c << ")" << endl;
		return true;
	}

	virtual const Configuration & configuration()
	{
		db<UDPNIC>(TRC) << "UDPNIC::configuration()" << endl;
		return _configuration;
	}

	virtual const Statistics& statistics()
	{
		db<UDPNIC>(TRC) << "UDPNIC::statistics()" << endl;
		_statistics.time_stamp = TSC::time_stamp();
		return _statistics;
	}

	void create_receive_thread()
	{
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_t threadHandle;
		pthread_create(&threadHandle, &attr, receive_thread, this);
		pthread_attr_destroy(&attr);
	}

	static void* receive_thread(void* p)
	{
		db<UDPNIC>(TRC) << "receive_thread()" << endl;

		UDPNIC* udpnic = (UDPNIC*)p;

		int _socket = -1;
		fd_set _fd;

		_socket = socket(AF_INET, SOCK_DGRAM, 0);

		struct sockaddr_in servaddr;
		memset(&servaddr, '\0', sizeof(sockaddr));
		servaddr.sin_family = AF_INET;
		servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
		servaddr.sin_port = htons(5001);
		bind(_socket, (struct sockaddr *)&servaddr, sizeof(sockaddr));

		FD_ZERO(&_fd);
		FD_SET(_socket, &_fd);

		while (true)
		{
			struct timeval timeout;
			timeout.tv_sec = 1;
			timeout.tv_usec = 0;
			fd_set readfd;
			memcpy(&readfd, &_fd, sizeof(fd_set));
			char data[2048];
			int size = 2048;
			if (select(_socket + 1, &readfd, NULL, NULL, &timeout) > 0)
			{
				int ret = recvfrom(_socket, data, size, 0, NULL, NULL);
				if (ret > 0)
					udpnic->receive(0, 0, data, size);
			}
		}
	}


#undef NULL
};