// EPOS Trustful SpaceTime Protocol Declarations

// #include <system/config.h>
#define __tstp__ 1

#if !defined (__tstp_common_h) && defined (__tstp__)
#define __tstp_common_h

//#include <network/ethernet.h>
//#include <network/ieee802_15_4.h>
#define __nic_common_only__
#include <machine/nic.h>
#undef __nic_common_only__
#include <smartdata.h>
//#include <utility/hash.h>
//#include <utility/string.h>
//#include <utility/array.h>
#include <utility/convert.h>

// __BEGIN_SYS

class TSTP: private Traits<TSTP>::NIC_Family, private SmartData, private NIC<Traits<TSTP>::NIC_Family>::Observer
{
    friend class Network;
    friend class Epoch;
    friend class TSTP_PCAP_Sniffer;

private:
    static const bool drop_expired = true;

public:
    // Parts
    typedef Traits<TSTP>::NIC_Family NIC_Family;
    template<typename Engine, bool duty_cycling> class MAC;
    class Router;
    class Locator;
    class Timekeeper;
    class Manager;
    class Security;

    // Imports
    using typename NIC_Family::Buffer;
    using typename NIC_Family::Metadata;
    using typename NIC_Family::Statistics;
    using typename NIC_Family::Configuration;

    typedef Data_Observer<Buffer, Unit> Observer;
    typedef Data_Observed<Buffer, Unit> Observed;

    // Header
    class Header: public SmartData::Header
    {
    public:
        typedef unsigned short Packet_Id;

    public:
        Header() {}

        void identify() {
            _id = 0;
            const char * ptr = reinterpret_cast<const char *>(this);
            for(unsigned int i = 0; i < 10; i += 2)
                _id ^= (ptr[i] << 8) | ptr[i + 1];
            _id &= 0xffff;
        }

    private:
        Packet_Id _id;
    } __attribute__((packed));

    // Packet
    // Each TSTP message is encapsulated in a single package. TSTP does not need nor supports fragmentation.
    class Packet: public Header
    {
    public:
        static const unsigned int MTU = NIC_Family::MTU - sizeof(Header);
        typedef unsigned char Data[MTU];

    public:
        Packet() {}

        Header * header() { return this; }

        template<typename T>
        T * data() { return reinterpret_cast<T *>(&_data); }

        friend Debug & operator<<(Debug & db, const Packet & p);

    private:
        unsigned char _data[MTU];
    } __attribute__((packed));

    // This field is packed first and matches the Frame Type field in the Frame Control in IEEE 802.15.4 MAC.
    // A version number above 4 renders TSTP into the reserved frame type zone and should avoid interference.
    enum Version {
        V0 = 4
    };

protected:
    TSTP(NIC<NIC_Family> * nic);

public:
    ~TSTP();

    static Buffer * alloc(unsigned int size);
    static int send(Buffer * buf);

    // Local Space-Time (network scope, sink at center)
    static Space here();
    static Time now();
    static Time_Stamp time_stamp();

    static Time relative(const Time & t);
    static Space relative(Global_Space global);
    static Space sink() { return Space(0, 0, 0); }

    // Global Space-Time
    static Global_Space absolute(const Space & s);
    static Time absolute(const Time & t);

    // Timer-related service routines
    static const PPM & timer_accuracy() { return _nic->configuration().timer_accuracy;}
    static const Hertz & timer_frequency() { return _nic->configuration().timer_frequency; }
    static Time_Stamp us2ts(const Time & time) { return Convert::us2count<Time, Time_Stamp>(timer_frequency(), time); }
    static Time ts2us(const Time_Stamp & ts) { return Convert::count2us<Hertz, Time_Stamp, Time>(timer_frequency(), ts); }

    // TSTP clients (e.g. SmartData) are unit-based Buffer observers
    static void attach(Data_Observer<Buffer, Unit> * sd, const Unit & unit) { _clients.attach(sd, unit); }
    static void detach(Data_Observer<Buffer, Unit> * sd, const Unit & unit) { _clients.detach(sd, unit); }
    static bool notify(const Unit & unit, Buffer * buf) { return _clients.notify(unit, buf); }

    friend Debug & operator<<(Debug & db, const Packet & p);

private:
    // TSTP components are unconditional Buffer observers
    static void attach(Data_Observer<Buffer> * part) { _parts.attach(part); }
    static void detach(Data_Observer<Buffer> * part) { _parts.detach(part); }
    static bool notify(Buffer * buf) { return _parts.notify(buf); }

    void update(NIC_Family::Observed * obs, const Protocol & prot, Buffer * buf);

    static void marshal(Buffer * buf);

    static void init();

private:
    static Security * _security;
    static Timekeeper * _timekeeper;
    static Locator * _locator;
    static Router * _router;
    static Manager * _manager;

    static NIC<NIC_Family> * _nic;
    static Data_Observed<Buffer> _parts;
    static Data_Observed<Buffer, Unit> _clients;
};

// __END_SYS

#endif

#if !defined (__tstp_h) && !defined (__tstp_common_only__) && defined (__tstp__)
#define __tstp_h

//#include "mac.h"
#include <network/tstp/security.h>
#include <network/tstp/locator.h>
#include <network/tstp/timekeeper.h>
#include <network/tstp/router.h>
#include <network/tstp/manager.h>

// __BEGIN_SYS

inline TSTP::~TSTP() { db<TSTP>(TRC) << "TSTP::~TSTP()" << endl; _nic->detach(this, 0); }
inline TSTP::Buffer * TSTP::alloc(unsigned int size) { return _nic->alloc(Address::BROADCAST, PROTO_TSTP, 0, 0, size); }
inline int TSTP::send(TSTP::Buffer * buf) { db<TSTP>(TRC) << "TSTP::send(buf=" << buf << ")" << endl; marshal(buf); return _nic->send(buf); }

inline TSTP::Space TSTP::here() { return Locator::here(); }
inline TSTP::Space TSTP::relative(TSTP::Global_Space s) { return Locator::relative(s); }
inline TSTP::Global_Space TSTP::absolute(const TSTP::Space & s) { return Locator::absolute(s); }

inline TSTP::Time TSTP::now() { return Timekeeper::now(); }
inline TSTP::Time TSTP::relative(const TSTP::Time & t) { return TSTP::_timekeeper->relative(t); }
inline TSTP::Time TSTP::absolute(const TSTP::Time & t) { return t ? TSTP::_timekeeper->relative(t) : t; }

// __END_SYS

#endif
