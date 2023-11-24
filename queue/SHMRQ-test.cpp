// #include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <iomanip>
#include <iostream>

#include "SHMRQ.tpp"

using std::string;

namespace leon_utl {

/* GoogleMock 提供的方式无法验证 SHMRQ_t 是否调用队中元素的构造器/析构器,因为 SHMRQ_t
 * 是原地调用 new/delete. 所以这里自己做的一个简易 mock 类.
 */
struct ElementMock {
	~ElementMock() {
		std::cerr << "销毁:" << this << std::endl;
		destroyed = true;
	};

	ElementMock( int64_t pl_ ): payload( pl_ ) {};

	ElementMock() {
		std::cerr << "缺省构造:" << this << std::endl;
		df_creatd = true;
	};

	ElementMock( ElementMock&& o ) {
		payload = o.payload;
		std::cerr << "移动构造:" << this << std::endl;
		mv_creatd = true;
	};

	ElementMock( const ElementMock& o ) {
		payload = o.payload;
		std::cerr << "复制构造:" << this << std::endl;
		cp_creatd = true;
	};

	ElementMock& operator=( ElementMock&& o ) {
		payload = o.payload;
		std::cerr << "移动赋值:" << this << std::endl;
		mv_assgnd = true;
		return *this;
	};

	ElementMock& operator=( const ElementMock& o ) {
		payload = o.payload;
		std::cerr << "复制赋值:" << this << std::endl;
		cp_assgnd = true;
		return *this;
	};

	void clear() {
		payload = 0;
		destroyed = false;
		df_creatd = false;
		mv_creatd = false;
		cp_creatd = false;
		mv_assgnd = false;
		cp_assgnd = false;
	};

	int64_t	payload {};
	// 确实来调用了析构器
	bool	destroyed {};
	bool	df_creatd {};
	bool	mv_creatd {};
	bool	cp_creatd {};
	bool	mv_assgnd {};
	bool	cp_assgnd {};
};

using TestSHMRQ_t = SHMRQ_t<ElementMock>;

constexpr int64_t TEST_CAPA { 4 };

struct TestSHMRQ_WITH_ELEMENT_F : public testing::Test {
	TestSHMRQ_t		testee { "TestWithElement", TEST_CAPA, true };
	ElementMock&	elmnt0 = testee._imp->_base[0].data;
	ElementMock&	elmnt1 = testee._imp->_base[1].data;
	ElementMock&	elmnt2 = testee._imp->_base[2].data;
	ElementMock&	elmnt3 = testee._imp->_base[3].data;

	void SetUp() override {
		ASSERT_EQ( testee.capa(), TEST_CAPA );
		ASSERT_EQ( testee.size(), 0 );
		elmnt0.clear();
		elmnt1.clear();
		elmnt2.clear();
		elmnt3.clear();
	};
};

TEST_F( TestSHMRQ_WITH_ELEMENT_F, elementLifeCycle ) {
	ElementMock tmp { 111 };

//==== 入队 ====================================================================
	// 如果以复制方式入队的话, "复制构造器"应得到调用
	ASSERT_FALSE( elmnt0.cp_creatd );
	ASSERT_TRUE( testee.enque( tmp ) );
	ASSERT_TRUE( elmnt0.cp_creatd );
	ASSERT_EQ( testee.size(), 1 );
	ASSERT_EQ( elmnt0.payload, tmp.payload );

	// 如果以移动方式入队的话, "移动构造器"应得到调用
	ASSERT_FALSE( elmnt1.mv_creatd );
	ASSERT_TRUE( testee.enque( ElementMock { 222 } ) );
	ASSERT_TRUE( elmnt1.mv_creatd );
	ASSERT_EQ( testee.size(), 2 );
	ASSERT_EQ( elmnt1.payload, 222 );

//==== 出队 ====================================================================
	tmp.payload = -1;
	ASSERT_FALSE( elmnt0.destroyed );
	ASSERT_TRUE( testee.deque( tmp ) );
	ASSERT_TRUE( elmnt0.destroyed );
	ASSERT_EQ( testee.size(), 1 );
	ASSERT_EQ( tmp.payload, 111 );

	tmp.payload = -1;
	ASSERT_FALSE( elmnt1.destroyed );
	ASSERT_TRUE( testee.deque( tmp ) );
	ASSERT_TRUE( elmnt1.destroyed );
	ASSERT_EQ( testee.size(), 0 );
	ASSERT_EQ( tmp.payload, 222 );
};

};	// namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
