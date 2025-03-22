#pragma once

namespace leon_utl {

/* 类定义
 * RingQue_t 用定长数组实现一个循环队列
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
template <typename T, typename SizeType>
class RingQue_t {
public:
//======== 类(static)接口 =======================================================
	// 最大内存占用
	static constexpr size_t MAX_MEM_USAGE = 0x40000000;  //1GB

	// 至少容纳几个元素
	static constexpr size_t LEAST_ELEMNTS = 4;

	// 最多容纳几个元素(要对齐到2的n次幂,因为要用mask)
#if( __GNUC__ >= 10 )
// gcc-10 之后要用这个
	static constexpr size_t MOST_ELEMENTS =
		std::bit_floor( MAX_MEM_USAGE / sizeof( T ) );
#else
// gcc-9 要用这个
	static constexpr size_t MOST_ELEMENTS =
		std::floor2( MAX_MEM_USAGE / sizeof( T ) );
#endif

//======== 对象接口 =============================================================
	// 只能显式构造
	explicit RingQue_t( SizeType capa_ ) {
		size_t c = capa_;
		// 容量要对齐为2的n次幂,因回绕指针不再用" % _capa", 改用" & _mask"
#if( __GNUC__ >= 10 )
// gcc-10 之后要用这个
		c = std::min( MOST_ELEMENTS, std::bit_ceil( c ) );
#else
// gcc-9 要用这个
		c = std::min( MOST_ELEMENTS, std::ceil2( c ) );
#endif
		if( c > std::numeric_limits<SizeType>::max() )
			throw "期望容量已超过SizeType所能表示的最大值!";

		_buff = std::make_unique<T[]>( c );
		_base = _buff.get();
		_mask = c - 1;
	};

	SizeType	capa() const { return _mask + 1; };
	void		clear() { _head = _tail = 0; };
	bool		empty() const { return _tail <= _head; };
	bool		full() const { return _tail - _head > _mask; };
	SizeType	size() const { return _tail - _head; };

	// 队首元素的引用(下一个出队位置)
	const T&	head() const { return _base[_head & _mask]; };
	// 队尾元素的引用(下一个入队位置)
	T&			tail() const { return _base[_tail & _mask]; };

	// 入队, _tail 增一
	void	enque() { ++_tail; };
	// 复制入队
	// void	enque( const T& pl ) { _base[_tail++ & _mask] = pl; };
	// 复制/移动入队
	template<typename U>
	void	enque( U&& pl_ ) { _base[_tail++ & _mask] = std::forward<U>( pl_ ); };

	// 出队, _head 增一
	T&&		deque() { return std::move( _base[_head++ & _mask] ); };
	void	deque( T& pl_ ) { pl_ = std::move( _base[_head++ & _mask] ); };

	// 作废队尾记录, _tail 减一
	void	rtail() { --_tail; };

//======== 内部实现 =============================================================
private:
	std::unique_ptr<T[]>	_buff {};
	T*			_base {};
	SizeType	_mask {};
	SizeType	_head {};
	SizeType	_tail {};
};

};  //namespace leon_utl
// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
