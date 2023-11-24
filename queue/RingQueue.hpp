#pragma once
// #include <algorithm> // max, min, sort, swap
// #include <cmath>     // abs, floor, log, log10, pow, round, sqrt
// #include <memory>
// #include <utility>

namespace leon_utl {

/* 类定义
 * RingQueue_t 用定长数组实现一个循环队列
 * 入队下标(生产者使用)指向队尾，出队下标(消费者使用)指向队首。下标单调增长(不能
 * 倒退)，达到数组边界就回绕;
 * 允许入队下标“追上/超越”出队下标，也就是允许入队操作覆盖尚未出队的数据！
 * 不使用STL的队列容器，是为了避免在运行过程中频繁地分配/释放内存，导致内存碎片化;
 * 也为了避免标准容器"悄悄地"的扩展容量, 导致"某一次入队操作时间剧增"等问题.
 *
 * 注意：
 * 1.本队列内部不考虑线程同步问题, 是否需要由使用者决定/实现
 * 2.发生一次“追上/超越”, 并不只是覆盖一个元素, 而是丢失整个队列!
 * 3.当队列为空时执行出队操作, 或者当队列已满时执行入队操作, 结果都是未定义(尽管
 *    有断言检查, 但那只在调试期有作用)。
 */
template <typename Payload_t>
class RingQueue_t {
//=======   对外接口    =======
public:
	static constexpr size_t MAX_CAPACITY =
		std::pow( 2,
				  std::floor(
					  std::log2( std::pow( 2, 31 ) / sizeof( Payload_t ) ) ) );

	// 只能显式构造
	RingQueue_t() = delete;
	RingQueue_t( RingQueue_t& ) = delete;
	RingQueue_t( RingQueue_t&& ) = delete;
	RingQueue_t( const RingQueue_t& ) = delete;
	RingQueue_t( const RingQueue_t&& ) = delete;
	RingQueue_t& operator=( RingQueue_t& ) = delete;
	RingQueue_t& operator=( RingQueue_t&& ) = delete;
	RingQueue_t& operator=( const RingQueue_t& ) = delete;
	RingQueue_t& operator=( const RingQueue_t&& ) = delete;
	explicit RingQueue_t( const size_t p_uiCapaHint ) {
		// 容量要对齐为2的n次幂,因回绕指针不再用" % m_capa", 改用" & m_mask"
		m_capa = std::pow( 2, std::ceil( std::log2( p_uiCapaHint ) ) );
		// 占用内存不能超过2G
		m_capa = std::min( MAX_CAPACITY, m_capa );

		m_upBuffer = std::make_unique< Payload_t[] >( m_capa );
		m_base = m_upBuffer.get();
		m_mask = m_capa - 1;
	};

	// 容量
	inline size_t capacity() const {
		return m_capa;
	};

	// 是否为空
	inline bool isEmpty() const {
		bool bResult = ( m_tail == m_head );
		asm volatile( "mfence" ::: "memory" );
		return bResult;
	};

	// 是否已满
	inline bool isFull() const {
		bool bResult = ( m_tail - m_head > m_mask );
		asm volatile( "mfence" ::: "memory" );
		return bResult;
	};

	// 抛弃队内元素
	inline void clear() {
		m_head = m_tail = 0;
	};

	// 是否有货, 等价于 !isEmpty()
	inline bool hasData() const {
//       bool bResult = ( m_tail != m_head );
//       asm volatile( "mfence" ::: "memory" );
		return ( m_tail != m_head );
	};

	// 可用(已入队元素)计数
	inline size_t availabeCount() const {
		return ( m_tail - m_head );
	};

	// 返回队尾元素的地址, 下标不动.这个元素应该是一个空白元素(可填入内容)
	inline Payload_t* tail() const {
		return m_base + ( m_tail & m_mask );
	};

	// 入队一个元素, 入队下标增一
	inline void enque() {
//       assert( ! isFull() );
		asm volatile( "mfence" ::: "memory" );
		++m_tail;
	};

	// 返回队首元素的地址, 下标不动.如果队列为空, 后果未定义
	inline const Payload_t* head() const {
//       assert( hasData() );
		return m_base + ( m_head & m_mask );
	};

	// 出队一个元素, 出队下标增一
	inline void deque() {
//       assert( hasData() );
		asm volatile( "mfence" ::: "memory" );
		++m_head;
	};

//=======   内部实现    =======
private:
	// 内部例程

	// 内部变量
// std::array<PayLoad_T, m_uiSize> amBuffer;
// PayLoad_T   amBuffer[ m_uiSize ];
	std::unique_ptr< Payload_t[] >  m_upBuffer;
	Payload_t*                 m_base;
	size_t                     m_capa = 4;
	size_t                     m_mask = 3;

	// 入队下标, 大于等于MaxSize时回绕
	size_t                     m_tail = 0;

	// 出队下标, 大于等于MaxSize时回绕
	size_t                     m_head = 0;
};

};  //namespace leon_utl
