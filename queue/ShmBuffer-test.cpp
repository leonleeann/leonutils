#include <filesystem>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/mman.h>	// shm_open, PROT_READ, PROT_WRITE, MAP_PRIVATE, MAP_ANON

#include "ShmBuffer.hpp"

namespace leon_ext {

namespace fs = std::filesystem;
using path_t = fs::path;

static constexpr char	UT_SHM_NAME[]	= "UT_BUFFER";
static constexpr size_t	UT_SHM_SIZE		= 4096;

TEST( TestShmBuffer, createdByWriter ) {
	ShmBuffer_t	rd_buf;
	ShmBuffer_t	wr_buf;

	shm_unlink( UT_SHM_NAME );
	path_t shm_path { "/dev/shm" };
	shm_path /= UT_SHM_NAME;
	ASSERT_FALSE( fs::exists( shm_path ) );

	wr_buf.make( UT_SHM_NAME, UT_SHM_SIZE );
	ASSERT_TRUE( fs::exists( shm_path ) );
	ASSERT_EQ( wr_buf.name(), UT_SHM_NAME );
	ASSERT_EQ( wr_buf.bytes(), UT_SHM_SIZE );

	rd_buf.plug( UT_SHM_NAME, false );
	ASSERT_EQ( rd_buf.name(), UT_SHM_NAME );
	ASSERT_EQ( rd_buf.bytes(), UT_SHM_SIZE );
	// 不是同一次映射,应该不相等
	ASSERT_NE( rd_buf.get(), wr_buf.get() );

	size_t* wr = reinterpret_cast<size_t*>( wr_buf.get() );
	size_t* rd = reinterpret_cast<size_t*>( rd_buf.get() );
	for( size_t j = 0; j < UT_SHM_SIZE / sizeof( size_t ); ++j ) {
		// 初始内存应该是全零
		ASSERT_EQ( wr[j], 0 );
		ASSERT_EQ( rd[j], 0 );

		// 写一边,从另一边读取,核对
		wr[j] = j;
		ASSERT_EQ( rd[j], j );
	}

	rd_buf.unplug( false );
	ASSERT_TRUE( fs::exists( shm_path ) );
	wr_buf.unplug( true );
	ASSERT_FALSE( fs::exists( shm_path ) );
};

TEST( TestShmBuffer, createdByReader ) {
	ShmBuffer_t	rd_buf;
	ShmBuffer_t	wr_buf;

	shm_unlink( UT_SHM_NAME );
	path_t shm_path { "/dev/shm" };
	shm_path /= UT_SHM_NAME;
	ASSERT_FALSE( fs::exists( shm_path ) );

	rd_buf.make( UT_SHM_NAME, UT_SHM_SIZE );
	ASSERT_TRUE( fs::exists( shm_path ) );
	ASSERT_EQ( rd_buf.name(), UT_SHM_NAME );
	ASSERT_EQ( rd_buf.bytes(), UT_SHM_SIZE );

	wr_buf.plug( UT_SHM_NAME, true );
	ASSERT_EQ( wr_buf.name(), UT_SHM_NAME );
	ASSERT_EQ( wr_buf.bytes(), UT_SHM_SIZE );

	size_t* wr = reinterpret_cast<size_t*>( wr_buf.get() );
	size_t* rd = reinterpret_cast<size_t*>( rd_buf.get() );
	for( size_t j = 0; j < UT_SHM_SIZE / sizeof( size_t ); ++j ) {
		// 初始内存应该是全零
		ASSERT_EQ( wr[j], 0 );
		ASSERT_EQ( rd[j], 0 );

		// 写一边,从另一边读取,核对
		wr[j] = j;
		ASSERT_EQ( rd[j], j );
	}

	rd_buf.unplug( false );
	ASSERT_TRUE( fs::exists( shm_path ) );
	wr_buf.unplug( true );
	ASSERT_FALSE( fs::exists( shm_path ) );
};

/* 越界访问内存只是UB,不是必然导致崩溃,所以没法测.
TEST( TestShmBuffer, crashWhenSubscriptBeyond ) {
	ShmBuffer_t	buf;

	shm_unlink( UT_SHM_NAME );
	path shm_path { "/dev/shm" };
	shm_path /= UT_SHM_NAME;
	buf.make( UT_SHM_NAME, UT_SHM_SIZE, true, false );

	size_t* wr = reinterpret_cast<size_t*>( buf.get() );
	size_t err_i = UT_SHM_SIZE / sizeof( size_t );

	// 下标超界是否崩溃?
	ASSERT_DEATH( lg_debg << wr[err_i], "" );

	// 写入一个只读的shm是否崩溃?
	ASSERT_DEATH( lg_debg << ( wr[0] = 1 ), "" );
}; */

}; // namespace leon_ext

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
