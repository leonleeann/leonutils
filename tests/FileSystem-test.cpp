#include <filesystem>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "leonutils/FileSystem.hpp"

namespace fs = std::filesystem;
using namespace leon_utl;
using path_t = fs::path;

TEST( TestFileSystem, makeTmpDir0 ) {
	str_t tmp_dir = new_tmp_dir();

	path_t p1 { tmp_dir };
	path_t p2 { ( tmp_dir.back() == '/' )
				? tmp_dir.substr( 0, tmp_dir.size() - 1 )
				: tmp_dir + '/' };
	std::cout << "p1:" << p1 << std::endl;
	std::cout << "p2:" << p2 << std::endl;

	ASSERT_TRUE( fs::exists( p1 ) );
	ASSERT_TRUE( fs::exists( p2 ) );

	del_tmp_dir( tmp_dir );
	ASSERT_FALSE( fs::exists( p1 ) );
	ASSERT_FALSE( fs::exists( p2 ) );
};

TEST( TestFileSystem, makeTmpDirWithSlash ) {
	str_t tmp_dir = new_tmp_dir() + '/';

	path_t p1 { tmp_dir };
	path_t p2 { ( tmp_dir.back() == '/' )
				? tmp_dir.substr( 0, tmp_dir.size() - 1 )
				: tmp_dir + '/' };
	std::cout << "p1:" << p1 << std::endl;
	std::cout << "p2:" << p2 << std::endl;

	ASSERT_TRUE( fs::exists( p1 ) );
	ASSERT_TRUE( fs::exists( p2 ) );

	del_tmp_dir( tmp_dir );
	ASSERT_FALSE( fs::exists( p1 ) );
	ASSERT_FALSE( fs::exists( p2 ) );
};

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
