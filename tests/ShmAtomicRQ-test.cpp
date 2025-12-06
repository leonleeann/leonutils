#include <filesystem>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <iomanip>
#include <iostream>
#include <sys/mman.h>	// shm_open, PROT_READ, PROT_WRITE, MAP_PRIVATE, MAP_ANON

#include "leonutils/ShmAtomicRQ.hpp"

namespace leon_utl {

namespace fs = std::filesystem;
using path_t = fs::path;

struct Rec3B_t {
	// unsigned char pch3[3];
	bool b0;
	bool b1;
	bool b2;
};

struct TestShmAtmRQ_F: public testing::Test {
	ShmAtmRQ_t	_testee;

	uintptr_t	headNode() { return _testee._head_node(); };
	uintptr_t	tailNode() { return _testee._head_node(); };

	uintptr_t	payload( uintptr_t node_ ) {
		auto payload = reinterpret_cast<ShmAtmRQ_t::Node_t*>( node_ )->goods;
		return reinterpret_cast<uintptr_t>( payload );
	};
};

// 1.看看每个 Node_t 是否都从64字节边界开始; 2.每个 Node_t 大小都是 128字节
TEST_F( TestShmAtmRQ_F, AlignTo64 ) {
	const char* test_name = "TestShmAtmRQ_F_AlignTo64";
	_testee.make( 8, test_name );
	char buffer[ShmAtmRQ_t::LOAD_SIZE];
	strncpy( buffer, test_name, ShmAtmRQ_t::LOAD_SIZE );
	_testee.enque( buffer );
	memset( buffer, 0, ShmAtmRQ_t::LOAD_SIZE );

	auto head_node = headNode();
	auto tail_node = tailNode();
	auto u64_haddr = reinterpret_cast<uint64_t>( head_node );
	auto u64_taddr = reinterpret_cast<uint64_t>( tail_node );
	ASSERT_EQ( u64_haddr % 64, 0 );
	ASSERT_EQ( u64_taddr % 64, 0 );

	auto head_addr = reinterpret_cast<ShmAtmRQ_t::Node_t*>( head_node );
	auto head_htag = reinterpret_cast<uintptr_t>( & head_addr->h_tag );
	auto head_ttag = reinterpret_cast<uintptr_t>( & head_addr->t_tag );
	ASSERT_EQ( head_htag % 8, 0 );
	ASSERT_EQ( head_ttag % 8, 0 );
	// 确保不在同一个 cache line
	ASSERT_GE( head_ttag, head_htag + 64 );
	ASSERT_STRCASEEQ( head_addr->goods, test_name );
};

// 如果放弃了所有权, 销毁对象时, 不应该删除底层 shm 文件
TEST( TestShmAtmRQ, Ownership ) {
	const char* test_name = "TestShmAtmRQ_Ownership";
	path_t shm_path { "/dev/shm" };
	shm_path /= test_name;
	shm_unlink( test_name );
	ASSERT_FALSE( fs::exists( shm_path ) );

	{
		ShmAtmRQ_t testee;
		testee.make( 8, test_name );
		ASSERT_EQ( testee.osFile(), shm_path );
		ASSERT_TRUE( fs::exists( shm_path ) );
		testee.releaseOwnership();	// 放弃所有权
	}
	ASSERT_TRUE( fs::exists( shm_path ) );

	{
		ShmAtmRQ_t testee;
		testee.make( 8, test_name );
		ASSERT_TRUE( fs::exists( shm_path ) );
//		testee.releaseOwnership(); 未放弃所有权, 对象销毁时会删除底层文件
	}
	ASSERT_FALSE( fs::exists( shm_path ) );
};

// 使用者可以提前删除底层 shm 文件
TEST( TestShmAtmRQ, removeOsShmFile ) {
	const char* test_name = "TestShmAtmRQ_removeOsShmFile";
	path_t shm_path { "/dev/shm" };
	shm_path /= test_name;
	shm_unlink( test_name );
	ASSERT_FALSE( fs::exists( shm_path ) );

	ShmAtmRQ_t testee;
	testee.make( 8, test_name );
	ASSERT_EQ( testee.osFile(), shm_path );
	ASSERT_TRUE( fs::exists( shm_path ) );

	// 调用删除
	testee.delOsFile();
	ASSERT_FALSE( fs::exists( shm_path ) );
};

// 自动扩展容量到2的n次方
TEST( TestShmAtmRQ, AutoExtToPowerOf2 ) {
	const char* test_name = "TestShmAtmRQ_AutoExtToPowerOf2";

	ShmAtmRQ_t testee;
	testee.make( 5, test_name );
	EXPECT_EQ( testee.capa(), 8 );
};

TEST( TestShmAtmRQ, emptyOrFully ) {
	const char* test_name = "ShmAtmRQ_emptyOrFully";
	ShmAtmRQ_t testee;
	testee.make( 4, test_name );

	EXPECT_TRUE( testee.empty() );
	EXPECT_FALSE( testee.full() );
	EXPECT_EQ( testee.size(), 0 );

	// 添加 1 个元素不会满
	testee.enque( test_name );
	EXPECT_FALSE( testee.empty() );
	EXPECT_FALSE( testee.full() );
	EXPECT_EQ( testee.size(), 1 );

	// 添加 2 个元素不会满
	testee.enque( test_name );
	EXPECT_FALSE( testee.empty() );
	EXPECT_FALSE( testee.full() );
	EXPECT_EQ( testee.size(), 2 );

	// 添加 3 个元素不会满
	testee.enque( test_name );
	EXPECT_FALSE( testee.empty() );
	EXPECT_FALSE( testee.full() );
	EXPECT_EQ( testee.size(), 3 );

	// 添加 4 个元素就满了
	testee.enque( test_name );
	EXPECT_FALSE( testee.empty() );
	EXPECT_TRUE( testee.full() );
	EXPECT_EQ( testee.size(), 4 );
};

// 满了之后出队数据还能恢复
TEST( TestShmAtmRQ, CanRecoverFromFull ) {
	const char* test_name = "ShmAtmRQ_CanRecoverFromFull";
	ShmAtmRQ_t testee;
	testee.make( 4, test_name );
	testee.enque( test_name );
	testee.enque( test_name );
	testee.enque( test_name );
	testee.enque( test_name );
	EXPECT_TRUE( testee.full() );
	EXPECT_EQ( testee.size(), 4 );
	EXPECT_FALSE( testee.enque( test_name ) );
	EXPECT_EQ( testee.err_cnt(), 1 );

	char buffer[ShmAtmRQ_t::LOAD_SIZE] {};
	testee.deque( buffer );
	EXPECT_EQ( testee.err_cnt(), 0 );
	EXPECT_FALSE( testee.empty() );
	EXPECT_FALSE( testee.full() );
	EXPECT_EQ( testee.size(), 3 );

	EXPECT_TRUE( testee.enque( test_name ) );
	EXPECT_EQ( testee.err_cnt(), 0 );
};

// 出入队数据核对
TEST( TestShmAtmRQ, DequeDataIsWhatEnqued ) {
	str_t test_name = "TestShmAtmRQ_DequeDataIsWhatEnqued";
	str_t use_case0 = test_name + "_0";
	str_t use_case1 = test_name + "_1";
	str_t use_case2 = test_name + "_2";
	str_t use_case3 = test_name + "_3";
	ShmAtmRQ_t testee;
	testee.make( 4, test_name );
	testee.enque( use_case0.c_str() );
	testee.enque( use_case1.c_str() );
	testee.enque( use_case2.c_str() );
	testee.enque( use_case3.c_str() );

	char buffer[ShmAtmRQ_t::LOAD_SIZE] {};
	testee.deque( buffer ); EXPECT_STREQ( buffer, use_case0.c_str() );
	testee.deque( buffer ); EXPECT_STREQ( buffer, use_case1.c_str() );
	testee.deque( buffer ); EXPECT_STREQ( buffer, use_case2.c_str() );
	testee.deque( buffer ); EXPECT_STREQ( buffer, use_case3.c_str() );
	EXPECT_FALSE( testee.deque( buffer ) );
	ASSERT_TRUE( testee.empty() );
};

/* 记录一个对象是移动构造还是复制构造的
struct LiveWatch_t {
	static int destroieds;	// 总共销毁计数
	static int deft_conss;	// 默认构造计数
	static int expl_conss;	// 显式构造计数
	static int move_conss;	// 移动构造计数
	static int copy_conss;	// 复制构造计数
	static int move_asgns;	// 移动赋值计数
	static int copy_asgns;	// 复制赋值计数
	static void clearCounts() {
		destroieds = 0;
		deft_conss = 0;
		expl_conss = 0;
		move_conss = 0;
		copy_conss = 0;
		move_asgns = 0;
		copy_asgns = 0;
	};

	// 销毁
	~LiveWatch_t() { ++destroieds; };
	// 默认构造
	LiveWatch_t() { ++deft_conss; };
	// 显式构造
	LiveWatch_t( int payload_ ) { payload = payload_; ++expl_conss; };
	// 移动构造
	LiveWatch_t( LiveWatch_t&& fr_ ) { payload = fr_.payload; ++move_conss; };
	// 复制构造
	LiveWatch_t( const LiveWatch_t& fr_ ) { payload = fr_.payload; ++copy_conss; };
	// 移动赋值
	LiveWatch_t& operator=( LiveWatch_t&& fr_ ) { payload = fr_.payload; ++move_asgns; return *this; };
	// 复制赋值
	LiveWatch_t& operator=( const LiveWatch_t& fr_ ) { payload = fr_.payload; ++copy_asgns; return *this; };

	int payload { -1 };
};

int LiveWatch_t::destroieds { 0 };
int LiveWatch_t::deft_conss { 0 };
int LiveWatch_t::expl_conss { 0 };
int LiveWatch_t::move_conss { 0 };
int LiveWatch_t::copy_conss { 0 };
int LiveWatch_t::move_asgns { 0 };
int LiveWatch_t::copy_asgns { 0 };
using RQWatches_t = ShmAtmRQ_t<LiveWatch_t, int32_t>;

TEST_F( TestShmAtmRQ_F, copyIn ) {
	RQWatches_t rq; rq.make( 4, "TestShmAtmRQ_F" );
	// 弄个不能改的对象
	const LiveWatch_t cw( 3 );
	LiveWatch_t::clearCounts();

	// 这应该用复制赋值
	rq.enque( cw );
	ASSERT_EQ( rq.size(), 1 );
	ASSERT_EQ( LiveWatch_t::destroieds, 0 );
	ASSERT_EQ( LiveWatch_t::deft_conss, 0 );
	ASSERT_EQ( LiveWatch_t::expl_conss, 0 );
	ASSERT_EQ( LiveWatch_t::move_conss, 0 );
	ASSERT_EQ( LiveWatch_t::copy_conss, 0 );
	ASSERT_EQ( LiveWatch_t::move_asgns, 0 );
	ASSERT_EQ( LiveWatch_t::copy_asgns, 1 );
};

TEST_F( TestShmAtmRQ_F, moveIn ) {
	LiveWatch_t::clearCounts();
	RQWatches_t rq; rq.make( 4, "TestShmAtmRQ_F" );
	ASSERT_EQ( LiveWatch_t::destroieds, 0 );
	ASSERT_EQ( LiveWatch_t::deft_conss, 0 );
	ASSERT_EQ( LiveWatch_t::expl_conss, 0 );
	ASSERT_EQ( LiveWatch_t::move_conss, 0 );
	ASSERT_EQ( LiveWatch_t::copy_conss, 0 );
	ASSERT_EQ( LiveWatch_t::move_asgns, 0 );
	ASSERT_EQ( LiveWatch_t::copy_asgns, 0 );
	LiveWatch_t::clearCounts();

	// 这应该用拷贝赋值
	rq.enque( LiveWatch_t { 1 } ); // 显式构造一次，然后入队
	ASSERT_EQ( rq.size(), 1 );
	ASSERT_EQ( LiveWatch_t::destroieds, 1 ); // 临时对象应该有一次销毁
	ASSERT_EQ( LiveWatch_t::deft_conss, 0 );
	ASSERT_EQ( LiveWatch_t::expl_conss, 1 );
	ASSERT_EQ( LiveWatch_t::move_conss, 0 );
	ASSERT_EQ( LiveWatch_t::copy_conss, 0 );
	ASSERT_EQ( LiveWatch_t::move_asgns, 0 ); // 与RingQue不同,ShmAtmRQ没有使用移动
	ASSERT_EQ( LiveWatch_t::copy_asgns, 1 );
};

TEST_F( TestShmAtmRQ_F, moveOut ) {
	LiveWatch_t::clearCounts();
	RQWatches_t rq; rq.make( 4, "TestShmAtmRQ_F" );
	LiveWatch_t lw;	// 这里也有一次构造
	ASSERT_EQ( LiveWatch_t::destroieds, 0 );
	ASSERT_EQ( LiveWatch_t::deft_conss, 1 );
	ASSERT_EQ( LiveWatch_t::expl_conss, 0 );
	ASSERT_EQ( LiveWatch_t::move_conss, 0 );
	ASSERT_EQ( LiveWatch_t::copy_conss, 0 );
	ASSERT_EQ( LiveWatch_t::move_asgns, 0 );
	ASSERT_EQ( LiveWatch_t::copy_asgns, 0 );
	rq.enque( 123 );
	LiveWatch_t::clearCounts();

	// 这应该用拷贝赋值
	rq.deque( lw );
	ASSERT_EQ( LiveWatch_t::destroieds, 0 ); // 队内对象依然存在，所以没有销毁
	ASSERT_EQ( LiveWatch_t::deft_conss, 0 );
	ASSERT_EQ( LiveWatch_t::expl_conss, 0 );
	ASSERT_EQ( LiveWatch_t::move_conss, 0 );
	ASSERT_EQ( LiveWatch_t::copy_conss, 0 );
	ASSERT_EQ( LiveWatch_t::move_asgns, 0 ); // 与RingQue不同,ShmAtmRQ没有使用移动
	ASSERT_EQ( LiveWatch_t::copy_asgns, 1 );
	ASSERT_EQ( rq.size(), 0 );
};
*/

};	// namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
