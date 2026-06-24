#include <gmock/gmock.h>

#include "leonutils/Rdtscp.hpp"

using namespace leon_utl;

TEST( TestTimeNow, calibrating ) {
	TscClock_t testee;

	testee.calibrate( 100ms );

	auto diff = ( testee.nowFree() - system_clock::now() ).count();
	ASSERT_LT( std::abs( diff ), 1000 );
};

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
