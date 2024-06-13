#pragma once
#include <cmath>		// abs, ceil, floor, isnan, log, log10, pow, round, sqrt

#include "convert/UnionTypes.hpp"

namespace leon_utl {

//========== 特殊数值判定 ========================================================

//template<typename T> inline bool isInf( const T ) { return false; };
//template<> inline bool isInf( const double v ) { return std::isinf( v ); };
//template<typename T> inline bool isNaN( const T& v ) { T t1 = v; return !( v == t1 ); };

// 二进制严格等于std::numeric_limits<T>::max()
template<typename T> inline bool strict_max( const T v_ ) {
	return v_ == std::numeric_limits<T>::max();
};
template<> inline bool strict_max( const float	v_ ) { return U32_u( v_ ) == F32_MAXIMU; };
template<> inline bool strict_max( const double	v_ ) { return U64_u( v_ ) == D64_MAXIMU; };

template<typename T> inline bool strict_low( const T v_ ) {
	return v_ == std::numeric_limits<T>::lowest();
};
template<> inline bool strict_low( const float	v_ ) { return U32_u( v_ ) == F32_LOWEST; };
template<> inline bool strict_low( const double	v_ ) { return U64_u( v_ ) == D64_LOWEST; };

// 浮点数存了一个整数
template<typename T>
inline bool about_int( T v ) {
	if( std::numeric_limits<T>::is_integer )
		return true;

	double rounded = std::round( v );
	double epsilon = std::max( std::abs( rounded * 0.0000000001 ), 0.0000000001 );
	return std::abs( rounded - v ) < epsilon;
};

//========== 各种转换 ===========================================================

//-------- 符号函数 --------
template <typename T>
inline constexpr int sign( T x, std::false_type ) {
	return T( 0 ) < x;
};
template <typename T>
inline constexpr int sign( T x, std::true_type ) {
	return int( T( 0 ) < x ) - int( x < T( 0 ) );
}
template <typename T>
inline constexpr int sign( T x ) {
	return sign( x, std::is_signed<T>() );
};

template<typename T>
inline int round2i32( T v ) {
	return std::floor( v + static_cast<T>( 0.5 ) );
};

template<typename T>
inline uint round2u32( T v ) {
	return std::floor( v + static_cast<T>( 0.5 ) );
};

template<typename T>
inline int64_t round2i64( T v ) {
	return std::floor( v + static_cast<T>( 0.5 ) );
};

template<typename T>
inline uint64_t round2u64( T v ) {
	return std::floor( v + static_cast<T>( 0.5 ) );
};

//========== 各种比较 ===========================================================

//-------- 不用if语句的max/min --------
/* 这是一个想当然的玩意儿,实测比std::max慢两倍!!!
template<typename T>
inline T max( T a_, T b_ ) {
	bool use_a = a_ >= b_;
	return use_a * a_ + !use_a * b_;
};
template<typename T>
inline T min( T a_, T b_ ) {
	bool use_a = a_ <= b_;
	return use_a * a_ + !use_a * b_;
};*/

//-------- 同型相等 --------
template<typename T>
inline bool eq( const T v1, const T v2 ) {
	return v1 == v2;
};

template<>
inline bool eq( const float v1, const float v2 ) {
	// 如果二进制逐位相等,那就相等
	if( U32_u( v1 ) == U32_u( v2 ) )
		return true;

	bool lnan = std::isnan( v1 );
	bool rnan = std::isnan( v2 );
	if( lnan && rnan )
		return true;
	if( lnan || rnan )
		return false;

	// 如果异号,那就不相等
	if( std::signbit( v1 ) != std::signbit( v2 ) )
		return false;

	bool linf = std::isinf( v1 );
	bool rinf = std::isinf( v2 );
	// 同号时,都是无穷大,那就相等
	if( linf && rinf )
		return true;
	// 同号时,只有一个无穷大,那就不等
	if( linf || rinf )
		return false;

	return std::abs( v1 - v2 ) <
		   std::max( std::abs( v1 ) * 0.000001, 0.000001 );
};

template<>
inline bool eq( const double v1, const double v2 ) {
	// 如果二进制逐位相等,那就相等
	if( U64_u( v1 ) == U64_u( v2 ) )
		return true;

	bool lnan = std::isnan( v1 );
	bool rnan = std::isnan( v2 );
	if( lnan && rnan )
		return true;
	if( lnan || rnan )
		return false;

	// 如果异号,那就不相等
	if( std::signbit( v1 ) != std::signbit( v2 ) )
		return false;

	bool linf = std::isinf( v1 );
	bool rinf = std::isinf( v2 );
	// 同号时,都是无穷大,那就相等
	if( linf && rinf )
		return true;
	// 同号时,只有一个无穷大,那就不等
	if( linf || rinf )
		return false;

	return std::abs( v1 - v2 ) <
		   std::max( std::abs( v1 ) * 0.0000000001, 0.0000000001 );
};

// 带极小值(epsilon)的比较
template<typename T>
inline bool eq( const T v1, const T v2, const T ) {
	return v1 == v2;
};

template<>
inline bool eq( const float v1, const float v2, const float epsilon ) {
	if( U32_u( v1 ) == U32_u( v2 ) )
		return true;

	bool lnan = std::isnan( v1 );
	bool rnan = std::isnan( v2 );
	if( lnan && rnan )
		return true;
	if( lnan || rnan )
		return false;
	return std::abs( v1 - v2 ) < epsilon;
};

template<>
inline bool eq( const double v1, const double v2, const double epsilon ) {
	if( U64_u( v1 ) == U64_u( v2 ) )
		return true;

	bool lnan = std::isnan( v1 );
	bool rnan = std::isnan( v2 );
	if( lnan && rnan )
		return true;
	if( lnan || rnan )
		return false;
	return std::abs( v1 - v2 ) < epsilon;
};

//-------- 异型相等 --------
template<typename T1, typename T2>
inline bool eq( const T1 v1, const T2 v2 ) {
	return v1 == v2;
};

template<>
inline bool eq( const int32_t i, const uint32_t u ) {
	return ( i >= 0 ) && static_cast<uint32_t>( i ) == u;
};
template<>
inline bool eq( const uint32_t u, const int32_t i ) {
	return eq<int32_t, uint32_t>( i, u );
};

template<>
inline bool eq( const int64_t i, const uint64_t u ) {
	return ( i >= 0 ) && static_cast<uint64_t>( i ) == u;
};
template<>
inline bool eq( const uint64_t u, const int64_t i ) {
	return eq<int64_t, uint64_t>( i, u );
};

template<>
inline bool eq( const double d, const float f ) {
	return eq<float>( static_cast<float>( d ), f );
};
template<>
inline bool eq( const float f, const double d ) {
	return eq<float>( static_cast<float>( d ), f );
};

//-------- 组合比较 --------
template<typename T>
inline bool ge( const T v1, const T v2 ) {
	return v1 > v2 || eq( v1, v2 );
};
template<typename T>
inline bool le( const T v1, const T v2 ) {
	return v1 < v2 || eq( v1, v2 );
};
template<typename T>
inline bool gt( const T v1, const T v2 ) {
	return v1 > v2 && !eq( v1, v2 );
};
template<typename T>
inline bool lt( const T v1, const T v2 ) {
	return v1 < v2 && !eq( v1, v2 );
};
inline bool gt( double v1, double v2, double epsilon ) {
	return v1 > v2 + epsilon;
};
inline bool lt( double v1, double v2, double epsilon ) {
	return v1 < v2 - epsilon;
};

// 自动处理"除数为零"的除法
double safe_div( double num, double deno );

// 找出小数精确位数
int decimals( const double );

};  // namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
