#pragma once
#include <LeonLog>
#include <atomic>
#include <bit>

#include "ShmBuffer.hpp"
#include "event/MemoryOrder.hpp"

namespace leon_ext {

using namespace leon_log;
using str_t = std::string;
using ai32_t = std::atomic_int32_t;
using ai64_t = std::atomic_int64_t;

// 统一容量(下标)类型,改的时候就改此一处.能用"有符号"就不用"无符号"
// using SIZE_TYPE = int32_t;
// using ATOM_SIZE = ai32_t;

// 统一队列命名规则,比如去掉特殊字符,加个前导"/"
// static str	MqName( const char* original );

/* 一个基于shm共享内存上原子变量,可跨进程使用的无锁循环队列
	特点:
		1.循环队列,整个生命周期不会追加/释放内存
		2.无锁
		3.多生产者/多消费者(MPMC)
		4.基于共享内存,可多进程使用,也可多线程使用
		5.线程安全,一个本对象可为多个线程使用

	注意:
		1.载荷数据类型 T 必须是 POD,因为跨进程. 构造器/析构器也没有意义!
		2.载荷数据类型 T 大小必须固定,因为本对象不能使用变长载荷;
*/
template<typename T, typename SIZE_TYPE>
class ShmAtmRQ_t final {
	static_assert( std::is_standard_layout_v<T> );
	// static_assert( std::is_trivially_copyable_v<T> );

	using ATOM_SIZE = std::atomic<SIZE_TYPE>;

	// 内部结构定义
	struct Node_t {
		alignas( 64 )
		ATOM_SIZE	h_tag;
		ATOM_SIZE	t_tag;
		T			goods;
		// 把本结构强行撑大到64字节,刚好一个 cache line,试试性能
		char _unuse[ 64 - sizeof( h_tag ) - sizeof( t_tag ) - sizeof( goods ) ];
	};

//====== 类接口 =================================================================
public:
	// 最大共享内存占用(单位:字节)
	static constexpr SIZE_TYPE MAX_SHM_USAGE = 0x40000000;	// 1G Bytes

	// 最少容量(单位:元素个数)
	static constexpr SIZE_TYPE LEAST_ELEMNTS = 4;

	// 最多容量(单位:元素个数,必须是2的整数次幂, 因为_mask必须是全1)
#if( __GNUC__ >= 10 )
	static constexpr SIZE_TYPE MOST_ELEMENTS =
		std::bit_floor( MAX_SHM_USAGE / sizeof( Node_t ) );
#else
	static constexpr SIZE_TYPE MOST_ELEMENTS =
		std::floor2( MAX_SHM_USAGE / sizeof( Node_t ) );
#endif

//====== 对象接口 ===============================================================
	ShmAtmRQ_t( ShmAtmRQ_t&& ) = delete;
	ShmAtmRQ_t( const ShmAtmRQ_t& ) = delete;
	ShmAtmRQ_t& operator=( ShmAtmRQ_t&& ) = delete;
	ShmAtmRQ_t& operator=( const ShmAtmRQ_t& ) = delete;
	explicit ShmAtmRQ_t() = default;

	// 作为"拥有者"创建本对象,删除重建底层SHM,并且设定容量、初始化公共部分
	void make( SIZE_TYPE capacity, const str_t& name );

	// 作为"访问者"创建本对象,只是对接底层SHM,并根据公共部分设定自身规格
	void plug( const str_t& name );
	~ShmAtmRQ_t();

	const str_t& name() const { return _buff.name(); };
	SIZE_TYPE	capa() const { return _capa; };

	// 为了方便测试,把这几个属性也暴露出来
	SIZE_TYPE	mask() const { return _mask; };
	SIZE_TYPE	head() const;
	SIZE_TYPE	tail() const;
	SIZE_TYPE	size() const;
	bool		full() const;
	bool		empty() const;

	// 因为跨进程要求 T 必须是POD的,所以这里是"移入"还是"复制"都无所谓了
	bool		enque( const T& );
	bool		deque( T& );
	void		clear();

	// 附赠的flag,便于跨进程简单沟通.本对象行为与此完全无关,只是用浪费的shm空间提供个小玩意
	int64_t		get_flag() const;		// 查看 SHM 上的数值
	void		set_flag( int64_t );	// 将一个数值设置到 SHM,供其它进程查看

//====== 内部实现 ===============================================================
private:
	// 让容量刚好是2的整数次幂, 因为_mask必须是全1
	static SIZE_TYPE AlignCapa( SIZE_TYPE want_capa );

	//---- 成员变量 -------------
	struct	Meta_t;
	Meta_t* const	_meta {};	// 控制区,同样的物理内存,在不同进程有不同地址值
	Node_t* const	_base {};	// 数据区,同样的物理内存,在不同进程有不同地址值
	SIZE_TYPE const	_capa {};	// 和_meta内部属性相同,复制到外面简化编码
	SIZE_TYPE const	_mask {};	// 和_meta内部属性相同,复制到外面简化编码
	ShmBuffer_t		_buff {};
	bool const		_ownr {};	// 为true就是拥有者,否则就是访问者
};

// shm 内存首部的控制区, 主要是队首、 队尾变量, 以及容量
template<typename T, typename SIZE_TYPE>
struct ShmAtmRQ_t<T, SIZE_TYPE>::Meta_t {
	alignas( 64 )
	ATOM_SIZE		_head;	// 消费者修改

	SIZE_TYPE const	_capa;	// 只应由创建者初始化
	SIZE_TYPE const	_mask;	// 只应由创建者初始化

	// 本对象的任何一个使用者都可以设置的标志,其它使用者都能看到,包括跨进程
	ai64_t			_flag;

	alignas( 64 )
	ATOM_SIZE		_tail;	// 生产者修改
};

template<typename T, typename SIZE_TYPE>
void ShmAtmRQ_t<T, SIZE_TYPE>::make( SIZE_TYPE capa_, const str_t& name_ ) {
	capa_ = AlignCapa( capa_ );
	size_t bytes = sizeof( Meta_t ) + capa_ * sizeof( Node_t );
	bytes = _buff.make( name_, bytes );

	/***************************************************************************
		注意: Meta_t 结构要在公共区最开始!!!!!!!!!!!!!!!
		因为 capa_ 参数只传给"拥有者","访问者"是通过控制区的 Meta_t::_capa 来获知容量的.
		而真正的数据区是变长的.如果 Meta_t 结构体放后面, "访问者"在尚未知道容量时, 无法找
		到 Meta_t 结构体的正确地址!!!!!!!!!!
	***************************************************************************/
	auto up = reinterpret_cast<uintptr_t>( _buff.get() );
	if( ( up & 63 ) != 0 )
		throw std::runtime_error( name_ + ":控制区未从64字节整倍数开始!" );
	const_cast<Meta_t*&>( _meta ) = reinterpret_cast<Meta_t*>( up );

	// 然后才是数据区
	up = reinterpret_cast<uintptr_t>( _meta + 1 );
	if( ( up & 63 ) != 0 )
		throw std::runtime_error( name_ + ":数据区未从64字节整倍数开始!" );

	const_cast<Node_t*&>( _base ) = reinterpret_cast<Node_t*>( up );
	const_cast<SIZE_TYPE&>( _meta->_capa ) = capa_;
	const_cast<SIZE_TYPE&>( _meta->_mask ) = capa_ - 1;
	_meta->_head.store( 0, mo_relaxed );
	_meta->_tail.store( 0, mo_relaxed );
	const_cast<SIZE_TYPE&>( _capa ) = _meta->_capa;
	const_cast<SIZE_TYPE&>( _mask ) = _meta->_mask;
	const_cast<bool&>( _ownr ) = true;

	for( SIZE_TYPE i = 0; i < capa_; ++i ) {
		auto& node = _base[i];
		node.h_tag.store( i - capa_, mo_relaxed );
		node.t_tag.store( i, mo_relaxed );
		// lg_debg << name_ << '[' << i << "].h:" << node.h_tag.load()
		//		<< ",.t:" << node.t_tag.load();
	}

	lg_debg << '"' << name_		<< "\"SHM Atomic RingQue 已对接:"
			<< "\n控制区[" << static_cast<void*>( _meta )  << "]尺寸:" << sizeof( Meta_t )
			<< "\n数据区[" << static_cast<void*>( _base ) << "]尺寸:" << _capa* sizeof( Node_t )
			<< "\n载荷尺寸:"	<< sizeof( T )	<< ",节点尺寸:"	<< sizeof( Node_t )
			<< ",容量:"		<< _capa		<< ",mask:"		<< _mask
			<< ",总尺寸:"	<< bytes		<< ",\nSHM:\t"	<< _buff.get();
};

template<typename T, typename SIZE_TYPE>
void ShmAtmRQ_t<T, SIZE_TYPE>::plug( const str_t& name_ ) {

	auto real_bytes = _buff.plug( name_, true );

	auto up = reinterpret_cast<uintptr_t>( _buff.get() );
	if( ( up & 63 ) != 0 )
		throw std::runtime_error( name_ + ":控制区未从64字节整倍数开始!" );
	const_cast<Meta_t*&>( _meta ) = reinterpret_cast<Meta_t*>( up );
	SIZE_TYPE capa = _meta->_capa;

	// 期望总字节数 = 头结构尺寸 + [ 期望容量 * 每记录尺寸 ]
	auto want_bytes = sizeof( Meta_t ) + capa * sizeof( Node_t );
	if( real_bytes != want_bytes )
		throw std::runtime_error( name_ + ":实际尺寸与期望尺寸不符!" );

	up = reinterpret_cast<uintptr_t>( _meta + 1 );
	if( ( up & 63 ) != 0 )
		throw std::runtime_error( name_ + ":数据区未从64字节整倍数开始!" );

	const_cast<Node_t*&>( _base ) = reinterpret_cast<Node_t*>( up );
	const_cast<SIZE_TYPE&>( _capa ) = _meta->_capa;
	const_cast<SIZE_TYPE&>( _mask ) = _meta->_mask;
	const_cast<bool&>( _ownr ) = false;
	/* for( SIZE_TYPE i = 0; i < capa_; ++i ) {
		auto& node = _base[i];
		lg_debg << name_ << '[' << i << "].h:" << node.h_tag.load() << ",.t:" << node.t_tag.load();
	} */

	lg_debg << '"' << name_		<< "\"SHM Atomic RingQue 已对接:"
			<< "\n控制区[" << static_cast<void*>( _meta )  << "]尺寸:" << sizeof( Meta_t )
			<< "\n数据区[" << static_cast<void*>( _base ) << "]尺寸:" << _capa* sizeof( Node_t )
			<< "\n载荷尺寸:"	<< sizeof( T )	<< ",节点尺寸:"	<< sizeof( Node_t )
			<< ",容量:"		<< _capa		<< ",mask:"		<< _mask
			<< ",总尺寸:"	<< real_bytes	<< ",\nSHM:\t"	<< _buff.get();
};

template<typename T, typename SIZE_TYPE>
ShmAtmRQ_t<T, SIZE_TYPE>::~ShmAtmRQ_t() {
	if( _meta )
		_buff.unplug( _ownr );
};

template<typename T, typename SIZE_TYPE>
SIZE_TYPE ShmAtmRQ_t<T, SIZE_TYPE>::head() const {
	return _meta->_head.load( mo_acquire );
};

template<typename T, typename SIZE_TYPE>
SIZE_TYPE ShmAtmRQ_t<T, SIZE_TYPE>::tail() const {
	return _meta->_tail.load( mo_acquire );
};

template<typename T, typename SIZE_TYPE>
SIZE_TYPE ShmAtmRQ_t<T, SIZE_TYPE>::size() const {
	SIZE_TYPE tail = _meta->_tail.load( mo_relaxed );
	SIZE_TYPE head = _meta->_head.load( mo_acquire );
	return tail - head;
};

template<typename T, typename SIZE_TYPE>
bool ShmAtmRQ_t<T, SIZE_TYPE>::full() const {
	SIZE_TYPE tail = _meta->_tail.load( mo_relaxed );
	SIZE_TYPE head = _meta->_head.load( mo_acquire );
	return tail - head >= _capa;
};

template<typename T, typename SIZE_TYPE>
bool ShmAtmRQ_t<T, SIZE_TYPE>::empty() const {
	SIZE_TYPE tail = _meta->_tail.load( mo_relaxed );
	SIZE_TYPE head = _meta->_head.load( mo_acquire );
	return tail == head;
};

template<typename T, typename SIZE_TYPE>
bool ShmAtmRQ_t<T, SIZE_TYPE>::enque( const T& g_ ) {
	ATOM_SIZE&	m_tail = _meta->_tail;
	SIZE_TYPE	cur_idx, old_idx { -1 };
	Node_t*		cur_nod;
	int			try_cnt { 0 };

	for( ;; ) {
		cur_idx = m_tail.load( mo_relaxed );
		cur_nod = _base + ( cur_idx & _mask );

		// 只有该节点的t标志等于当前下标,才说明它是空节点(可用),否则队列可能已满,需要重试
		if( cur_nod->t_tag.load( mo_relaxed ) == cur_idx &&
				// 是否能成功抢占该节点,成功的话就可以填入数据了,失败的话就重试
				m_tail.compare_exchange_weak( cur_idx, cur_idx + 1, mo_relaxed ) )
			break;

		if( cur_idx != old_idx ) {
			old_idx = cur_idx;
			try_cnt = 0;
		}
		if( ++try_cnt > 3 ) {
			lg_erro << _buff.name() << "入队失败,已抛弃!";
			return false;
		}
	}

	cur_nod->goods = g_;
	cur_nod->h_tag.store( cur_idx, mo_release );
	return true;
};

template<typename T, typename SIZE_TYPE>
bool ShmAtmRQ_t<T, SIZE_TYPE>::deque( T& g_ ) {
	ATOM_SIZE&	m_head = _meta->_head;
	SIZE_TYPE	cur_idx;
	Node_t*		cur_nod;
	int			try_cnt { 0 };

	for( ;; ) {
		cur_idx = m_head.load( mo_relaxed );
		cur_nod = _base + ( cur_idx & _mask );

		// 如果该节点的h标志不等于当前下标,说明它没有包含有效载荷,也就是队列空了,没必要继续了.
		if( cur_nod->h_tag.load( mo_relaxed ) != cur_idx )
			return false;

		// 是否能成功抢占该节点,成功的话就可以出队数据了,失败的话就重试
		if( m_head.compare_exchange_weak( cur_idx, cur_idx + 1, mo_relaxed ) )
			break;

		if( ++try_cnt > 3 ) {
			// lg_erro << _buff.name() << "出队失败!";
			return false;
		}
	}

	g_ = cur_nod->goods;
	cur_nod->t_tag.store( cur_idx + _capa, mo_release );
	return true;
};

template<typename T, typename SIZE_TYPE>
void ShmAtmRQ_t<T, SIZE_TYPE>::clear() {
	T g;
	while( deque( g ) );
};

template<typename T, typename SIZE_TYPE>
SIZE_TYPE ShmAtmRQ_t<T, SIZE_TYPE>::AlignCapa( SIZE_TYPE w_ ) {

#if( __GNUC__ >= 10 )
	w_ = std::bit_ceil( static_cast<size_t>( w_ ) );
#else
	w_ = std::ceil2( w_ );
#endif

	return std::max( LEAST_ELEMNTS, std::min( MOST_ELEMENTS, w_ ) );
};

template<typename T, typename SIZE_TYPE>
int64_t ShmAtmRQ_t<T, SIZE_TYPE>::get_flag() const {
	return _meta->_flag.load( mo_acquire );
};

template<typename T, typename SIZE_TYPE>
void ShmAtmRQ_t<T, SIZE_TYPE>::set_flag( int64_t f_ ) {
	_meta->_flag.store( f_, mo_release );
};

};	// namespace leon_ext

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
