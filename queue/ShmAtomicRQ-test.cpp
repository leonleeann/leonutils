#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <iostream>
#include <iomanip>

#include "ShmAtomicRQ.hpp"

namespace octopus {

// 自动扩展容量到2的n次方
TEST( TestShmAtmRQ, AutoExtToPowerOf2 ) {

	ASSERT_EQ( sizeof( Rec3B_t ),  3 );

	RQ3B_t rq( 5 );
	EXPECT_EQ( rq.capa(), 8 );

	RQ3B_t rq1( 17 );
	EXPECT_EQ( rq1.capa(), 32 );

	/* 注释原因: 并非本测试不该有,只因分配一块大内存实在太慢,影响日常检查.需测试时可临时恢复
	size_t max_capa = RQ3B_t::MAX_MEM_USAGE / sizeof( Rec3B_t );
	max_capa = std::pow( 2, std::floor( std::log2( static_cast<double>( max_capa ) ) ) );
	RQ3B_t rq2( max_capa );
	ASSERT_EQ( rq2.capa(), max_capa );
	*/
};

TEST( TestRingQue, emptyOrFully ) {
	RQInt_t rq( 4 );

	EXPECT_TRUE( rq.empty() );
	EXPECT_FALSE( rq.fully() );
	EXPECT_EQ( rq.size(), 0 );

	// 添加 1 个元素不会满
	rq.enque();
	EXPECT_FALSE( rq.empty() );
	EXPECT_FALSE( rq.fully() );
	EXPECT_EQ( rq.size(), 1 );

	// 添加 2 个元素不会满
	rq.enque();
	EXPECT_FALSE( rq.empty() );
	EXPECT_FALSE( rq.fully() );
	EXPECT_EQ( rq.size(), 2 );

	// 添加 3 个元素不会满
	rq.enque();
	EXPECT_FALSE( rq.empty() );
	EXPECT_FALSE( rq.fully() );
	EXPECT_EQ( rq.size(), 3 );

	// 添加 4 个元素就满了
	rq.enque();
	EXPECT_FALSE( rq.empty() );
	EXPECT_TRUE( rq.fully() );
	EXPECT_EQ( rq.size(), 4 );
};

// 满了之后出队数据还能恢复
TEST( TestRingQue, CanRecoverFromFull ) {
	RQInt_t rq( 4 );
	rq.enque();
	rq.enque();
	rq.enque();
	rq.enque();
	EXPECT_TRUE( rq.fully() );
	EXPECT_EQ( rq.size(), 4 );

	rq.deque();
	EXPECT_FALSE( rq.empty() );
	EXPECT_FALSE( rq.fully() );
	EXPECT_EQ( rq.size(), 3 );
};

// 出入队数据核对
TEST( TestRingQue, DequeDataIsWhatEnqued ) {
	RQInt_t rq( 4 );

	rq.enque( 1 );
	rq.enque( 2 );
	rq.enque( 3 );
	rq.enque( 4 );

	EXPECT_EQ( rq.head(), 1 );
	rq.deque();

	EXPECT_EQ( rq.head(), 2 );
	rq.deque();

	EXPECT_EQ( rq.head(), 3 );
	rq.deque();

	EXPECT_EQ( rq.head(), 4 );
	rq.deque();

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
using RQWatches_t = RingQue_t<LiveWatch_t>;

TEST( TestRingQue, copyIn ) {
	RQWatches_t rq( 4 );
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

TEST( TestRingQue, moveIn ) {
	LiveWatch_t::clearCounts();
	RQWatches_t rq( 4 );
	ASSERT_EQ( LiveWatch_t::destroieds, 0 );
	ASSERT_EQ( LiveWatch_t::deft_conss, 4 ); // 队内对象都会被默认构造一次
	ASSERT_EQ( LiveWatch_t::expl_conss, 0 );
	ASSERT_EQ( LiveWatch_t::move_conss, 0 );
	ASSERT_EQ( LiveWatch_t::copy_conss, 0 );
	ASSERT_EQ( LiveWatch_t::move_asgns, 0 );
	ASSERT_EQ( LiveWatch_t::copy_asgns, 0 );
	LiveWatch_t::clearCounts();

	// 这应该用移动赋值
	rq.enque( LiveWatch_t { 1 } ); // 显式构造一次，然后移入
	ASSERT_EQ( rq.size(), 1 );
	ASSERT_EQ( LiveWatch_t::destroieds, 1 ); // 临时对象应该有一次销毁
	ASSERT_EQ( LiveWatch_t::deft_conss, 0 );
	ASSERT_EQ( LiveWatch_t::expl_conss, 1 );
	ASSERT_EQ( LiveWatch_t::move_conss, 0 );
	ASSERT_EQ( LiveWatch_t::copy_conss, 0 );
	ASSERT_EQ( LiveWatch_t::move_asgns, 1 );
	ASSERT_EQ( LiveWatch_t::copy_asgns, 0 );
};

TEST( TestRingQue, moveOut ) {
	LiveWatch_t::clearCounts();
	RQWatches_t rq( 4 );
	LiveWatch_t lw;	// 这里也有一次构造
	ASSERT_EQ( LiveWatch_t::destroieds, 0 );
	ASSERT_EQ( LiveWatch_t::deft_conss, 5 ); // 队内对象都会被默认构造一次
	ASSERT_EQ( LiveWatch_t::expl_conss, 0 );
	ASSERT_EQ( LiveWatch_t::move_conss, 0 );
	ASSERT_EQ( LiveWatch_t::copy_conss, 0 );
	ASSERT_EQ( LiveWatch_t::move_asgns, 0 );
	ASSERT_EQ( LiveWatch_t::copy_asgns, 0 );
	rq.enque( 123 );
	LiveWatch_t::clearCounts();

	// 这应该用移动赋值
	rq.deque( lw ); // 这是移出
	ASSERT_EQ( LiveWatch_t::destroieds, 0 ); // 队内对象依然存在，所以没有销毁
	ASSERT_EQ( LiveWatch_t::deft_conss, 0 );
	ASSERT_EQ( LiveWatch_t::expl_conss, 0 );
	ASSERT_EQ( LiveWatch_t::move_conss, 0 );
	ASSERT_EQ( LiveWatch_t::copy_conss, 0 );
	ASSERT_EQ( LiveWatch_t::move_asgns, 1 );
	ASSERT_EQ( LiveWatch_t::copy_asgns, 0 );
	ASSERT_EQ( rq.size(), 0 );
};

};	// namespace octopus

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
