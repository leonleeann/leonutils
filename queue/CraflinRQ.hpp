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
namespace leon_ext {

using std::atomic_size_t;

template <typename T>
class CraflinRQ_t {
	friend struct TestCraflinRQ_F;

public:
	struct Node_t {
		alignas( 64 )T data;
		atomic_size_t  tail;
		atomic_size_t  head;
	};

	static constexpr size_t MAX_MEM_USAGE = 0x40000000;  //1GB
	static constexpr size_t LEAST_ELEMNTS = 4;

	CraflinRQ_t() = delete;
	CraflinRQ_t( CraflinRQ_t& ) = delete;
	CraflinRQ_t( CraflinRQ_t&& ) = delete;
	CraflinRQ_t( const CraflinRQ_t& ) = delete;
	CraflinRQ_t( const CraflinRQ_t&& ) = delete;
	CraflinRQ_t& operator=( CraflinRQ_t& ) = delete;
	CraflinRQ_t& operator=( CraflinRQ_t&& ) = delete;
	CraflinRQ_t& operator=( const CraflinRQ_t& ) = delete;
	CraflinRQ_t& operator=( const CraflinRQ_t&& ) = delete;

	explicit CraflinRQ_t( size_t capacity );
	~CraflinRQ_t();

	size_t capa() const;
	size_t size() const;
	template <typename U>
	bool enque( U&& data );
	bool deque( T& data );

protected:
	// 对齐到64字节(一个CPU cache line的大小),保证以下变量在同一个cache line里面
	alignas( 64 )Node_t* const	_base = nullptr;
	const size_t				_capa = 0;
	const size_t				_mask = 0;
	Node_t*						_buff = nullptr;
	std::allocator<Node_t>		_allc;

	// 对齐到64字节(一个CPU cache line的大小),保证 m_tail, m_head 不在同一个line里面
	alignas( 64 )atomic_size_t _tail = {};
	alignas( 64 )atomic_size_t _head = {};
};

}; // namespace leon_ext

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
