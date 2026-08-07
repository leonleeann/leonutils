#pragma once
#include <array>
#include <concepts>
#include <iomanip>
#include <leonutils/Algorithms.hpp>
#include <limits>
#include <sstream>
#include <type_traits>	// is_floating_point_v, is_signed_v, is_unsigned_v

using std::array;
using oss_t = std::ostringstream;
using str_t = std::string;	using str_cp = const str_t*; using str_cr = const str_t&;

namespace leon_utl {

template<typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

/* array<integral, N> 可以直接用"=="比较, array<floating_point, N> 呢?
	还是必须用 leon_utl::eq() */

template <Numeric T, size_t N>
bool eq( array<T, N> const& a0_, array<T, N> const& a1_ ) {

	bool equal = true;

	for( size_t i = 0; i < N; ++i )
		equal = equal && eq( a0_[i], a1_[i] );

	return equal;
};

template <Numeric T, size_t N>
bool eq( array<T, N> const& a0_, array<T, N> const& a1_, T eps_ ) {

	bool equal = true;

	for( size_t i = 0; i < N; ++i )
		equal = equal && eq( a0_[i], a1_[i], eps_ );

	return equal;
};

template <Numeric T, size_t N>
bool ne( array<T, N> const& a0_, array<T, N> const& a1_, T eps_ ) {
	return ! eq( a0_, a1_, eps_ );
};

template <Numeric T, size_t N>
str_t array_to_str( array<T, N> const& arr_ ) {

	oss_t oss;

	if constexpr( std::floating_point<T> )
		oss << std::setprecision( std::numeric_limits<double>::max_digits10 );

	oss << '{';
	bool first = true;

	for( auto v : arr_ ) {
		if( !first ) oss << ',';
		oss << v;
		first = false;
	}

	oss << '}';
	return oss.str();
};

};	//namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
