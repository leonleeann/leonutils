#include <algorithm>	// erase, fill, for_each, max, min, remove, sort, swap
#include <atomic>
#include <bit>
#include <cstring>		// strlen, strncmp, strncpy, memset, memcpy, memmove, strerror
#include <fcntl.h>		// O_RDONLY, S_IRUSR, S_IWUSR
#include <filesystem>
#include <iostream>
#include <sys/mman.h>	// shm_open, PROT_READ, PROT_WRITE, MAP_PRIVATE, MAP_ANON
// #include <unistd.h>	// syscall, ftruncate

#include "SHMRQ.hpp"

using namespace std::filesystem;
using std::atomic_int64_t;
using std::string;

namespace leon_utl {

inline constexpr std::memory_order mo_acq_rel = std::memory_order::acq_rel;
inline constexpr std::memory_order mo_acquire = std::memory_order::acquire;
inline constexpr std::memory_order mo_consume = std::memory_order::consume;
inline constexpr std::memory_order mo_relaxed = std::memory_order::relaxed;
inline constexpr std::memory_order mo_release = std::memory_order::release;
inline constexpr std::memory_order mo_seq_cst = std::memory_order::seq_cst;

static string GetValidSHMName( const string& org_ ) {
	// 名字的全局唯一性由使用者解决
	string qn = path( org_.c_str() ).stem().c_str();
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
struct SHMRQ_t<T>::Imp_t {
	struct Node_t {
		alignas( 64 )
		atomic_int64_t	head;
		atomic_int64_t	tail;
		T				data;
	};

	Imp_t( const string& shm, int64_t capa, bool owning );
	~Imp_t();

	// 占用一个空闲的队尾 Node
	bool occup( Node_t*& node, int64_t& tail );

	// 入队
	// bool enque( const T& src );
	// bool enque( T&& src );
	template <typename U>
	bool enque( U&& src );

	// 出队
	bool deque( T& dest );

	// 对齐到64字节(一个CPU cache line的大小),保证以下变量在同一个cache line里面
	alignas( 64 )
	// 基址
	Node_t* const	_base = nullptr;
	// 容量
	const int64_t	_capa = 0;
	// 访问掩码
	const int64_t	_mask = 0;
	// raw buffer
	void*			_rawb = nullptr;
	// 共享内存名称
	const string	_shmn;
	// 我是共享内存的创建者/拥有者(owner)
	bool			_ownr;

	// 对齐到64字节(一个CPU cache line的大小),保证_tail,_head不在同一个line里面
	alignas( 64 )
	atomic_int64_t	_head = {};
	alignas( 64 )
	atomic_int64_t	_tail = {};
};

template <typename T>
SHMRQ_t<T>::Imp_t::Imp_t( const string& shm_, int64_t capa_, bool owning_ )
	: _shmn( shm_ ), _ownr( owning_ ) {

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
	// gcc-9 要用这个
	constexpr int64_t MOST_ELEMENTS = std::floor2( MAX_MEM_USAGE / sizeof( Node_t ) );
	capa_ = std::ceil2( capa_ );
#endif

	capa_ = std::min( capa_, MOST_ELEMENTS );
	capa_ = std::max( capa_, LEAST_ELEMNTS );
	const_cast<int64_t&>( _capa ) = capa_;
	const_cast<int64_t&>( _mask ) = capa_ - 1;
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
	}
	_head.store( 0, mo_relaxed );
	_tail.store( 0, mo_relaxed );
};

template <typename T>
SHMRQ_t<T>::Imp_t::~Imp_t() {
	// 如果队内还剩有效数据,逐一调用它们的析构器,确保它们内部的资源释放(指针指向的资源)
	int64_t tail = _tail.load();
	for( int64_t j = _head.load(); j != tail; ++j )
// 		( &( _base[ j & _mask ].data ) )->~T();
		_base[ j & _mask ].data.~T();

	// 析构器里面不能抛异常
	if( munmap( _rawb, _capa * sizeof( Node_t ) ) != 0 )
		std::cerr << _shmn << ":munmap:" << strerror( errno ) << std::endl;
	if( _ownr && shm_unlink( _shmn.c_str() ) != 0 )
		std::cerr << _shmn << ":shm_unlink:" << strerror( errno ) << std::endl;
};

template <typename T>
const string& SHMRQ_t<T>::name() const {
	return _imp->_shmn;
};

template <typename T>
bool SHMRQ_t<T>::Imp_t::occup( Node_t*& node_, int64_t& tail_ ) {
	tail_ = _tail.load( mo_relaxed );
	do {
		node_ = & _base[tail_ & _mask];
		if( node_->tail.load( mo_relaxed ) != tail_ )
			return false;
	} while( ! _tail.compare_exchange_weak( tail_, tail_ + 1, mo_relaxed ) );
	return true;
};

/* template <typename T>
bool SHMRQ_t<T>::Imp_t::enque( const T& src_ ) {
	Node_t* node;
	int64_t tail;
	if( ! occup( node, tail ) )
		return false;

	new( & node->data ) T( src_ );
	node->head.store( tail, mo_release );
	return true;
}; */

template <typename T>
template <typename U>
bool SHMRQ_t<T>::Imp_t::enque( U&& src_ ) {
	Node_t* node;
	int64_t tail;
	if( ! occup( node, tail ) )
		return false;

	// new( & node->data ) T( std::move( src_ ) );
	new( & node->data ) T( std::forward<U>( src_ ) );

	node->head.store( tail, mo_release );
	return true;
};

template <typename T>
bool SHMRQ_t<T>::Imp_t::deque( T& dest_ ) {
	Node_t* node;
	int64_t head = _head.load( mo_relaxed );
	do {
		node = & _base[head & _mask];
		if( node->head.load( mo_relaxed ) != head )
			return false;
	} while( ! _head.compare_exchange_weak( head, head + 1, mo_relaxed ) );

	dest_ = std::move( node->data );
	node->data.~T();
	node->tail.store( head + _capa, mo_release );
	return true;
};

template <typename T>
SHMRQ_t<T>::SHMRQ_t( const string& shmn_, int64_t capa_, bool owning_ ) {
	_imp = new Imp_t( GetValidSHMName( shmn_ ), capa_, owning_ );
};

template <typename T>
SHMRQ_t<T>::~SHMRQ_t() {
	delete _imp;
};

template <typename T>
int64_t SHMRQ_t<T>::capa() const {
	return _imp->_capa;
};

template <typename T>
int64_t SHMRQ_t<T>::size() const {
	int64_t head = _imp->_head.load( mo_acquire );
	return _imp->_tail.load( mo_relaxed ) - head;
};

// template <typename T>
// bool SHMRQ_t<T>::enque( const T& src_ ) { return _imp->enque( src_ ); };

template <typename T>
template <typename U>
bool SHMRQ_t<T>::enque( U&& src_ ) {
	return _imp->enque( std::forward<U>( src_ ) );
};

template <typename T>
bool SHMRQ_t<T>::deque( T& dest_ ) {
	return _imp->deque( dest_ );
};

};  // namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
