#pragma once

#include <system/types.h>

class Periodic_Thread
{
private:
	Microsecond _period;

public:
	const Microsecond& period() { return _period; }
	void period(const Microsecond& p) { _period = p; }
	static volatile bool wait_next() {

		// TCB - to be implemented.
		return true;
	}
};