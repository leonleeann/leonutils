#include <atomic>
#include <cmath>        // abs, ceil, floor, isnan, log, log10, pow, round, sqrt
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <iomanip>
#include <iostream>

#include "queue/CraflinRQ.tpp"

using namespace std;

namespace leon_utl {

int imp_created = 0; // 隐式创建计数
int exp_created = 0; // 显式创建计数
int destroied = 0;   // 销毁计数

struct Record3Bytes {
	Record3Bytes() {
		++imp_created;
	};

	Record3Bytes( const Record3Bytes& o ) :
		b0( o.b0 ), b1( o.b1 ), b2( o.b2 ) {
		++exp_created;
	};

	~Record3Bytes() {
		++destroied;
	};

//------------------------------------------
	// unsigned char pch3[3];
	bool b0;
	bool b1;
	bool b2;
};

// 自动扩展容量到2的n次方
TEST( TestCraflinRQ, AutoExtToPowerOf2 ) {

	ASSERT_EQ( sizeof( Record3Bytes ), 3 );
	ASSERT_EQ( sizeof( CraflinRQ_t<Record3Bytes>::Node_t ), 64 * 2 );

	CraflinRQ_t<Record3Bytes> rq0 { 0 };
	ASSERT_EQ( rq0.capa(), 4 );

	CraflinRQ_t<Record3Bytes> rq1 { 5 };
	ASSERT_EQ( rq1.capa(), 8 );

	CraflinRQ_t<Record3Bytes> rq2 { 17 };
	ASSERT_EQ( rq2.capa(), 32 );
};

/* 去掉这个测试不是因为不该这样测,只是因为它确实太慢了! */
// 占用内存不超1G
TEST( TestCraflinRQ, NeverUseMemoryMoreThan1G ) {
	QueSize_t max_capa = pow( 2, log2( CraflinRQ_t<int>::MAX_MEM_USAGE
									   / sizeof( CraflinRQ_t<int>::Node_t ) ) );
	CraflinRQ_t<int> rq0 { max_capa + 1 };
	ASSERT_EQ( rq0.capa(), max_capa );

	CraflinRQ_t<int> rq1 { max_capa };
	ASSERT_EQ( rq1.capa(), max_capa );

	max_capa = pow( 2, log2( CraflinRQ_t<Record3Bytes>::MAX_MEM_USAGE
							 / sizeof( CraflinRQ_t<Record3Bytes>::Node_t ) ) );

	CraflinRQ_t<Record3Bytes> rq2 { max_capa + 1 };
	ASSERT_EQ( rq2.capa(), max_capa );

	CraflinRQ_t<Record3Bytes> rq3 { max_capa };
	ASSERT_EQ( rq3.capa(), max_capa );
};

TEST( TestCraflinRQ, sizeAndPayload ) {
	CraflinRQ_t<int> rq( 4 );

	ASSERT_EQ( rq.size(), 0 );

	ASSERT_TRUE( rq.enque( 111 ) );
	ASSERT_EQ( rq.size(), 1 );

	ASSERT_TRUE( rq.enque( 222 ) );
	ASSERT_EQ( rq.size(), 2 );

	ASSERT_TRUE( rq.enque( 333 ) );
	ASSERT_EQ( rq.size(), 3 );

	ASSERT_TRUE( rq.enque( 444 ) );
	ASSERT_EQ( rq.size(), 4 );

	ASSERT_FALSE( rq.enque( 555 ) );
	ASSERT_EQ( rq.size(), 4 );

// 满了之后出队数据还能恢复
	int i = -1;
	rq.deque( i );
	ASSERT_EQ( i, 111 );
	rq.deque( i );
	ASSERT_EQ( i, 222 );
	rq.deque( i );
	ASSERT_EQ( i, 333 );
	rq.deque( i );
	ASSERT_EQ( i, 444 );
	ASSERT_EQ( rq.size(), 0 );
};

// 记录一个对象是移动构造还是复制构造的
struct ConsWatch_t {
	ConsWatch_t() {
		++deft_conss;	// 这是默认构造的
	};

	ConsWatch_t( int payload_ ) {
		payload = payload_;
		++expl_conss;	// 这是显式构造的
	};

	ConsWatch_t( ConsWatch_t&& fr_ ) {
		payload = fr_.payload;
		++move_conss;	// 这是移动构造的
	};

	ConsWatch_t( const ConsWatch_t& fr_ ) {
		payload = fr_.payload;
		++copy_conss;	// 这是复制构造的
	};

	ConsWatch_t& operator=( ConsWatch_t&& fr_ ) {
		payload = fr_.payload;
		++move_asgns;	// 这是移动赋值
		return *this;
	};

	ConsWatch_t& operator=( const ConsWatch_t& fr_ ) {
		payload = fr_.payload;
		++copy_asgns;	// 这是复制赋值
		return *this;
	};

	int payload { -1 };

	static atomic_int deft_conss;// 默认构造计数
	static atomic_int expl_conss;// 显式构造计数
	static atomic_int move_conss;// 移动构造计数
	static atomic_int copy_conss;// 复制构造计数
	static atomic_int move_asgns;// 移动赋值计数
	static atomic_int copy_asgns;// 复制赋值计数

	static void ClearCounts() {
		deft_conss = 0;
		expl_conss = 0;
		move_conss = 0;
		copy_conss = 0;
		move_asgns = 0;
		copy_asgns = 0;
	};
};

atomic_int ConsWatch_t::deft_conss { 0 };
atomic_int ConsWatch_t::expl_conss { 0 };
atomic_int ConsWatch_t::move_conss { 0 };
atomic_int ConsWatch_t::copy_conss { 0 };
atomic_int ConsWatch_t::move_asgns { 0 };
atomic_int ConsWatch_t::copy_asgns { 0 };

// 确保该新造对象时新造，该移动对象时移动
TEST( TestCraflinRQ, perfectForwarding ) {
	ConsWatch_t::ClearCounts();
	CraflinRQ_t<ConsWatch_t> rq( 4 );
	ASSERT_EQ( ConsWatch_t::deft_conss.load(), 0 );
	ASSERT_EQ( ConsWatch_t::expl_conss.load(), 0 );
	ASSERT_EQ( ConsWatch_t::move_conss.load(), 0 );
	ASSERT_EQ( ConsWatch_t::copy_conss.load(), 0 );
	ASSERT_EQ( ConsWatch_t::move_asgns.load(), 0 );
	ASSERT_EQ( ConsWatch_t::copy_asgns.load(), 0 );

	// 这应该用移动构造
	ASSERT_TRUE( rq.enque( ConsWatch_t { 1 } ) ); // 显式构造一次，然后移入
	ASSERT_EQ( rq.size(), 1 );
	ASSERT_EQ( ConsWatch_t::deft_conss.load(), 0 );
	ASSERT_EQ( ConsWatch_t::expl_conss.load(), 1 );
	ASSERT_EQ( ConsWatch_t::move_conss.load(), 1 );
	ASSERT_EQ( ConsWatch_t::copy_conss.load(), 0 );
	ASSERT_EQ( ConsWatch_t::move_asgns.load(), 0 );
	ASSERT_EQ( ConsWatch_t::copy_asgns.load(), 0 );

	ConsWatch_t cw;	// 这里也有一次构造
	// 这应该用移动赋值
	ASSERT_TRUE( rq.deque( cw ) ); // 这是移出
	ASSERT_EQ( ConsWatch_t::deft_conss.load(), 1 );
	ASSERT_EQ( ConsWatch_t::expl_conss.load(), 1 );
	ASSERT_EQ( ConsWatch_t::move_conss.load(), 1 );
	ASSERT_EQ( ConsWatch_t::copy_conss.load(), 0 );
	ASSERT_EQ( ConsWatch_t::move_asgns.load(), 1 );
	ASSERT_EQ( ConsWatch_t::copy_asgns.load(), 0 );
	ASSERT_EQ( rq.size(), 0 );

	// 这应该用复制构造
	const ConsWatch_t cc( 3 ); // 这里也有一次构造
	ASSERT_TRUE( rq.enque( cc ) );
	ASSERT_EQ( ConsWatch_t::deft_conss.load(), 1 );
	ASSERT_EQ( ConsWatch_t::expl_conss.load(), 2 );
	ASSERT_EQ( ConsWatch_t::move_conss.load(), 1 );
	ASSERT_EQ( ConsWatch_t::copy_conss.load(), 1 );
	ASSERT_EQ( ConsWatch_t::move_asgns.load(), 1 );
	ASSERT_EQ( ConsWatch_t::copy_asgns.load(), 0 );
	ASSERT_EQ( rq.size(), 1 );
};

// 地址对齐核对
struct TestCraflinRQ_F : public testing::Test {
	CraflinRQ_t<Record3Bytes>					_rq { 4 };

	const CraflinRQ_t<Record3Bytes>::Node_t*	_buff = _rq._buff;
	const AtmSize_t*							_head = &_rq._head;
	const AtmSize_t*							_tail = &_rq._tail;

	TestCraflinRQ_F() {
		imp_created = exp_created = destroied = 0;
	};
};

TEST_F( TestCraflinRQ_F, checkingWhetherAddressAligned ) {
	auto node = _buff;
	for( QueSize_t k = 0; k < _rq.capa(); ++k, ++node ) {
		ASSERT_EQ( reinterpret_cast<uintptr_t>( node ) % 64, 0 );
		ASSERT_EQ( reinterpret_cast<uintptr_t>( & node->tail ) % 64, 0 );
		ASSERT_EQ( reinterpret_cast<uintptr_t>( & node->head ) % 64, 0 );
	}

	ASSERT_EQ( reinterpret_cast<uintptr_t>( _buff ) % 64, 0 );
	ASSERT_EQ( reinterpret_cast<uintptr_t>( _tail ) % 64, 0 );
	ASSERT_EQ( reinterpret_cast<uintptr_t>( _head ) % 64, 0 );
};

TEST_F( TestCraflinRQ_F, nodeConstructorWouldBeCalled ) {
	// 节点构造器和析构器应该尚未被调过
	ASSERT_EQ( imp_created, 0 );
	ASSERT_EQ( exp_created, 0 );
	ASSERT_EQ( destroied, 0 );

	Record3Bytes r3, r4;
	ASSERT_EQ( imp_created, 2 );
	r3.b0 = r3.b1 = r3.b2 = false;
	r4.b0 = r4.b1 = r4.b2 = true;

	// 入队一个,构造器应该被调1次
	ASSERT_TRUE( _rq.enque( r3 ) );
	ASSERT_EQ( exp_created, 1 );
	ASSERT_EQ( destroied, 0 );

	// 出队一个,析构器应该被调1次
	ASSERT_FALSE( r3.b0 == r4.b0 && r3.b1 == r4.b1 && r3.b2 == r4.b2 );
	ASSERT_TRUE( _rq.deque( r4 ) );
	ASSERT_TRUE( r3.b0 == r4.b0 && r3.b1 == r4.b1 && r3.b2 == r4.b2 );
	ASSERT_EQ( exp_created, 1 );
	ASSERT_EQ( destroied, 1 );
};

}; // namespace leon_utl
// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
