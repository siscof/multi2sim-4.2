#include <iostream>

#include <cmath>
#include <stdint.h>

#include "Callback.h"

namespace ClockDomain
{
    typedef DRAMSim::CallbackBase <void> ClockUpdateCB;

	class ClockDomainCrosser
	{
		public:
		ClockUpdateCB *callback;
		uint64_t clock1, clock2;
		uint64_t counter1, counter2;

		ClockDomainCrosser(ClockUpdateCB const &callback);
		ClockDomainCrosser(uint64_t _clock1, uint64_t _clock2, ClockUpdateCB const &callback);
		ClockDomainCrosser(double ratio, ClockUpdateCB const &callback);
		~ClockDomainCrosser();
		void update();
	};

	class TestObj
	{
		public:
		TestObj() {}
		void cb();
		int test();
	};
}
