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
using RQ3B_t = ShmAtmRQ_t<Rec3B_t, int32_t>;
using RQInt_t = ShmAtmRQ_t<int, int32_t>;

// 1.看看每个 Node_t 是否都从64字节边界开始; 2.每个 Node_t 大小
TEST( TestShmAtmRQ, AlignTo64 ) {
	RQ3B_t rq;
	rq.make( 8, "TestShmAtmRQ_AlignTo64" );
	rq.enque( Rec3B_t{} );
	rq.enque( Rec3B_t{} );
//	static_assert( sizeof( RQ3B_t::Node_t ) == 64 );

	ASSERT_EQ( reinterpret_cast<uint64_t>( rq._head_addr() ) % 64, 0 );
	ASSERT_EQ( reinterpret_cast<uint64_t>( rq._tail_addr() ) % 64, 0 );
};

// 如果放弃了所有权, 销毁对象时, 不应该删除底层 shm 文件
TEST( TestShmAtmRQ, Ownership ) {
	path_t shm_path { "/dev/shm" };
	shm_path /= "TestShmAtmRQ";
	shm_unlink( "TestShmAtmRQ" );
	ASSERT_FALSE( fs::exists( shm_path ) );

	{
		RQInt_t rq;
		rq.make( 8, "TestShmAtmRQ" );
		ASSERT_EQ( rq.osFile(), shm_path );
		ASSERT_TRUE( fs::exists( shm_path ) );
		rq.releaseOwnership();	// 放弃所有权
	}
	ASSERT_TRUE( fs::exists( shm_path ) );

	{
		RQInt_t rq;
		rq.make( 8, "TestShmAtmRQ" );
		ASSERT_TRUE( fs::exists( shm_path ) );
//		rq.releaseOwnership(); 未放弃所有权, 对象销毁时会删除底层文件
	}
	ASSERT_FALSE( fs::exists( shm_path ) );
};

// 使用者可以提前删除底层 shm 文件
TEST( TestShmAtmRQ, removeOsShmFile ) {
	path_t shm_path { "/dev/shm" };
	shm_path /= "TestShmAtmRQ";
	shm_unlink( "TestShmAtmRQ" );
	ASSERT_FALSE( fs::exists( shm_path ) );

	RQInt_t rq;
	rq.make( 8, "TestShmAtmRQ" );
	ASSERT_EQ( rq.osFile(), shm_path );
	ASSERT_TRUE( fs::exists( shm_path ) );

	// 调用删除
	rq.delOsFile();

	ASSERT_FALSE( fs::exists( shm_path ) );
};

// 自动扩展容量到2的n次方
TEST( TestShmAtmRQ, AutoExtToPowerOf2 ) {

	ASSERT_EQ( sizeof( Rec3B_t ),  3 );

	RQ3B_t rq;
	rq.make( 5, "TestShmAtmRQ" );
	EXPECT_EQ( rq.capa(), 8 );

	RQ3B_t rq1;
	rq1.make( 17, "TestShmAtmRQ" );
	EXPECT_EQ( rq1.capa(), 32 );

	/* 注释原因: 并非本测试不该有,只因分配一块大内存实在太慢,影响日常检查.需测试时可临时恢复
	size_t max_capa = RQ3B_t::MAX_MEM_USAGE / sizeof( Rec3B_t );
	max_capa = std::pow( 2, std::floor( std::log2( static_cast<double>( max_capa ) ) ) );
	RQ3B_t rq2( max_capa );
	ASSERT_EQ( rq2.capa(), max_capa );
	*/
};

TEST( TestShmAtmRQ, emptyOrFully ) {
	RQInt_t rq;
	rq.make( 4, "TestShmAtmRQ" );

	EXPECT_TRUE( rq.empty() );
	EXPECT_FALSE( rq.full() );
	EXPECT_EQ( rq.size(), 0 );

	// 添加 1 个元素不会满
	rq.enque( 111 );
	EXPECT_FALSE( rq.empty() );
	EXPECT_FALSE( rq.full() );
	EXPECT_EQ( rq.size(), 1 );

	// 添加 2 个元素不会满
	rq.enque( 222 );
	EXPECT_FALSE( rq.empty() );
	EXPECT_FALSE( rq.full() );
	EXPECT_EQ( rq.size(), 2 );

	// 添加 3 个元素不会满
	rq.enque( 333 );
	EXPECT_FALSE( rq.empty() );
	EXPECT_FALSE( rq.full() );
	EXPECT_EQ( rq.size(), 3 );

	// 添加 4 个元素就满了
	rq.enque( 444 );
	EXPECT_FALSE( rq.empty() );
	EXPECT_TRUE( rq.full() );
	EXPECT_EQ( rq.size(), 4 );
};

// 满了之后出队数据还能恢复
TEST( TestShmAtmRQ, CanRecoverFromFull ) {
	RQInt_t rq;
	rq.make( 4, "TestShmAtmRQ" );
	rq.enque( 111 );
	rq.enque( 222 );
	rq.enque( 333 );
	rq.enque( 444 );
	EXPECT_TRUE( rq.full() );
	EXPECT_EQ( rq.size(), 4 );
	EXPECT_FALSE( rq.enque( 555 ) );
	EXPECT_EQ( rq.err_cnt(), 1 );

	int tmp;
	rq.deque( tmp );
	EXPECT_EQ( rq.err_cnt(), 0 );
	EXPECT_FALSE( rq.empty() );
	EXPECT_FALSE( rq.full() );
	EXPECT_EQ( rq.size(), 3 );

	EXPECT_TRUE( rq.enque( 666 ) );
	EXPECT_EQ( rq.err_cnt(), 0 );
};

// 出入队数据核对
TEST( TestShmAtmRQ, DequeDataIsWhatEnqued ) {
	RQInt_t rq;
	rq.make( 4, "TestShmAtmRQ" );

	rq.enque( 1 );
	rq.enque( 2 );
	rq.enque( 3 );
	rq.enque( 4 );

	int tmp;
	EXPECT_EQ( rq.head(), 0 );
	rq.deque( tmp );
	EXPECT_EQ( rq.head(), 1 );
	rq.deque( tmp );
	EXPECT_EQ( rq.head(), 2 );
	rq.deque( tmp );
	EXPECT_EQ( rq.head(), 3 );
	rq.deque( tmp );
	ASSERT_TRUE( rq.empty() );
};

// 记录一个对象是移动构造还是复制构造的
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

TEST( TestShmAtmRQ, copyIn ) {
	RQWatches_t rq; rq.make( 4, "TestShmAtmRQ" );
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

TEST( TestShmAtmRQ, moveIn ) {
	LiveWatch_t::clearCounts();
	RQWatches_t rq; rq.make( 4, "TestShmAtmRQ" );
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

TEST( TestShmAtmRQ, moveOut ) {
	LiveWatch_t::clearCounts();
	RQWatches_t rq; rq.make( 4, "TestShmAtmRQ" );
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

};	// namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
