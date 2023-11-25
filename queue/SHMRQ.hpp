#pragma once
#include <algorithm>	// erase, fill, for_each, max, min, remove, sort, swap
#include <atomic>
#include <bit>
#include <cstring>		// strlen, strncmp, strncpy, memset, memcpy, memmove, strerror
#include <fcntl.h>		// O_RDONLY, S_IRUSR, S_IWUSR
#include <filesystem>
#include <iostream>
#include <string>
#include <sys/mman.h>	// shm_open, PROT_READ, PROT_WRITE, MAP_PRIVATE, MAP_ANON

/* 一个基于共享内存可跨进程使用的无锁环状队列
 * 特点:
 *   1.无锁
 *   2.环状队列,整个生命周期不会追加/释放内存
 *   3.多生产者/多消费者(MPMC)
 *   4.基于共享内存,可多进程使用,也可多线程使用
 *
 * 注意!!!!!!!!!!!!!!!!!!!
 *   1.不论多少个生产者、消费者,本对象的拥有者(owner)只能有一个,它负责创建共享内存
 *   2.跨进程使用时,队列的载荷不能是指针!
 *   3.曾经使用无符号的 size_t 作为容量类型是 STL 的历史错误,所以改用 int64_t
 */

namespace leon_utl {

inline constexpr std::memory_order mo_acq_rel = std::memory_order::acq_rel;
inline constexpr std::memory_order mo_acquire = std::memory_order::acquire;
inline constexpr std::memory_order mo_consume = std::memory_order::consume;
inline constexpr std::memory_order mo_relaxed = std::memory_order::relaxed;
inline constexpr std::memory_order mo_release = std::memory_order::release;
inline constexpr std::memory_order mo_seq_cst = std::memory_order::seq_cst;

static std::string GetValidSHMName( const std::string& org_ ) {
	// 名字的全局唯一性由使用者解决
	std::string qn = std::filesystem::path( org_.c_str() ).stem().c_str();
	qn.erase( std::remove( qn.begin(), qn.end(), ' ' ), qn.end() );
	qn.erase( std::remove( qn.begin(), qn.end(), '!' ), qn.end() );
	qn.erase( std::remove( qn.begin(), qn.end(), '"' ), qn.end() );
	qn.erase( std::remove( qn.begin(), qn.end(), '#' ), qn.end() );
	qn.erase( std::remove( qn.begin(), qn.end(), '$' ), qn.end() );
	qn.erase( std::remove( qn.begin(), qn.end(), '%' ), qn.end() );
	qn.erase( std::remove( qn.begin(), qn.end(), '&' ), qn.end() );
	qn.erase( std::remove( qn.begin(), qn.end(), '(' ), qn.end() );
	qn.erase( std::remove( qn.begin(), qn.end(), ')' ), qn.end() );
	qn.erase( std::remove( qn.begin(), qn.end(), '*' ), qn.end() );
	qn.erase( std::remove( qn.begin(), qn.end(), '+' ), qn.end() );
	qn.erase( std::remove( qn.begin(), qn.end(), ',' ), qn.end() );
	qn.erase( std::remove( qn.begin(), qn.end(), '-' ), qn.end() );
	qn.erase( std::remove( qn.begin(), qn.end(), '.' ), qn.end() );
	qn.erase( std::remove( qn.begin(), qn.end(), '/' ), qn.end() );
	qn.erase( std::remove( qn.begin(), qn.end(), ':' ), qn.end() );
	qn.erase( std::remove( qn.begin(), qn.end(), ';' ), qn.end() );
	qn.erase( std::remove( qn.begin(), qn.end(), '<' ), qn.end() );
	qn.erase( std::remove( qn.begin(), qn.end(), '=' ), qn.end() );
	qn.erase( std::remove( qn.begin(), qn.end(), '>' ), qn.end() );
	qn.erase( std::remove( qn.begin(), qn.end(), '?' ), qn.end() );
	qn.erase( std::remove( qn.begin(), qn.end(), '@' ), qn.end() );
	qn.erase( std::remove( qn.begin(), qn.end(), '[' ), qn.end() );
	qn.erase( std::remove( qn.begin(), qn.end(), '\'' ), qn.end() );
	qn.erase( std::remove( qn.begin(), qn.end(), '\\' ), qn.end() );
	qn.erase( std::remove( qn.begin(), qn.end(), ']' ), qn.end() );
	qn.erase( std::remove( qn.begin(), qn.end(), '^' ), qn.end() );
	qn.erase( std::remove( qn.begin(), qn.end(), '`' ), qn.end() );
	qn.erase( std::remove( qn.begin(), qn.end(), '{' ), qn.end() );
	qn.erase( std::remove( qn.begin(), qn.end(), '|' ), qn.end() );
	qn.erase( std::remove( qn.begin(), qn.end(), '}' ), qn.end() );
	qn.erase( std::remove( qn.begin(), qn.end(), '~' ), qn.end() );
	return '/' + qn;
};

template <typename T>
class SHMRQ_t {
	friend struct TestSHMRQ_WITH_ELEMENT_F;

public:
	struct Node_t {
		alignas( 64 )
		std::atomic_int64_t	head;
		std::atomic_int64_t	tail;
		T					data;
	};

	// 最大内存占用(单位:字节)
	static constexpr int64_t MAX_MEM_USAGE = 0x40000000;  // 1G Bytes

	// 最少容量(单位:元素个数)
	static constexpr int64_t LEAST_ELEMNTS = 4;

	// 由consumer来构造,会创建共享内存,反之producer只会访问
	explicit SHMRQ_t( const std::string& shm_name, int64_t capacity, bool owning );
	~SHMRQ_t();

	// 向OS申请的共享内存名称
	const std::string& name() const { return _shmn; };

	// 队列容量(单位:元素个数)
	int64_t capa() const { return _capa; };

	// 当前占用(单位:元素个数)
	int64_t size() const {
		return _tail.load( mo_relaxed ) - _head.load( mo_acquire );
	};

	// 通用入队(不再区分复制还是移动)
	template<typename U>
	bool enque( U&& data );

	// 移动出队
	bool deque( T& data );

protected:
	// 对齐到64字节(一个CPU cache line的大小),保证以下变量在同一个cache line里面
	alignas( 64 )
	// 基址
	Node_t* const		_base = nullptr;
	// 容量
	const int64_t		_capa = 0;
	// 访问掩码
	const int64_t		_mask = 0;
	// raw buffer
	void*				_rawb = nullptr;
	// 共享内存名称
	const std::string	_shmn;
	// 我是共享内存的创建者/拥有者(owner)
	bool				_ownr;

	// 对齐到64字节(一个CPU cache line的大小),保证_tail,_head不在同一个line里面
	alignas( 64 )
	std::atomic_int64_t	_head = {};
	alignas( 64 )
	std::atomic_int64_t	_tail = {};
};

template <typename T>
inline SHMRQ_t<T>::SHMRQ_t( const std::string& shm_, int64_t capa_, bool own_ )
	: _shmn( GetValidSHMName( shm_ ) ), _ownr( own_ ) {
	/*======== 确定容量 =============================================================
			1.最大内存占用不能超过 MAX_MEM_USAGE 字节
			2.让容量刚好是2的整数次幂,为了保证 _mask 低位必须是全1
			3.容量至少保证 LEAST_ELEMNTS, 避免头尾重叠
			等价算法:
			int64_t MOST_ELEMENTS = std::log2( MAX_MEM_USAGE / sizeof( Node_t ) );
			MOST_ELEMENTS = std::pow( 2, MOST_ELEMENTS );
			capa_ = std::pow( 2, std::ceil( std::log2( capa_ ) ) );
			*/

#if( __GNUC__ >= 10 )
	// gcc-10 及之后要用这个
	constexpr int64_t MOST_ELEMENTS = std::bit_floor( MAX_MEM_USAGE / sizeof( Node_t ) );
	capa_ = std::bit_ceil( static_cast<size_t>( capa_ ) );
#else
	// gcc-9 及之前要用这个
	constexpr int64_t MOST_ELEMENTS = std::floor2( MAX_MEM_USAGE / sizeof( Node_t ) );
	capa_ = std::ceil2( capa_ );
#endif

	capa_ = std::min( capa_, MOST_ELEMENTS );
	capa_ = std::max( capa_, LEAST_ELEMNTS );
	const_cast<int64_t&>( _capa ) = capa_;
	const_cast<int64_t&>( _mask ) = _capa - 1;
	int64_t bytes = _capa * sizeof( Node_t );

//======== 分配内存 =============================================================
	int shm_fd;
	if( _ownr )
		shm_fd = shm_open( _shmn.c_str(), O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR );
	else
		shm_fd = shm_open( _shmn.c_str(), O_RDWR, S_IRUSR | S_IWUSR );
	if( shm_fd < 0 )
		throw std::runtime_error( _shmn + ":shm_open error:" + strerror( errno ) );
	if( ftruncate( shm_fd, bytes ) != 0 )
		throw std::runtime_error( _shmn + ":ftruncate error:" + strerror( errno ) );
	_rawb = mmap( NULL, bytes, PROT_READ | PROT_WRITE, MAP_SHARED_VALIDATE, shm_fd, 0 );
	if( _rawb == MAP_FAILED )
		throw std::runtime_error( _shmn + ":mmap error:" + strerror( errno ) );
	if( close( shm_fd ) != 0 )
		throw std::runtime_error( _shmn + ":close(shm_fd) error:" + strerror( errno ) );

	// 对齐到整64字节边界
	if( ( reinterpret_cast<uintptr_t>( _rawb ) & 63 ) != 0 )
		throw std::runtime_error( _shmn + ":地址未从64字节整倍数开始!" );

//======== 初始化 ===============================================================
	const_cast<Node_t*&>( _base ) = reinterpret_cast<Node_t*>( _rawb );
	for( int64_t i = 0; i < _capa; ++i ) {
		auto& node = _base[i];
		node.head.store( -1, mo_relaxed );
		node.tail.store( i, mo_relaxed );
		// 从OS申请的共享内存都是全零，可以不必初始化 data
	}
	_head.store( 0, mo_relaxed );
	_tail.store( 0, mo_relaxed );
};

template <typename T>
inline SHMRQ_t<T>::~SHMRQ_t() {
	// 如果队内还剩有效数据,逐一调用它们的析构器,确保它们内部的资源释放(比如uniq_ptr)
	int64_t tail = _tail.load();
	for( int64_t j = _head.load(); j != tail; ++j )
		_base[ j & _mask ].data.~T();

	// 析构器里面不能抛异常
	if( munmap( _rawb, _capa * sizeof( Node_t ) ) != 0 )
		std::cerr << _shmn << ":munmap:" << strerror( errno ) << std::endl;
	if( _ownr && shm_unlink( _shmn.c_str() ) != 0 )
		std::cerr << _shmn << ":shm_unlink:" << strerror( errno ) << std::endl;
};

template <typename T>
template <typename U>
inline bool SHMRQ_t<T>::enque( U&& src_ ) {
	int64_t tail = _tail.load( mo_relaxed );
	Node_t* node;
	do {
		node = _base + ( tail & _mask );
		if( node->tail.load( mo_relaxed ) != tail )
			// 队列已满
			return false;
	} while( ! _tail.compare_exchange_weak( tail, tail + 1, mo_relaxed ) );

	// new( & node->data ) T( std::move( src_ ) );
	new( & node->data ) T( std::forward<U>( src_ ) );
	node->head.store( tail, mo_release );
	return true;
};

template <typename T>
bool SHMRQ_t<T>::deque( T& dest_ ) {
	int64_t head = _head.load( mo_relaxed );
	Node_t* node;
	do {
		node = _base + ( head & _mask );
		if( node->head.load( mo_relaxed ) != head )
			// 队列已空或者恰好该元素尚未就绪(碰撞)
			return false;
	} while( ! _head.compare_exchange_weak( head, head + 1, mo_relaxed ) );

	dest_ = std::move( node->data );
	node->data.~T();
	node->tail.store( head + _capa, mo_release );
	return true;
};

};  // namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
