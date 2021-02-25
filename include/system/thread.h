#pragma once

#include <system/types.h>

class Thread
{
public:
	Thread(int (*)()) { }
	static void yield()
	{
		// To be implemented.
	}
};

class Periodic_Thread : public Thread
{
private:
	Microsecond _period;

public:
	Periodic_Thread(const Microsecond&, int(*)(unsigned int, SmartData::Time, Responsive_SmartData<Dummy_Transducer>*),
		unsigned int&, const SmartData::Time&, Responsive_SmartData<Dummy_Transducer>*);
	const Microsecond& period() { return _period; }
	void period(const Microsecond& p) { _period = p; }
	static volatile bool wait_next() {

		// TCB - to be implemented.
		return true;
	}
};

class Alarm
{
public:
	Alarm(const Microsecond & time, Handler * handler, unsigned int times = 1);
};