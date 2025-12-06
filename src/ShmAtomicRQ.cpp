#include <cstring>

#include "leonutils/ShmAtomicRQ.hpp"

namespace leon_utl {

void ShmAtmRQ_t::make( SIZE_TYPE capa_, const str_t& name_ ) {
	capa_ = _AlignCapa( capa_ );
	size_t bytes = sizeof( Meta_t ) + capa_ * sizeof( Node_t );
	bytes = _buff.make( name_, bytes, true );

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

	/*	lg_debg << '"' << name_		<< "\"SHM Atomic RingQue 已对接:"
				<< "\n控制区[" << static_cast<void*>( _meta )  << "]尺寸:" << sizeof( Meta_t )
				<< "\n数据区[" << static_cast<void*>( _base ) << "]尺寸:" << _capa* sizeof( Node_t )
				<< "\n载荷尺寸:"	<< sizeof( T )	<< ",节点尺寸:"	<< sizeof( Node_t )
				<< ",容量:"		<< _capa		<< ",mask:"		<< _mask
				<< ",总尺寸:"	<< bytes		<< ",\nSHM:\t"	<< _buff.get(); */
};

void ShmAtmRQ_t::plug( const str_t& name_ ) {

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

	/*	lg_debg << '"' << name_		<< "\"SHM Atomic RingQue 已对接:"
				<< "\n控制区[" << static_cast<void*>( _meta )  << "]尺寸:" << sizeof( Meta_t )
				<< "\n数据区[" << static_cast<void*>( _base ) << "]尺寸:" << _capa* sizeof( Node_t )
				<< "\n载荷尺寸:"	<< sizeof( T )	<< ",节点尺寸:"	<< sizeof( Node_t )
				<< ",容量:"		<< _capa		<< ",mask:"		<< _mask
				<< ",总尺寸:"	<< real_bytes	<< ",\nSHM:\t"	<< _buff.get(); */
};

ShmAtmRQ_t::~ShmAtmRQ_t() {
	if( _meta )
		_buff.unplug( _ownr );
};

bool ShmAtmRQ_t::enque( const void* g_ ) {
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
			++_errs;
//			lg_erro << _buff.name() << "入队失败,已抛弃!";
			return false;
		}
	}

	_errs.store( 0, mo_relaxed );
//	cur_nod->goods = g_;
	memcpy( cur_nod->goods, g_, sizeof( Node_t::goods ) );
	cur_nod->h_tag.store( cur_idx, mo_release );
	return true;
};

bool ShmAtmRQ_t::deque( void* g_ ) {
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
			++_errs;
			// lg_erro << _buff.name() << "出队失败!";
			return false;
		}
	}

	_errs.store( 0, mo_relaxed );
//	g_ = cur_nod->goods;
	memcpy( g_, cur_nod->goods, sizeof( Node_t::goods ) );
	cur_nod->t_tag.store( cur_idx + _capa, mo_release );
	return true;
};

void ShmAtmRQ_t::clear() {
	uint8_t buf[ sizeof( Node_t::goods ) ];
	while( deque( buf ) );
};

ShmAtmRQ_t::SIZE_TYPE ShmAtmRQ_t::_AlignCapa( SIZE_TYPE w_ ) {

#if( __GNUC__ >= 10 )
	w_ = std::bit_ceil( static_cast<size_t>( w_ ) );
#else
	w_ = std::ceil2( w_ );
#endif

	return std::max( LEAST_ELEMNTS, std::min( MOST_ELEMENTS, w_ ) );
};

int64_t ShmAtmRQ_t::get_flag() const {
	return _meta->_flag.load( mo_acquire );
};

void ShmAtmRQ_t::set_flag( int64_t f_ ) {
	_meta->_flag.store( f_, mo_release );
};

};	// namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
