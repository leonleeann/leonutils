#include <cmath>        // abs, ceil, floor, isnan, log, log10, pow, round, sqrt
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <limits>

#include "leonutils/Algorithms.hpp"

using namespace leon_utl;
using std::numeric_limits;

TEST( TestAlgorithms, numericChecking ) {
//	float f0 = numeric_limits<float>::quiet_NaN();
//	ASSERT_TRUE( std::isnan( f0 ) );
//	ASSERT_TRUE( isnan( f0 ) );
//	ASSERT_FALSE( strict_max( f0 ) );

	ASSERT_TRUE( strict_max( numeric_limits<float>::max() ) );
	ASSERT_FALSE( strict_max( numeric_limits<float>::quiet_NaN() ) );
	ASSERT_TRUE( strict_low( numeric_limits<float>::lowest() ) );

	ASSERT_TRUE( strict_max( numeric_limits<double>::max() ) );
	ASSERT_FALSE( strict_max( numeric_limits<double>::quiet_NaN() ) );
	ASSERT_TRUE( strict_low( numeric_limits<double>::lowest() ) );

	char	cc = 123;
	int		ii = 123;
	float	f7 =  1234567.1;
	float	f8 = 12345678.1;
	double	d15 =  999999999.1;
	double	d16 = 9999999999.1;

	ASSERT_TRUE( about_int( cc ) );
	ASSERT_TRUE( about_int( ii ) );
	ASSERT_FALSE( about_int( f7 ) );
	ASSERT_TRUE( about_int( f8 ) );
	ASSERT_FALSE( about_int( d15 ) );
	ASSERT_TRUE( about_int( d16 ) );

	f7 =  -1234567.1;
	f8 = -12345678.1;
	d15 =  -999999999.1;
	d16 = -9999999999.1;
	ASSERT_FALSE( about_int( f7 ) );
	ASSERT_TRUE( about_int( f8 ) );
	ASSERT_FALSE( about_int( d15 ) );
	ASSERT_TRUE( about_int( d16 ) );

	ASSERT_FALSE( about_int( 0.00001f ) );
	ASSERT_TRUE( about_int( 0.000f ) );
	ASSERT_FALSE( about_int( 0.000000001l ) );
	ASSERT_TRUE( about_int( 0.0000000000l ) );
};

TEST( TestAlgorithms, signFunction ) {
	char c0 = -128;
	ASSERT_EQ( sign( c0 ), -1 );
	c0 = 127;
	ASSERT_EQ( sign( c0 ), 1 );
	c0 = 0;
	ASSERT_EQ( sign( c0 ), 0 );

	unsigned char c1 = 0;
	ASSERT_EQ( sign( c1 ), 0 );
	c1 = 255;
	ASSERT_EQ( sign( c1 ), 1 );
	++c1;
	ASSERT_EQ( sign( c1 ), 0 );

	int i32 = 0xFFFFFFFF;
	ASSERT_EQ( sign( i32 ), -1 );
	++i32;
	ASSERT_EQ( sign( i32 ), 0 );
	i32 = std::numeric_limits<int>::lowest();
	--i32;
	ASSERT_EQ( sign( i32 ), 1 );
	ASSERT_EQ( sign( std::numeric_limits<int>::max() ), 1 );

	uint32_t u32 = std::numeric_limits<uint32_t>::lowest();
	ASSERT_EQ( sign( u32 ), 0 );
	--u32;
	ASSERT_EQ( sign( u32 ), 1 );
	u32 = std::numeric_limits<uint32_t>::max();
	ASSERT_EQ( sign( u32 ), 1 );
	++u32;
	ASSERT_EQ( sign( u32 ), 0 );

	float f32 = 0.0f;
	ASSERT_EQ( sign( f32 ), 0 );
	ASSERT_EQ( sign( std::numeric_limits<float>::lowest() ), -1 );
	ASSERT_EQ( sign( std::numeric_limits<float>::min() ), 1 );
	ASSERT_EQ( sign( std::numeric_limits<float>::max() ), 1 );

	double f64 = 0.0;
	ASSERT_EQ( sign( f64 ), 0 );
	ASSERT_EQ( sign( std::numeric_limits<double>::lowest() ), -1 );
	ASSERT_EQ( sign( std::numeric_limits<double>::min() ), 1 );
	ASSERT_EQ( sign( std::numeric_limits<double>::max() ), 1 );
};

TEST( TestAlgorithms, int32Equality ) {
	int i32 = 2147483647, i2 = 2147483647;
	ASSERT_TRUE( eq( i32, i2 ) );
	--i2;
	ASSERT_FALSE( eq( i32, i2 ) );

	i32 = std::numeric_limits<int32_t>::max();
	uint32_t u32 = i32;
	ASSERT_TRUE( eq( i32, u32 ) );
	ASSERT_TRUE( eq( u32, i32 ) );
	ASSERT_TRUE( strict_max( i32 ) );
	ASSERT_FALSE( strict_max( u32 ) );
};

TEST( TestAlgorithms, int64Equality ) {
	int64_t  i64 = std::numeric_limits<int64_t>::max();
	uint64_t u64 = i64;
	ASSERT_TRUE( eq( i64, u64 ) );
	ASSERT_TRUE( eq( u64, i64 ) );
	ASSERT_TRUE( strict_max( i64 ) );
	ASSERT_FALSE( strict_max( u64 ) );
};

TEST( TestAlgorithms, floatEquality ) {
	float f1 = 1000000.0f, f2 = 1000000.1f;
	ASSERT_TRUE( eq( f1, f2 ) );
	f2 = 1000001.0f;
	ASSERT_FALSE( eq( f1, f2 ) );
	f1 = 0.0f;
	ASSERT_FALSE( eq( f1, f2 ) );
	f2 = 0.0f;
	ASSERT_TRUE( eq( f1, f2 ) );

	f2 = f1 = std::numeric_limits<float>::quiet_NaN();
	ASSERT_TRUE( eq( f1, f2 ) );

	f1 = std::numeric_limits<float>::infinity();
	f2 = 1000001.0f;
	ASSERT_FALSE( eq( f1, f2 ) );
	f2 = f1;
	ASSERT_TRUE( eq( f1, f2 ) );
	ASSERT_TRUE( eq( f2, f1 ) );

	U32_u f2uT;
	f2uT.asF = std::numeric_limits<float>::max();
	ASSERT_TRUE( strict_max( f2uT.asF ) );
	ASSERT_FALSE( strict_max( f1 ) );
	ASSERT_FALSE( eq( f1, f2uT.asF ) );
	ASSERT_FALSE( eq( f2uT.asF, f1 ) );
	f1 = f2uT.asF;
	ASSERT_TRUE( f1 == f2uT.asF );
	ASSERT_TRUE( eq( f1, f2uT.asF ) );
	ASSERT_TRUE( eq( f2uT.asF, f1 ) );
	--f2uT.asU;
	ASSERT_FALSE( strict_max( f2uT.asF ) );
	ASSERT_FALSE( f1 == f2uT.asF );
	ASSERT_TRUE( eq( f1, f2uT.asF ) );
	ASSERT_TRUE( eq( f2uT.asF, f1 ) );
};

TEST( TestAlgorithms, floatEqualWithEpsilon ) {
	float f1 = 100.0f, f2 = 100.1f;
	ASSERT_TRUE( eq( f1, f2, 1.0f ) );
	ASSERT_FALSE( eq( f1, f2, 0.09f ) );
	f2 = f1 = std::numeric_limits<float>::quiet_NaN();
	ASSERT_TRUE( eq( f1, f2 ) );

	f2 = f1 = std::numeric_limits<float>::infinity();
	ASSERT_TRUE( eq( f1, f2, 10000.f ) );
	f2 = - std::numeric_limits<float>::infinity();
	ASSERT_FALSE( eq( f1, f2, 10000.f ) );
};

TEST( TestAlgorithms, doubleEqualWithEpsilon ) {
	double d1 = 100.0f, d2 = 100.1;
	ASSERT_TRUE( eq( d1, d2, 1.0 ) );
	ASSERT_FALSE( eq( d1, d2, 0.09 ) );
	d2 = d1 = std::numeric_limits<double>::quiet_NaN();
	ASSERT_TRUE( eq( d1, d2 ) );

	d2 = d1 = std::numeric_limits<double>::infinity();
	ASSERT_TRUE( eq( d1, d2, 10000. ) );
	d2 = - std::numeric_limits<double>::infinity();
	ASSERT_FALSE( eq( d1, d2, 10000. ) );
};

TEST( TestAlgorithms, doulbeEquality ) {
	double d1 = 1234567890.0, d2 = 1234567890.1;
	/* U64_u u2 = d1; ++u2.asU; d2 = u2.asD;
	lg_fatl << "d1:" << format( d1, 0, 30 )
	          << ", d2:" << format( d2, 0, 30 );*/
	ASSERT_TRUE( eq( d1, d2 ) );
	d2 = 1234567890.2;
	ASSERT_FALSE( eq( d1, d2 ) );
	d1 = 0.0;
	ASSERT_FALSE( eq( d1, d2 ) );
	d2 = 0.0;
	ASSERT_TRUE( eq( d1, d2 ) );

	d2 = d1 = std::numeric_limits<double>::quiet_NaN();
	ASSERT_TRUE( eq( d1, d2 ) );

	d1 = std::numeric_limits<double>::infinity();
	d2 = 1000001.0f;
	ASSERT_FALSE( eq( d1, d2 ) );
	ASSERT_FALSE( eq( d2, d1 ) );
	d2 = d1;
	ASSERT_TRUE( eq( d1, d2 ) );
	ASSERT_TRUE( eq( d2, d1 ) );

	U64_u d2uT;
	d2uT.asD = std::numeric_limits<double>::max();
	ASSERT_TRUE( strict_max( d2uT.asD ) );
	ASSERT_FALSE( strict_max( d1 ) );
	ASSERT_FALSE( eq( d1, d2uT.asD ) );
	ASSERT_FALSE( eq( d2uT.asD, d1 ) );
	d1 = d2uT.asD = std::numeric_limits<double>::max();
	ASSERT_TRUE( d1 == d2uT.asD );
	ASSERT_TRUE( eq( d1, d2uT.asD ) );
	ASSERT_TRUE( eq( d2uT.asD, d1 ) );
	--d2uT.asU;
	ASSERT_FALSE( strict_max( d2uT.asD ) );
	ASSERT_FALSE( d1 == d2uT.asD );
	ASSERT_TRUE( eq( d1, d2uT.asD ) );
	ASSERT_TRUE( eq( d2uT.asD, d1 ) );
};

TEST( TestAlgorithms, floatGreatLessThan ) {
	U32_u f1, f2;
	f1.asF = 3.141592654;
	f2.asU = f1.asU - 1;
	ASSERT_FALSE( f1.asF == f2.asF );
	ASSERT_TRUE( f1.asF > f2.asF );
	ASSERT_TRUE( eq( f1.asF, f2.asF ) );
	ASSERT_FALSE( gt( f1.asF, f2.asF ) );
	ASSERT_FALSE( gt( f2.asF, f1.asF ) );
	ASSERT_TRUE( ge( f1.asF, f2.asF ) );
	ASSERT_TRUE( ge( f2.asF, f1.asF ) );
	ASSERT_TRUE( le( f1.asF, f2.asF ) );
	ASSERT_TRUE( le( f2.asF, f1.asF ) );
	ASSERT_FALSE( lt( f1.asF, f2.asF ) );
	ASSERT_FALSE( lt( f2.asF, f1.asF ) );

	f2.asF = f1.asF / 2;
	ASSERT_TRUE( f1.asF > f2.asF );
	ASSERT_FALSE( eq( f1.asF, f2.asF ) );
	ASSERT_TRUE( gt( f1.asF, f2.asF ) );
	ASSERT_FALSE( gt( f2.asF, f1.asF ) );
	ASSERT_TRUE( ge( f1.asF, f2.asF ) );
	ASSERT_FALSE( ge( f2.asF, f1.asF ) );
	ASSERT_FALSE( le( f1.asF, f2.asF ) );
	ASSERT_TRUE( le( f2.asF, f1.asF ) );
	ASSERT_FALSE( lt( f1.asF, f2.asF ) );
	ASSERT_TRUE( lt( f2.asF, f1.asF ) );

	f1.asF = 3.141592654;
	f2.asF = f1.asF + 0.4999;
	ASSERT_FALSE( gt( f2.asF, f1.asF, 0.5F ) );
	ASSERT_TRUE( gt( f2.asF, f1.asF, 0.25F ) );
};

TEST( TestAlgorithms, doubleGreatLessThan ) {
	U64_u d1, d2;
	d1.asD = 3.141592654;
	d2.asU = d1.asU - 1;
	ASSERT_FALSE( d1.asD == d2.asD );
	ASSERT_TRUE( d1.asD > d2.asD );
	ASSERT_TRUE( eq( d1.asD, d2.asD ) );
	ASSERT_FALSE( gt( d1.asD, d2.asD ) );
	ASSERT_FALSE( gt( d2.asD, d1.asD ) );
	ASSERT_TRUE( ge( d1.asD, d2.asD ) );
	ASSERT_TRUE( ge( d2.asD, d1.asD ) );
	ASSERT_TRUE( le( d1.asD, d2.asD ) );
	ASSERT_TRUE( le( d2.asD, d1.asD ) );
	ASSERT_FALSE( lt( d1.asD, d2.asD ) );
	ASSERT_FALSE( lt( d2.asD, d1.asD ) );

	d2.asD = d1.asD / 2;
	ASSERT_TRUE( d1.asD > d2.asD );
	ASSERT_FALSE( eq( d1.asD, d2.asD ) );
	ASSERT_TRUE( gt( d1.asD, d2.asD ) );
	ASSERT_FALSE( gt( d2.asD, d1.asD ) );
	ASSERT_TRUE( ge( d1.asD, d2.asD ) );
	ASSERT_FALSE( ge( d2.asD, d1.asD ) );
	ASSERT_FALSE( le( d1.asD, d2.asD ) );
	ASSERT_TRUE( le( d2.asD, d1.asD ) );
	ASSERT_FALSE( lt( d1.asD, d2.asD ) );
	ASSERT_TRUE( lt( d2.asD, d1.asD ) );

	d1.asD = 3.141592654;
	d2.asD = d1.asD + 0.4999;
	ASSERT_FALSE( gt( d2.asD, d1.asD, 0.5 ) );
	ASSERT_TRUE( ge( d2.asD, d1.asD, 0.5 ) );
	ASSERT_TRUE( le( d2.asD, d1.asD, 0.5 ) );

	ASSERT_TRUE( gt( d2.asD, d1.asD, 0.25 ) );
	ASSERT_FALSE( le( d2.asD, d1.asD, 0.25 ) );
};

TEST( TestAlgorithms, extreamFloatValues ) {
	float nan1 = std::numeric_limits<float>::quiet_NaN();
	float inf1 = std::numeric_limits<float>::infinity();
	float neg_inf = - std::numeric_limits<float>::infinity();
	float max1 = std::numeric_limits<float>::max();
	// 正无穷 加一点点, 还相等吗?
	ASSERT_TRUE( eq( inf1 + 1, inf1 ) );
	ASSERT_TRUE( eq( neg_inf - 1, neg_inf ) );
	ASSERT_FALSE( eq( neg_inf, inf1 ) );
	U32_u u32;
	u32.asF = neg_inf;
	std::cout << "inf::asF" << u32.asU << std::endl;
	u32.asF = neg_inf - 1;
	std::cout << "inf-1::asF" << u32.asU << std::endl;

	ASSERT_FALSE( nan1 > inf1 );
	ASSERT_FALSE( nan1 < inf1 );
	ASSERT_TRUE( inf1 > max1 );
	ASSERT_FALSE( nan1 > max1 );
	ASSERT_FALSE( nan1 < max1 );

	ASSERT_FALSE( eq( nan1, inf1 ) );
	ASSERT_FALSE( eq( nan1, max1 ) );
	ASSERT_FALSE( eq( inf1, nan1 ) );
	ASSERT_FALSE( eq( inf1, max1 ) );
	ASSERT_FALSE( eq( max1, inf1 ) );
	ASSERT_FALSE( eq( max1, nan1 ) );

	ASSERT_FALSE( gt( nan1, inf1 ) );
	ASSERT_FALSE( gt( nan1, max1 ) );
	ASSERT_FALSE( gt( inf1, nan1 ) );
	ASSERT_TRUE( gt( inf1, max1 ) );
	ASSERT_FALSE( gt( max1, nan1 ) );
	ASSERT_FALSE( gt( max1, inf1 ) );

	ASSERT_FALSE( ge( nan1, inf1 ) );
	ASSERT_FALSE( ge( nan1, max1 ) );
	ASSERT_FALSE( ge( inf1, nan1 ) );
	ASSERT_TRUE( ge( inf1, max1 ) );
	ASSERT_FALSE( ge( max1, nan1 ) );
	ASSERT_FALSE( ge( max1, inf1 ) );

	ASSERT_FALSE( lt( nan1, inf1 ) );
	ASSERT_FALSE( lt( nan1, max1 ) );
	ASSERT_FALSE( lt( inf1, nan1 ) );
	ASSERT_FALSE( lt( inf1, max1 ) );
	ASSERT_FALSE( lt( max1, nan1 ) );
	ASSERT_TRUE( lt( max1, inf1 ) );

	ASSERT_FALSE( le( nan1, inf1 ) );
	ASSERT_FALSE( le( nan1, max1 ) );
	ASSERT_FALSE( le( inf1, nan1 ) );
	ASSERT_FALSE( le( inf1, max1 ) );
	ASSERT_FALSE( le( max1, nan1 ) );
	ASSERT_TRUE( le( max1, inf1 ) );
};

TEST( TestAlgorithms, extreamDoubleValues ) {
	double nan1 = std::numeric_limits<double>::quiet_NaN();
	double inf1 = std::numeric_limits<double>::infinity();
	double max1 = std::numeric_limits<double>::max();
	ASSERT_FALSE( nan1 > inf1 );
	ASSERT_FALSE( nan1 < inf1 );
	ASSERT_TRUE( inf1 > max1 );
	ASSERT_FALSE( nan1 > max1 );
	ASSERT_FALSE( nan1 < max1 );

	ASSERT_FALSE( eq( nan1, inf1 ) );
	ASSERT_FALSE( eq( nan1, max1 ) );
	ASSERT_FALSE( eq( inf1, nan1 ) );
	ASSERT_FALSE( eq( inf1, max1 ) );
	ASSERT_FALSE( eq( max1, inf1 ) );
	ASSERT_FALSE( eq( max1, nan1 ) );

	ASSERT_FALSE( gt( nan1, inf1 ) );
	ASSERT_FALSE( gt( nan1, max1 ) );
	ASSERT_FALSE( gt( inf1, nan1 ) );
	ASSERT_TRUE( gt( inf1, max1 ) );
	ASSERT_FALSE( gt( max1, nan1 ) );
	ASSERT_FALSE( gt( max1, inf1 ) );

	ASSERT_FALSE( ge( nan1, inf1 ) );
	ASSERT_FALSE( ge( nan1, max1 ) );
	ASSERT_FALSE( ge( inf1, nan1 ) );
	ASSERT_TRUE( ge( inf1, max1 ) );
	ASSERT_FALSE( ge( max1, nan1 ) );
	ASSERT_FALSE( ge( max1, inf1 ) );

	ASSERT_FALSE( lt( nan1, inf1 ) );
	ASSERT_FALSE( lt( nan1, max1 ) );
	ASSERT_FALSE( lt( inf1, nan1 ) );
	ASSERT_FALSE( lt( inf1, max1 ) );
	ASSERT_FALSE( lt( max1, nan1 ) );
	ASSERT_TRUE( lt( max1, inf1 ) );

	ASSERT_FALSE( le( nan1, inf1 ) );
	ASSERT_FALSE( le( nan1, max1 ) );
	ASSERT_FALSE( le( inf1, nan1 ) );
	ASSERT_FALSE( le( inf1, max1 ) );
	ASSERT_FALSE( le( max1, nan1 ) );
	ASSERT_TRUE( le( max1, inf1 ) );
};

TEST( TestAlgorithms, findDecimals ) {
	ASSERT_EQ( decimals( 100 ), 0 );
	ASSERT_EQ( decimals( 10 ), 0 );
	ASSERT_EQ( decimals( 1 ), 0 );

	ASSERT_EQ( decimals( 0.1 ), 1 );
	ASSERT_EQ( decimals( 0.5 ), 1 );
	ASSERT_EQ( decimals( 0.9 ), 1 );

	ASSERT_EQ( decimals( 0.01 ), 2 );
	ASSERT_EQ( decimals( 0.99 ), 2 );
	ASSERT_EQ( decimals( -0.01 ), 2 );
	ASSERT_EQ( decimals( -0.99 ), 2 );

	ASSERT_EQ( decimals( 0.001 ), 3 );
	ASSERT_EQ( decimals( 0.015 ), 3 );
	ASSERT_EQ( decimals( 0.055 ), 3 );
	ASSERT_EQ( decimals( 0.099 ), 3 );
	ASSERT_EQ( decimals( 0.999 ), 3 );
	ASSERT_EQ( decimals( -0.001 ), 3 );
	ASSERT_EQ( decimals( -0.015 ), 3 );
	ASSERT_EQ( decimals( -0.055 ), 3 );
	ASSERT_EQ( decimals( -0.099 ), 3 );
	ASSERT_EQ( decimals( -0.999 ), 3 );

	ASSERT_EQ( decimals( 1.0001 ), 4 );
	ASSERT_EQ( decimals( 1.0015 ), 4 );
	ASSERT_EQ( decimals( 1.0055 ), 4 );
	ASSERT_EQ( decimals( 1.0099 ), 4 );
	ASSERT_EQ( decimals( 0.9999 ), 4 );
	ASSERT_EQ( decimals( -1.0001 ), 4 );
	ASSERT_EQ( decimals( -1.0015 ), 4 );
	ASSERT_EQ( decimals( -1.0055 ), 4 );
	ASSERT_EQ( decimals( -1.0099 ), 4 );
	ASSERT_EQ( decimals( -0.9999 ), 4 );

	ASSERT_EQ( decimals( 1.00001 ), 5 );
	ASSERT_EQ( decimals( 1.00015 ), 5 );
	ASSERT_EQ( decimals( 1.00055 ), 5 );
	ASSERT_EQ( decimals( 1.00099 ), 5 );
	ASSERT_EQ( decimals( 0.99999 ), 5 );
	ASSERT_EQ( decimals( -1.00001 ), 5 );
	ASSERT_EQ( decimals( -1.00015 ), 5 );
	ASSERT_EQ( decimals( -1.00055 ), 5 );
	ASSERT_EQ( decimals( -1.00099 ), 5 );
	ASSERT_EQ( decimals( -0.99999 ), 5 );
};

TEST( TestAlgorithms, numericConvert ) {
	char     cc = 123;
	int      ii = 123;
	float    ff = 123456.1;
	double   dd = 999999999.1;

	ASSERT_EQ( round2i32( cc ), 123 );
	ASSERT_EQ( round2i32( ii ), 123 );
	ASSERT_EQ( round2i32( ff ), 123456 );
	ASSERT_EQ( round2i32( dd ), 999999999 );

	ff = 123456.5;
	dd = 999999999.5;
	ASSERT_EQ( round2i32( ff ), 123457 );
	ASSERT_EQ( round2i32( dd ), 1000000000 );

	ff = -123456.1;
	dd = -999999999.1;
	ASSERT_EQ( round2i32( ff ), -123456 );
	ASSERT_EQ( round2i32( dd ), -999999999 );

	ff = -123456.5;
	dd = -999999999.5;
	ASSERT_EQ( round2i32( ff ), -123456 );
	ASSERT_EQ( round2i32( dd ), -999999999 );

	ff = -123456.51;
	dd = -999999999.50001;
	ASSERT_EQ( round2i32( ff ), -123457 );
	ASSERT_EQ( round2i32( dd ), -1000000000 );
};

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
