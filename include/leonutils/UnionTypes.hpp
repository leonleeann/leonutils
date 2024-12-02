#pragma once
#include <limits>
#include <string>
#include <string_view>

// 为了强制 asU( *reinterpret_cast<const uint64_t*>( p ) )
// #pragma GCC diagnostic push
// #pragma GCC diagnostic ignored "-Warray-bounds"

using str_t = std::string;
using stv_t = std::string_view;

namespace leon_utl {

union U16_u {
	U16_u() = default;
	constexpr U16_u( U16_u&& ) = default;
	constexpr U16_u( const U16_u& ) = default;
	constexpr U16_u& operator=( U16_u&& ) = default;
	constexpr U16_u& operator=( const U16_u& ) = default;

	constexpr U16_u( int16_t i_ )	:	asI( i_ ) {};
	constexpr U16_u( uint16_t u_ )	:	asU( u_ ) {};
	constexpr U16_u( const char* p_ )
		: b0( p_ ? p_[0] : 0 ), b1( p_ ? p_[1] : 0 ) {};
	U16_u( const str_t& s_ ): asU( *reinterpret_cast<const uint16_t*>( s_.c_str() ) ) {};

	constexpr U16_u operator=( int16_t i_ )		{ asI = i_; return *this; };
	constexpr U16_u operator=( uint16_t u_ )	{ asU = u_; return *this; };
	constexpr U16_u operator=( const char* p_ ) {
		b0 = p_ ? p_[0] : 0;
		b1 = p_ ? p_[1] : 0;
		return *this;
	};
	U16_u operator=( const str_t& s_ ) {
		asU = * reinterpret_cast<const uint16_t*>( s_.c_str() ); return *this;
	};

	constexpr bool operator< ( U16_u o_ ) const { return asU <  o_.asU; };
	constexpr bool operator<=( U16_u o_ ) const { return asU <= o_.asU; };
	constexpr bool operator> ( U16_u o_ ) const { return asU >  o_.asU; };
	constexpr bool operator>=( U16_u o_ ) const { return asU >= o_.asU; };
	constexpr bool operator!=( U16_u o_ ) const { return asU != o_.asU; };
	constexpr bool operator==( U16_u o_ ) const { return asU == o_.asU; };

	constexpr stv_t view() const {
		return stv_t( asA, sizeof( uint16_t ) );
	};

	str_t str() const {
		if( b1 == 0 )
			return asA;
		return str_t( asA, sizeof( uint16_t ) );
	};

	int16_t		asI;
	uint16_t	asU;
	char		asA[ sizeof( uint16_t ) ];
	struct		{ uint8_t b0; uint8_t b1; };
};

union U32_u {
	U32_u() = default;
	constexpr U32_u( U32_u&& ) = default;
	constexpr U32_u( const U32_u& ) = default;
	constexpr U32_u& operator=( U32_u&& ) = default;
	constexpr U32_u& operator=( const U32_u& ) = default;

	constexpr U32_u( float f_ )		: asF( f_ ) {};
	constexpr U32_u( int32_t i_ )	: asI( i_ ) {};
	constexpr U32_u( uint32_t u_ )	: asU( u_ ) {};
	constexpr U32_u operator=( float f_ )		{ asF = f_; return *this; };
	constexpr U32_u operator=( int32_t i_ )		{ asI = i_; return *this; };
	constexpr U32_u operator=( uint32_t u_ )	{ asU = u_; return *this; };

	U32_u( const char* p_ ): asU( *reinterpret_cast<const uint32_t*>( p_ ) ) {};
	U32_u( const str_t& s_ ): U32_u( s_.c_str() ) {};
	U32_u operator=( const char* p_ ) {
		asU = * reinterpret_cast<const uint32_t*>( p_ ); return *this;
	};
	U32_u operator=( const str_t& s_ ) {
		asU = * reinterpret_cast<const uint32_t*>( s_.c_str() ); return *this;
	};

	constexpr bool operator< ( U32_u o_ ) const { return asU <  o_.asU; };
	constexpr bool operator<=( U32_u o_ ) const { return asU <= o_.asU; };
	constexpr bool operator> ( U32_u o_ ) const { return asU >  o_.asU; };
	constexpr bool operator>=( U32_u o_ ) const { return asU >= o_.asU; };
	constexpr bool operator!=( U32_u o_ ) const { return asU != o_.asU; };
	constexpr bool operator==( U32_u o_ ) const { return asU == o_.asU; };

	constexpr stv_t view() const  {
		return stv_t( asA, sizeof( uint32_t ) );
	};

	str_t str() const  {
		// 有 Null char, 就可以当作 Null terminated string
		if( b3 == 0 || b2 == 0 || b1 == 0 )
			return asA;
		return str_t( asA, sizeof( uint32_t ) );
	};

	float		asF;
	int32_t		asI;
	uint32_t	asU;
	char		asA[ sizeof( uint32_t ) ];
	struct		{ uint16_t w0; uint16_t w1; };
	struct		{ uint8_t b0; uint8_t b1; uint8_t b2; uint8_t b3; };
};

union U64_u {
	U64_u() = default;
	constexpr U64_u( U64_u&& ) = default;
	constexpr U64_u( const U64_u& ) = default;
	constexpr U64_u& operator=( U64_u&& ) = default;
	constexpr U64_u& operator=( const U64_u& ) = default;

	constexpr U64_u( double d_ )	: asD( d_ ) {};
	constexpr U64_u( int64_t i_ )	: asI( i_ ) {};
	constexpr U64_u( uint64_t u_ )	: asU( u_ ) {};
	constexpr U64_u operator=( double d_ )		{ asD = d_; return *this; };
	constexpr U64_u operator=( int64_t i_ )		{ asI = i_; return *this; };
	constexpr U64_u operator=( uint64_t u_ )	{ asU = u_; return *this; };

	U64_u( const char* p_ ): asU( *reinterpret_cast<const uint64_t*>( p_ ) ) {};
	U64_u( const str_t& s_ ): U64_u( s_.c_str() ) {};
	U64_u operator=( const char* p_ ) {
		asU = * reinterpret_cast<const uint64_t*>( p_ ); return *this;
	};
	U64_u operator=( const str_t& s_ ) {
		asU = * reinterpret_cast<const uint64_t*>( s_.c_str() ); return *this;
	};

	constexpr bool operator< ( U64_u o_ ) const { return asU <  o_.asU; };
	constexpr bool operator<=( U64_u o_ ) const { return asU <= o_.asU; };
	constexpr bool operator> ( U64_u o_ ) const { return asU >  o_.asU; };
	constexpr bool operator>=( U64_u o_ ) const { return asU >= o_.asU; };
	constexpr bool operator!=( U64_u o_ ) const { return asU != o_.asU; };
	constexpr bool operator==( U64_u o_ ) const { return asU == o_.asU; };

	constexpr stv_t view() const  {
		return stv_t( asA, sizeof( uint64_t ) );
	};

	str_t str() const  {
		// 有 Null char, 就可以当作 Null terminated string
		if( b7 == 0 || b6 == 0 || b5 == 0 || b4 == 0 ||
				b3 == 0 || b2 == 0 || b1 == 0 )
			return asA;
		return str_t( asA, sizeof( uint64_t ) );
	};

	double		asD;
	int64_t		asI;
	uint64_t	asU;
	char		asA[ sizeof( uint64_t ) ];
	struct		{ uint32_t dw0; uint32_t dw1; };
	struct		{ uint16_t w0; uint16_t w1; uint16_t w2; uint16_t w3; };
	struct		{
		uint8_t b0; uint8_t b1; uint8_t b2; uint8_t b3;
		uint8_t b4; uint8_t b5; uint8_t b6; uint8_t b7;
	};
};

inline constexpr float		RawI32F( const int32_t i_ )		{ return U32_u( i_ ).asF; };
inline constexpr float		RawU32F( const uint32_t u_ )	{ return U32_u( u_ ).asF; };
inline constexpr int32_t	RawF32I( const float  f_ )		{ return U32_u( f_ ).asI; };
inline constexpr int32_t	RawU32I( const uint32_t u_ )	{ return U32_u( u_ ).asI; };
inline constexpr uint32_t	RawF32U( const float  f_ )		{ return U32_u( f_ ).asU; };
inline constexpr uint32_t	RawI32U( const int32_t i_ )		{ return U32_u( i_ ).asU; };

inline constexpr float		RawI64D( const int64_t i_ )		{ return U64_u( i_ ).asD; };
inline constexpr float		RawU64D( const uint64_t u_ )	{ return U64_u( u_ ).asD; };
inline constexpr int64_t	RawF64I( const double f_ )		{ return U64_u( f_ ).asI; };
inline constexpr int64_t	RawU64I( const uint64_t u_ )	{ return U64_u( u_ ).asI; };
inline constexpr uint64_t	RawF64U( const double f_ )		{ return U64_u( f_ ).asU; };
inline constexpr uint64_t	RawI64U( const int64_t i_ )		{ return U64_u( i_ ).asU; };

// 把浮点数的某些特殊值按照 U32/U64 的形式返回,可用于某些二进制的快速比较
constexpr U32_u F32_MAXIMU( std::numeric_limits<float>::max() );		//.asU = 0x7f7fffff
constexpr U32_u F32_LOWEST( std::numeric_limits<float>::lowest() );
constexpr U64_u D64_MAXIMU( std::numeric_limits<double>::max() );		//.asU = 0x7fefffffffffffff
constexpr U64_u D64_LOWEST( std::numeric_limits<double>::lowest() );

}; //namespace leon_utl

// #pragma GCC diagnostic pop

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
