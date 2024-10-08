#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <limits>

// #include "leonutils/Algorithms.hpp"
#include "leonutils/UnionTypes.hpp"

using namespace leon_utl;

TEST( TestUnionTypes, U16 ) {
	ASSERT_EQ( sizeof( U16_u ), 2 );
	ASSERT_TRUE( std::is_trivial<U16_u>::value );
	ASSERT_TRUE( std::is_standard_layout<U16_u>::value );

	U16_u u16 { "78" };
	ASSERT_EQ( u16.asA[0], '7' );
	ASSERT_EQ( u16.asA[1], '8' );

	u16 = "12";
	ASSERT_EQ( u16.asA[0], '1' );
	ASSERT_EQ( u16.asA[1], '2' );

	u16.asU = uint16_t( 0 );
	ASSERT_EQ( u16.asU, 0 );
	u16.asU = uint16_t( 0xffff );
	ASSERT_EQ( u16.asU, 65535 );
	ASSERT_EQ( u16.asI, -1 );

	u16.b0 = 0;
	u16.b1 = 255;
	ASSERT_EQ( u16.asU, 65535 - 255 );
	ASSERT_EQ( u16.asI, -1 - 255 );
};

TEST( TestUnionTypes, U32 ) {
	ASSERT_EQ( sizeof( U32_u ), 4 );
	ASSERT_TRUE( std::is_trivial<U32_u>::value );
	ASSERT_TRUE( std::is_standard_layout<U32_u>::value );

	U32_u u32 { "1234" };
	ASSERT_EQ( u32.asA[0], '1' );
	ASSERT_EQ( u32.asA[1], '2' );
	ASSERT_EQ( u32.asA[2], '3' );
	ASSERT_EQ( u32.asA[3], '4' );

	u32 = "5678";
	ASSERT_EQ( u32.asA[0], '5' );
	ASSERT_EQ( u32.asA[1], '6' );
	ASSERT_EQ( u32.asA[2], '7' );
	ASSERT_EQ( u32.asA[3], '8' );

	u32 = static_cast<uint32_t>( 0 );
	ASSERT_EQ( u32.asU, 0 );
	u32 = static_cast<uint32_t>( 0xffffffff );
	ASSERT_EQ( u32.asU, 0xffffffffU );
	ASSERT_EQ( u32.asI, -1 );

	u32 = 0u;
	ASSERT_EQ( u32.asI, 0 );
	ASSERT_EQ( u32.asU, 0u );
	u32.b3 = 128;
	ASSERT_EQ( u32.asU, 0x80000000U );
	ASSERT_EQ( u32.asI, 0x80000000 );
};

TEST( TestUnionTypes, U64 ) {
	ASSERT_EQ( sizeof( U64_u ), 8 );
// ASSERT_TRUE( std::is_trivial<U64_u>::value );
	ASSERT_TRUE( std::is_standard_layout<U64_u>::value );

	U64_u u64 { "12345678" };
	ASSERT_EQ( u64.asA[0], '1' );
	ASSERT_EQ( u64.asA[1], '2' );
	ASSERT_EQ( u64.asA[2], '3' );
	ASSERT_EQ( u64.asA[3], '4' );
	ASSERT_EQ( u64.asA[4], '5' );
	ASSERT_EQ( u64.asA[5], '6' );
	ASSERT_EQ( u64.asA[6], '7' );
	ASSERT_EQ( u64.asA[7], '8' );
	ASSERT_EQ( u64.view(), "12345678" );
	ASSERT_EQ( u64.str(), "12345678" );

	u64.b3 = 0;
	ASSERT_STREQ( u64.asA, "123" );
	str_t str0 = str_t( "123" ).append( 1, 0 ).append( "5678" );
	ASSERT_EQ( u64.str(), "123" );
	stv_t sv { str0 };
	ASSERT_EQ( u64.view(), sv );

	u64 = static_cast<uint64_t>( 0 );
	ASSERT_EQ( u64.asU, 0UL );
	u64 = static_cast<uint64_t>( 0xffffffffffffffffUL );
	ASSERT_EQ( u64.asU, 0xffffffffffffffffUL );
	ASSERT_EQ( u64.asI, -1UL );

	u64 = 0UL;
	ASSERT_EQ( u64.asI, 0 );
	ASSERT_EQ( u64.asU, 0UL );
	u64.b7 = 128;
	ASSERT_EQ( u64.asU, 0x8000000000000000UL );
	ASSERT_EQ( u64.asI, 0x8000000000000000L );
};

// cstring "硬转"为 uint64_t
TEST( TestUnionTypes, Convertion6Chars ) {
	U64_u uca2u;

	ASSERT_EQ( sizeof( uca2u ), 8 );

	uca2u = 0xFFFFFFFFFFFFFFFF;
	EXPECT_EQ( uca2u.b0, 0xFF );
	EXPECT_EQ( uca2u.b1, 0xFF );
	EXPECT_EQ( uca2u.b2, 0xFF );
	EXPECT_EQ( uca2u.b3, 0xFF );
	EXPECT_EQ( uca2u.b4, 0xFF );
	EXPECT_EQ( uca2u.b5, 0xFF );
	EXPECT_EQ( uca2u.b6, 0xFF );
	EXPECT_EQ( uca2u.b7, 0xFF );

	strncpy( uca2u.asA, "123456", 8 );
	EXPECT_EQ( uca2u.asU, 59602136937009 );
	EXPECT_EQ( uca2u.b0, '1' );
	EXPECT_EQ( uca2u.b1, '2' );
	EXPECT_EQ( uca2u.b2, '3' );
	EXPECT_EQ( uca2u.b3, '4' );
	EXPECT_EQ( uca2u.b4, '5' );
	EXPECT_EQ( uca2u.b5, '6' );
	EXPECT_EQ( uca2u.b6,  0 );
	EXPECT_EQ( uca2u.b7,  0 );

	uca2u = 0xFFFFFFFFFFFFFFFF;
	strncpy( uca2u.asA, "12345", 8 );
	EXPECT_EQ( uca2u.w0, 12849 );
	EXPECT_EQ( uca2u.w1, 13363 );
	EXPECT_EQ( uca2u.b0, '1' );
	EXPECT_EQ( uca2u.b1, '2' );
	EXPECT_EQ( uca2u.b2, '3' );
	EXPECT_EQ( uca2u.b3, '4' );
	EXPECT_EQ( uca2u.b4, '5' );
	EXPECT_EQ( uca2u.b5,  0 );
	EXPECT_EQ( uca2u.b6,  0 );
	EXPECT_EQ( uca2u.b7,  0 );
};

// cstring "硬转"为 uint64_t
TEST( TestUnionTypes, strncpyWipeTail ) {
	U64_u uca2u0, uca2u1;

	uca2u0.b0 = '0';
	uca2u0.b1 = '1';
	uca2u0.b2 = '2';
	uca2u0.b3 =  0 ;
	uca2u0.b4 = '4';
	uca2u0.b5 = '5';
	uca2u0.b6 = '6';
	uca2u0.b7 = '7';
	uca2u1.asU = 0xFFFFFFFFFFFFFFFF;

	strncpy( uca2u1.asA, uca2u0.asA, 8 );
	EXPECT_EQ( uca2u1.b0, '0' );
	EXPECT_EQ( uca2u1.b1, '1' );
	EXPECT_EQ( uca2u1.b2, '2' );
	EXPECT_EQ( uca2u1.b3,  0 );
	EXPECT_EQ( uca2u1.b4,  0 );
	EXPECT_EQ( uca2u1.b5,  0 );
	EXPECT_EQ( uca2u1.b6,  0 );
	EXPECT_EQ( uca2u1.b7,  0 );

	uca2u0.b0 = '0';
	uca2u0.b1 = '1';
	uca2u0.b2 = '2';
	uca2u0.b3 = '3';
	uca2u0.b4 = '4';
	uca2u0.b5 = '5';
	uca2u0.b6 = '6';
	uca2u0.b7 = '7';
	uca2u1 = 0xFFFFFFFFFFFFFFFF;

	strncpy( uca2u1.asA, uca2u0.asA, 8 );
	EXPECT_EQ( uca2u1.b0, '0' );
	EXPECT_EQ( uca2u1.b1, '1' );
	EXPECT_EQ( uca2u1.b2, '2' );
	EXPECT_EQ( uca2u1.b3, '3' );
	EXPECT_EQ( uca2u1.b4, '4' );
	EXPECT_EQ( uca2u1.b5, '5' );
	EXPECT_EQ( uca2u1.b6, '6' );
	EXPECT_EQ( uca2u1.b7, '7' );
};

TEST( TestUnionTypes, convertFromToString ) {
	U64_u u0;
	u0.b0 = '0';
	u0.b1 = '1';
	u0.b2 = '2';
	u0.b3 = '3';
	u0.b4 = '4';
	u0.b5 = '5';
	u0.b6 = '6';
	u0.b7 = '7';

	str_t s0 = u0.str();
	ASSERT_EQ( s0, str_t( "01234567" ) );
	u0.b3 = '\0';
	s0 = u0.str();
	str_t s1 = { '0', '1', '2', '\0', '4', '5', '6', '7' };
	ASSERT_EQ( s0, str_t( "012" ) );
	ASSERT_EQ( u0.view(), stv_t( s1 ) );

	s0 = { 'A', 'B', '\0', 'D', 'E', 'F', 'G', 'H' };
	u0 = s0;
	ASSERT_EQ( u0.b0, 'A' );
	ASSERT_EQ( u0.b1, 'B' );
	ASSERT_EQ( u0.b2, '\0' );
	ASSERT_EQ( u0.b3, 'D' );
	ASSERT_EQ( u0.b4, 'E' );
	ASSERT_EQ( u0.b5, 'F' );
	ASSERT_EQ( u0.b6, 'G' );
	ASSERT_EQ( u0.b7, 'H' );
};

TEST( TestUnionTypes, RawConvert ) {
	ASSERT_EQ( RawU32I( uint32_t( 0x80000000 ) ), int32_t( -2147483648 ) );
	ASSERT_EQ( RawU32I( uint32_t( 0x80000001 ) ), int32_t( -2147483647 ) );
	ASSERT_EQ( RawU32I( uint32_t( 0xffffffff ) ), int32_t( -1 ) );
	ASSERT_EQ( RawU32I( uint32_t( 0 ) ),          int32_t( 0 ) );
	ASSERT_EQ( RawU32I( uint32_t( 0x7fffffff ) ), int32_t( 2147483647 ) );

	ASSERT_EQ( RawI32U( int32_t( -2147483648 ) ), uint32_t( 0x80000000 ) );
	ASSERT_EQ( RawI32U( int32_t( -2147483647 ) ), uint32_t( 0x80000001 ) );
	ASSERT_EQ( RawI32U( int32_t( -1 ) ),          uint32_t( 0xffffffff ) );
	ASSERT_EQ( RawI32U( int32_t( 0 ) ),           uint32_t( 0 ) );
	ASSERT_EQ( RawI32U( int32_t( 2147483647 ) ),  uint32_t( 0x7fffffff ) );

	ASSERT_EQ( RawU64I( uint64_t( 0x8000000000000000 ) ), std::numeric_limits<int64_t>::min() );
	ASSERT_EQ( RawU64I( uint64_t( 0x8000000000000001 ) ), std::numeric_limits<int64_t>::min() + 1 );
	ASSERT_EQ( RawU64I( uint64_t( 0xffffffffffffffff ) ), int64_t( -1 ) );
	ASSERT_EQ( RawU64I( uint64_t( 0 ) ),                   int64_t( 0 ) );
	ASSERT_EQ( RawU64I( uint64_t( 0x7fffffffffffffff ) ), std::numeric_limits<int64_t>::max() );

	ASSERT_EQ( RawI64U( std::numeric_limits<int64_t>::min() ),     uint64_t( 0x8000000000000000 ) );
	ASSERT_EQ( RawI64U( std::numeric_limits<int64_t>::min() + 1 ), uint64_t( 0x8000000000000001 ) );
	ASSERT_EQ( RawI64U( int64_t( -1 ) ),                      uint64_t( 0xffffffffffffffff ) );
	ASSERT_EQ( RawI64U( int64_t( 0 ) ),                       uint64_t( 0 ) );
	ASSERT_EQ( RawI64U( std::numeric_limits<int64_t>::max() ),     uint64_t( 0x7fffffffffffffff ) );

//	ASSERT_TRUE( leon_utl::eq( RawU64D( 0x407b4999999999abUL ), 436.6 ) );
};

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
