#include <cmath>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <iostream>
#include <iomanip>

#include "RingQueue.hpp"

using namespace leon_utl;

struct Record3Bytes {
	// unsigned char pch3[3];
	bool b0;
	bool b1;
	bool b2;
};

// 自动扩展容量到2的n次方
TEST( TestRingQue, AutoExtToPowerOf2 ) {

	ASSERT_EQ( sizeof( Record3Bytes ),  3 );

	RingQueue_t<Record3Bytes> rq( 5 );
	EXPECT_EQ( rq.capacity(), 8 );

	RingQueue_t<Record3Bytes> rq1( 17 );
	EXPECT_EQ( rq1.capacity(), 32 );

	size_t max_size_in_byte = 2147483648;
	size_t max_item_count =
		std::pow(
			2,
			std::floor(
				std::log2( double( max_size_in_byte / sizeof( Record3Bytes ) ) )
			)
		);
	RingQueue_t<Record3Bytes> rq2( max_size_in_byte + 1 );
	EXPECT_EQ( rq2.capacity(), max_item_count );
};

// 占用内存不超2G
TEST( TestRingQue, NeverUseMemoryMoreThan2G ) {
	size_t iElements = 2 * 1024ul * 1024ul * 1024ul / sizeof( int );

	RingQueue_t<int> rq0( iElements + 1 );

	EXPECT_EQ( rq0.capacity(), iElements );

	RingQueue_t<int> rq1( iElements );

	EXPECT_EQ( rq1.capacity(), iElements );
};

// 创建之后应该是空的
TEST( TestRingQue, ShouldBeEmptyOnCreated ) {
	RingQueue_t<int> rq( 8 );

	EXPECT_TRUE( rq.isEmpty() );
	EXPECT_FALSE( rq.hasData() );
	EXPECT_FALSE( rq.isFull() );
	EXPECT_EQ( rq.availabeCount(), 0 );
};

// 添加 1 个元素不会满
TEST( TestRingQue, BeNotFullAfter1Enque ) {
	RingQueue_t<int> rq( 4 );

	rq.enque();

	EXPECT_FALSE( rq.isEmpty() );
	EXPECT_TRUE( rq.hasData() );
	EXPECT_FALSE( rq.isFull() );
	EXPECT_EQ( rq.availabeCount(), 1 );
};

// 添加 2 个元素不会满
TEST( TestRingQue, BeNotFullAfter2Enque ) {
	RingQueue_t<int> rq( 4 );

	rq.enque();
	rq.enque();

	EXPECT_FALSE( rq.isEmpty() );
	EXPECT_TRUE( rq.hasData() );
	EXPECT_FALSE( rq.isFull() );
	EXPECT_EQ( rq.availabeCount(), 2 );
};

// 添加 3 个元素不会满
TEST( TestRingQue, BeNotFullAfter3Enque ) {
	RingQueue_t<int> rq( 4 );

	rq.enque();
	rq.enque();
	rq.enque();

	EXPECT_FALSE( rq.isEmpty() );
	EXPECT_TRUE( rq.hasData() );
	EXPECT_FALSE( rq.isFull() );
	EXPECT_EQ( rq.availabeCount(), 3 );
};

// 添加 n 个元素就会满
TEST( TestRingQue, BeFullAfterN ) {
	RingQueue_t<int> rq( 4 );

	rq.enque();
	rq.enque();
	rq.enque();
	rq.enque();

	EXPECT_FALSE( rq.isEmpty() );
	EXPECT_TRUE( rq.hasData() );
	EXPECT_TRUE( rq.isFull() );
	EXPECT_EQ( rq.availabeCount(), 4 );
};

// 满了之后出队数据还能恢复
TEST( TestRingQue, CanRecoverFromFull ) {
	RingQueue_t<int> rq( 4 );

	rq.enque();
	rq.enque();
	rq.enque();
	rq.enque();
	rq.deque();

	EXPECT_FALSE( rq.isEmpty() );
	EXPECT_TRUE( rq.hasData() );
	EXPECT_FALSE( rq.isFull() );
	EXPECT_EQ( rq.availabeCount(), 3 );
};

// 出入队数据核对
TEST( TestRingQue, DequeDataIsWhatEnqued ) {
	RingQueue_t<int> rq( 4 );

	*( rq.tail() ) = 1;
	rq.enque();
	*( rq.tail() ) = 2;
	rq.enque();
	*( rq.tail() ) = 3;
	rq.enque();
	*( rq.tail() ) = 4;
	rq.enque();

	EXPECT_EQ( *( rq.head() ), 1 );
	rq.deque();

	EXPECT_EQ( *( rq.head() ), 2 );
	rq.deque();

	EXPECT_EQ( *( rq.head() ), 3 );
	rq.deque();

	EXPECT_EQ( *( rq.head() ), 4 );
	rq.deque();

	*( rq.tail() ) = 5;
	rq.enque();

	EXPECT_EQ( *( rq.head() ), 5 );
	rq.deque();
};
