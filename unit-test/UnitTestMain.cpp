#include <gmock/gmock.h>
#include <gtest/gtest.h>

/*
class GlobalEnv : public testing::Environment {
public:
	virtual ~GlobalEnv() override {};
	void SetUp() override {
		octopus::unit_test::initializeUCQT();
	};
	void TearDown() override {};
};
*/

GTEST_API_ int main( int argc, char** argv ) {
	testing::InitGoogleTest( &argc, argv );
//	testing::AddGlobalTestEnvironment( new GlobalEnv );
	auto res = RUN_ALL_TESTS();
	return res;
};

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
