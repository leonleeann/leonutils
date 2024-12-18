#include <filesystem>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "leonutils/FileSystem.hpp"

namespace fs = std::filesystem;
using namespace leon_utl;
using namespace std;

TEST( TestMiscTools, makeTmpDir ) {
//	path_t old_dir = fs::current_path();

	path_t tmp_dir = make_dir();
	ASSERT_TRUE( exists( tmp_dir ) );

//	path_t cur_dir = fs::current_path();
//	ASSERT_STREQ( tmp_dir.c_str(), cur_dir.c_str() );
//	fs::current_path( old_dir );
};

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
