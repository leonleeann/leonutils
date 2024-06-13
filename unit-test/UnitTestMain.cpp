#include <gmock/gmock.h>
#include <gtest/gtest.h>

GTEST_API_ int main( int argc, char** argv ) {
	testing::InitGoogleTest( &argc, argv );
	auto res = RUN_ALL_TESTS();
	return res;
};

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
