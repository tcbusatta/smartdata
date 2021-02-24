#pragma once

#include <system/types.h>

class Periodic_Thread
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