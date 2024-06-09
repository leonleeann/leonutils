#pragma once
#include <bit>			// bit_ceil, bit_floor(floor2 从gcc-10开始已改名为 bit_floor)
#include <cmath>		// abs, ceil, floor, isnan, log, log10, log2, pow, round, sqrt
#include <memory>		// allocator

#include "CraflinRQ.hpp"

namespace leon_utl {

inline constexpr std::memory_order mo_relaxed = std::memory_order::relaxed;
// inline constexpr std::memory_order mo_consume = std::memory_order::consume;
inline constexpr std::memory_order mo_acquire = std::memory_order::acquire;
inline constexpr std::memory_order mo_release = std::memory_order::release;
inline constexpr std::memory_order mo_acq_rel = std::memory_order::acq_rel;
inline constexpr std::memory_order mo_seq_cst = std::memory_order::seq_cst;

template <typename T>
inline CraflinRQ_t<T>::CraflinRQ_t( QueSize_t capa_ ) {
	// 让容量刚好是2的整数次幂, 因为_mask必须是全1
	// 目的是 capa_ = std::pow( 2, std::ceil( std::log2( capa_ ) ) ); 实际用下面的方法
#if( __GNUC__ >= 10 )
// gcc-10 之后要用这个
	capa_ = std::bit_ceil( capa_ );
	constexpr QueSize_t MOST_ELEMENTS = std::bit_floor( MAX_MEM_USAGE / sizeof( Node_t ) );
#else
// gcc-9 要用这个
	capa_ = std::ceil2( capa_ );
	constexpr QueSize_t MOST_ELEMENTS = std::floor2( MAX_MEM_USAGE / sizeof( Node_t ) );
#endif

	capa_ = std::min( capa_, MOST_ELEMENTS );
	capa_ = std::max( capa_, LEAST_ELEMNTS );
	const_cast<QueSize_t&>( _capa ) = capa_;
	const_cast<QueSize_t&>( _mask ) = capa_ - 1;

	// 分配内存
	_buff = _allc.allocate( capa_ );
	// 检查是否对齐到整64字节边界
	if( ( reinterpret_cast<uintptr_t>( _buff ) & 63 ) != 0 )
		throw std::runtime_error( "CraflinRQ_t:地址未从64字节整倍数开始!" );

	const_cast<Node_t*&>( _base ) = _buff;
	for( QueSize_t i = 0; i < _capa; ++i ) {
		_base[i].tail.store( i, mo_relaxed );
		_base[i].head.store( -1, mo_relaxed );
//		_base[i].data = {};
//		_allc.construct_at( & _base[i] );
	}

	_tail.store( 0, mo_relaxed );
	_head.store( 0, mo_relaxed );
};

template <typename T>
inline CraflinRQ_t<T>::~CraflinRQ_t() {
	for( QueSize_t j = _head.load(); j < _tail.load(); ++j )
// 		_allc.destroy_at( & _base[j & _mask] );
		_base[j & _mask].data.~T();

	_allc.deallocate( _buff, _capa );
};

template <typename T>
inline QueSize_t CraflinRQ_t<T>::capa() const {
	return _capa;
};

template <typename T>
inline QueSize_t CraflinRQ_t<T>::size() const {
	QueSize_t head = _head.load( mo_acquire );
	return _tail.load( mo_relaxed ) - head;
};

template <typename T>
template <typename U>
inline bool CraflinRQ_t<T>::enque( U&& src_ ) {
	Node_t* node;
	QueSize_t tail = _tail.load( mo_relaxed );
	do {
		node = & _base[tail & _mask];
		if( node->tail.load( mo_relaxed ) != tail )
			return false;
	} while( ! _tail.compare_exchange_weak( tail, tail + 1, mo_relaxed ) );

//	_allc.construct_at( node, std::move( src_ ) );
	new( & node->data ) T( std::forward<U>( src_ ) );
	node->head.store( tail, mo_release );
	return true;
};

template <typename T>
inline bool CraflinRQ_t<T>::deque( T& dest_ ) {
	Node_t* node;
	QueSize_t head = _head.load( mo_relaxed );
	do {
		node = & _base[head & _mask];
		if( node->head.load( mo_relaxed ) != head )
			return false;
	} while( ! _head.compare_exchange_weak( head, head + 1, mo_relaxed ) );

	dest_ = std::move( node->data );
// 	_allc.destroy_at( node );
	node->data.~T();
	node->tail.store( head + _capa, mo_release );
	return true;
};

}; // namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
