#pragma once
#include <atomic>
#include <memory>		// allocator
// #include <cstddef>

/* 一个多线程无锁的环状队列
 * 特点:
 *   1.无锁
 *   2.环状队列,整个生命周期不会追加/释放内存
 *   3.支持多线程，多生产者/多消费者(MPMC)
 *   4.只能在同一进程内使用
 */
namespace leon_utl {

using AtmSize_t = std::atomic_size_t;
using QueSize_t = std::size_t;

template <typename T>
class CraflinRQ_t {
	friend struct TestCraflinRQ_F;

public:
	struct Node_t {
		alignas( 64 ) AtmSize_t	head;
		T						data;
		alignas( 64 ) AtmSize_t	tail;
	};

	static constexpr QueSize_t MAX_MEM_USAGE = 0x40000000;  //1GB
	static constexpr QueSize_t LEAST_ELEMNTS = 4;

	CraflinRQ_t() = delete;
	CraflinRQ_t( CraflinRQ_t& ) = delete;
	CraflinRQ_t( CraflinRQ_t&& ) = delete;
	CraflinRQ_t( const CraflinRQ_t& ) = delete;
	CraflinRQ_t( const CraflinRQ_t&& ) = delete;
	CraflinRQ_t& operator=( CraflinRQ_t& ) = delete;
	CraflinRQ_t& operator=( CraflinRQ_t&& ) = delete;
	CraflinRQ_t& operator=( const CraflinRQ_t& ) = delete;
	CraflinRQ_t& operator=( const CraflinRQ_t&& ) = delete;

	explicit CraflinRQ_t( QueSize_t capacity );
	~CraflinRQ_t();

	QueSize_t capa() const;
	QueSize_t size() const;
	template <typename U>
	bool enque( U&& data );
	bool deque( T& data );

protected:
	// 对齐到64字节(一个CPU cache line的大小),保证以下变量在同一个cache line里面
	alignas( 64 )Node_t* const	_base {};
	Node_t*						_buff {};
	const QueSize_t				_capa {};
	const QueSize_t				_mask {};
	std::allocator<Node_t>		_allc {};

	// 对齐到64字节(一个CPU cache line的大小),保证 _tail, _head 不在同一个line里面
	alignas( 64 ) AtmSize_t		_head = {};

	// 对齐到64字节(一个CPU cache line的大小),保证 _tail, _head 不在同一个line里面
	alignas( 64 ) AtmSize_t		_tail = {};
};

}; // namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
