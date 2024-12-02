#include <gtest/gtest.h>
#include <iomanip>
#include <iostream>
#include <map>

#include "SHMRQ.hpp"

template<typename K, typename V, typename C = std::less<K>,
		 typename A = std::allocator<std::pair<const K, V> > >
using map_t = std::map<K, V, C, A>;
using Ptr2Bool_t = map_t<void*, bool>;

namespace leon_utl {

/* GoogleMock 提供的方式无法验证 SHMRQ_t 是否调用队中元素的构造器/析构器,因为 SHMRQ_t
 * 是原地调用 new/delete. 所以这里自己做的一个简易 mock 类.
 */
struct ElementMock {
	/* 在析构函数内部对自有成员变量的修改代码,会被gcc优化掉(你都要销毁了,就没必要执行了)
	 * 所以不能简单用自有成员变量来记录析构函数是否被调用,只好弄个map来记录 */
	static Ptr2Bool_t Destroyed;
	static Ptr2Bool_t MvCreated;
	static Ptr2Bool_t CpCreated;
	static Ptr2Bool_t MvAssignd;
	static Ptr2Bool_t CpAssignd;

	~ElementMock() {
		Destroyed[this] = true;
		std::cerr << "销毁:" << this << std::endl;
	};

	ElementMock( int64_t pl_ ): payload( pl_ ) {};

	ElementMock() {
		std::cerr << "缺省构造:" << this << std::endl;
	};

	ElementMock( ElementMock&& o ) {
		payload = o.payload;
		MvCreated[this] = true;
		std::cerr << "移动构造:" << this << std::endl;
	};

	ElementMock( const ElementMock& o ) {
		payload = o.payload;
		CpCreated[this] = true;
		std::cerr << "复制构造:" << this << std::endl;
	};

	ElementMock& operator=( ElementMock&& o ) {
		payload = o.payload;
		MvAssignd[this] = true;
		std::cerr << "移动赋值:" << this << std::endl;
		return *this;
	};

	ElementMock& operator=( const ElementMock& o ) {
		payload = o.payload;
		CpAssignd[this] = true;
		std::cerr << "复制赋值:" << this << std::endl;
		return *this;
	};

	void clear() {
		payload = 0;
		Destroyed[this] = false;
		MvCreated[this] = false;
		CpCreated[this] = false;
		MvAssignd[this] = false;
		CpAssignd[this] = false;
	};

	int64_t	payload {};
};

Ptr2Bool_t ElementMock::Destroyed {};
Ptr2Bool_t ElementMock::MvCreated {};
Ptr2Bool_t ElementMock::CpCreated {};
Ptr2Bool_t ElementMock::MvAssignd {};
Ptr2Bool_t ElementMock::CpAssignd {};

using TestSHMRQ_t = SHMRQ_t<ElementMock>;

constexpr int64_t TEST_CAPA { 4 };

struct TestSHMRQ_WITH_ELEMENT_F : public testing::Test {
	TestSHMRQ_t		testee { "TestWithElement", TEST_CAPA, true };
	ElementMock*	elmnt0 = & testee._base[0].data;
	ElementMock*	elmnt1 = & testee._base[1].data;
	ElementMock*	elmnt2 = & testee._base[2].data;
	ElementMock*	elmnt3 = & testee._base[3].data;

	void SetUp() override {
		ASSERT_EQ( testee.capa(), TEST_CAPA );
		ASSERT_EQ( testee.size(), 0 );
		elmnt0->clear();
		elmnt1->clear();
		elmnt2->clear();
		elmnt3->clear();
		std::cout << "elmnt0创建于:" << elmnt0 << std::endl;
		std::cout << "elmnt1创建于:" << elmnt1 << std::endl;
		std::cout << "elmnt2创建于:" << elmnt2 << std::endl;
		std::cout << "elmnt3创建于:" << elmnt3 << std::endl;
	};
};

TEST_F( TestSHMRQ_WITH_ELEMENT_F, elementLifeCycle ) {
	ElementMock tmp { -1 };

//==== 入队 ====================================================================
	// 如果以复制方式入队的话, "复制构造器"应得到调用
	ASSERT_FALSE( ElementMock::CpCreated[elmnt0] );
	ASSERT_TRUE( testee.enque( tmp ) );
	ASSERT_TRUE( ElementMock::CpCreated[elmnt0] );
	ASSERT_EQ( testee.size(), 1 );
	ASSERT_EQ( elmnt0->payload, tmp.payload );

	// 如果以移动方式入队的话, "移动构造器"应得到调用
	tmp.payload = 111;
	ASSERT_FALSE( ElementMock::MvCreated[elmnt1] );
	ASSERT_TRUE( testee.enque( std::move( tmp ) ) );
	ASSERT_TRUE( ElementMock::MvCreated[elmnt1] );
	ASSERT_EQ( testee.size(), 2 );
	ASSERT_EQ( elmnt1->payload, 111 );

	ASSERT_FALSE( ElementMock::MvCreated[elmnt2] );
	ASSERT_TRUE( testee.enque( ElementMock { 222 } ) );
	ASSERT_TRUE( ElementMock::MvCreated[elmnt2] );
	ASSERT_EQ( testee.size(), 3 );
	ASSERT_EQ( elmnt2->payload, 222 );

	ASSERT_FALSE( ElementMock::MvCreated[elmnt3] );
	ASSERT_TRUE( testee.enque( ElementMock { 333 } ) );
	ASSERT_TRUE( ElementMock::MvCreated[elmnt3] );
	ASSERT_EQ( testee.size(), 4 );
	ASSERT_EQ( elmnt3->payload, 333 );

	// 队列已满,再入队应该失败
	ASSERT_FALSE( testee.enque( ElementMock { 444 } ) );
	ASSERT_EQ( testee.size(), 4 );

//==== 出队 ====================================================================
	tmp.payload = -2;
	ASSERT_FALSE( ElementMock::Destroyed[elmnt0] );
	ASSERT_TRUE( testee.deque( tmp ) );
	ASSERT_TRUE( ElementMock::Destroyed[elmnt0] );
	ASSERT_EQ( testee.size(), 3 );
	ASSERT_EQ( tmp.payload, -1 );

	ASSERT_FALSE( ElementMock::Destroyed[elmnt1] );
	ASSERT_TRUE( testee.deque( tmp ) );
	ASSERT_TRUE( ElementMock::Destroyed[elmnt1] );
	ASSERT_EQ( testee.size(), 2 );
	ASSERT_EQ( tmp.payload, 111 );

	ASSERT_FALSE( ElementMock::Destroyed[elmnt2] );
	ASSERT_TRUE( testee.deque( tmp ) );
	ASSERT_TRUE( ElementMock::Destroyed[elmnt2] );
	ASSERT_EQ( testee.size(), 1 );
	ASSERT_EQ( tmp.payload, 222 );

	ASSERT_FALSE( ElementMock::Destroyed[elmnt3] );
	ASSERT_TRUE( testee.deque( tmp ) );
	ASSERT_TRUE( ElementMock::Destroyed[elmnt3] );
	ASSERT_EQ( testee.size(), 0 );
	ASSERT_EQ( tmp.payload, 333 );
};

};	// namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
