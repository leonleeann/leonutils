#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "leonutils/Converts.hpp"

using namespace leon_utl;
using namespace std;

TEST( TestConverts, is_alpha ) {
	ASSERT_FALSE( is_alpha( '\0' ) );
	ASSERT_FALSE( is_alpha( '\255' ) );
	ASSERT_FALSE( is_alpha( '.' ) );
	ASSERT_FALSE( is_alpha( '0' ) );
	ASSERT_FALSE( is_alpha( '9' ) );
	ASSERT_TRUE( is_alpha( 'a' ) );
	ASSERT_TRUE( is_alpha( 'A' ) );
	ASSERT_TRUE( is_alpha( 'Z' ) );
};

TEST( TestConverts, is_digit ) {
	ASSERT_FALSE( is_digit( '\0' ) );
	ASSERT_FALSE( is_digit( '\255' ) );
	ASSERT_FALSE( is_digit( '.' ) );
	ASSERT_FALSE( is_digit( 'a' ) );
	ASSERT_FALSE( is_digit( 'A' ) );
	ASSERT_FALSE( is_digit( 'z' ) );
	ASSERT_FALSE( is_digit( 'Z' ) );
	ASSERT_TRUE( is_digit( '0' ) );
	ASSERT_TRUE( is_digit( '1' ) );
	ASSERT_TRUE( is_digit( '9' ) );
};

TEST( TestConverts, is_number ) {
	ASSERT_FALSE( is_number( "        ", 8 ) );
	ASSERT_FALSE( is_number( "       ", 8 ) );
	ASSERT_FALSE( is_number( "\0       ", 8 ) );
	ASSERT_FALSE( is_number( "a", 1 ) );
	ASSERT_FALSE( is_number( " a    \0\0", 8 ) );
	ASSERT_FALSE( is_number( " a", 3 ) );
	ASSERT_FALSE( is_number( " a", 2 ) );
	ASSERT_FALSE( is_number( "123a    ", 8 ) );
	ASSERT_FALSE( is_number( " 123a  ", 8 ) );
	ASSERT_FALSE( is_number( " 12 34 ", 8 ) );
	ASSERT_TRUE( is_number( "123    ", 8 ) );
	ASSERT_TRUE( is_number( "123", 3 ) );
	ASSERT_TRUE( is_number( "123", 4 ) );
	ASSERT_TRUE( is_number( "  123   ", 8 ) );
	ASSERT_TRUE( is_number( "     123", 8 ) );
	ASSERT_TRUE( is_number( "11\0    ", 8 ) );
	ASSERT_TRUE( is_number( "  123  \0", 8 ) );
};

TEST( TestConverts, trimString ) {
	string s0 = " ABC ";
	ASSERT_EQ( trim_l( s0 ), string( "ABC " ) );
	ASSERT_EQ( trim_r( s0 ), string( " ABC" ) );
	ASSERT_EQ( trim( s0 ), string( "ABC" ) );
};

TEST( TestConverts, padString ) {
	string s0 = "ABC";
	ASSERT_EQ( pad_l( s0, 5 ), string( " ABC " ) );
	ASSERT_EQ( pad_l( s0, 5, "" ), string( "  ABC" ) );
	ASSERT_EQ( pad_l( s0, 5, "", '=' ), string( "==ABC" ) );

	ASSERT_EQ( pad_l( "ABCD", 5 ), string( "ABCD " ) );
	ASSERT_EQ( pad_l( "ABCD", 5, "" ), string( " ABCD" ) );
	ASSERT_EQ( pad_l( "====", 8, "ccc", '/' ), string( "/====ccc" ) );
	ASSERT_EQ( pad_l( "====", 6, "ccc", '/' ), string( "====ccc" ) );

	ASSERT_EQ( pad_r( "ABCD", 6 ), string( " ABCD " ) );
	ASSERT_EQ( pad_r( "ABCD", 6, "" ), string( "ABCD  " ) );
	ASSERT_EQ( pad_r( "ABCD", 6, "==" ), string( "==ABCD" ) );
	ASSERT_EQ( pad_r( "ABCD", 8, ":", '*' ), string( ":ABCD***" ) );
	ASSERT_EQ( pad_r( "ABCD", 3, ":", '*' ), string( ":ABCD" ) );
};

TEST( TestConverts, parse_int ) {
	int result;
	const char* end;

	ASSERT_NO_THROW( end = parse_int( result, nullptr, 255 ) );
	ASSERT_EQ( end, nullptr );

	ASSERT_NO_THROW( end = parse_int( result, "123", 255 ) );
	ASSERT_EQ( result, 123 );
	ASSERT_EQ( *end, '\0' );
	ASSERT_EQ( * parse_int( result, "456789", 3 ), '7' );
	ASSERT_EQ( result, 456 );
	ASSERT_EQ( * parse_int( result, "12.34", 5 ), '.' );
	ASSERT_EQ( result, 12 );
};

TEST( TestConverts, parse_all ) {
	int result;
	const char* end;

	ASSERT_NO_THROW( end = parse_all( result, nullptr ) );
	ASSERT_EQ( end, nullptr );

	ASSERT_NO_THROW( end = parse_all( result, "123" ) );
	ASSERT_EQ( result, 123 );
	ASSERT_EQ( * parse_all( result, "456789" ), '\0' );
	ASSERT_EQ( result, 456789 );
	ASSERT_EQ( * parse_all( result, "12.34" ), '.' );
	ASSERT_EQ( result, 12 );
};

// 整数, 全默认参数, 结果不应有小数部分
TEST( TestConverts, ResultFromIntHasNoFrac ) {
	unsigned char  uc8Input = 123;
	char           chrInput = 123;
	int8_t         i08Input = 123;
	int16_t        i16Input = 123;
	int32_t        i32Input = 123;
	int64_t        i64Input = 123;
	uint32_t       u32Input = 123;
	uint64_t       u64Input = 123;
	string         strExpect = "123";

	EXPECT_EQ( fmt<int>( uc8Input ), strExpect );
	EXPECT_EQ( fmt<int>( chrInput ), strExpect );
	EXPECT_EQ( fmt<int>( i08Input ), strExpect );
	EXPECT_EQ( fmt( i16Input ), strExpect );
	EXPECT_EQ( fmt( i32Input ), strExpect );
	EXPECT_EQ( fmt( i64Input ), strExpect );
	EXPECT_EQ( fmt( u32Input ), strExpect );
	EXPECT_EQ( fmt( u64Input ), strExpect );
};

// 负整数, 全默认参数, 是否正确?
TEST( TestConverts, ResultFromNegInt ) {
	char           chrInput = -123;
	int8_t         i08Input = -123;
	int16_t        i16Input = -123;
	int32_t        i32Input = -123;
	int64_t        i64Input = -123;
	string         strExpect = "-123";

	EXPECT_EQ( fmt<int>( chrInput, 0 ), strExpect );
	EXPECT_EQ( fmt<int>( i08Input, 0 ), strExpect );
	EXPECT_EQ( fmt( i16Input, 0 ), strExpect );
	EXPECT_EQ( fmt( i32Input, 0 ), strExpect );
	EXPECT_EQ( fmt( i64Input, 0 ), strExpect );
};

// 正整数, 指定宽度
TEST( TestConverts, IntWithWidth ) {
	unsigned char  uc8Input = 123;
	char           chrInput = 123;
	int8_t         i08Input = 123;
	int16_t        i16Input = 123;
	int32_t        i32Input = 123;
	int64_t        i64Input = 123;
	uint32_t       u32Input = 123;
	uint64_t       u64Input = 123;
	string         strExpect = "  123";

	EXPECT_EQ( fmt<int>( uc8Input, 5 ), strExpect );
	EXPECT_EQ( fmt<int>( chrInput, 5 ), strExpect );
	EXPECT_EQ( fmt<int>( i08Input, 5 ), strExpect );
	EXPECT_EQ( fmt( i16Input, 5 ), strExpect );
	EXPECT_EQ( fmt( i32Input, 5 ), strExpect );
	EXPECT_EQ( fmt( i64Input, 5 ), strExpect );
	EXPECT_EQ( fmt( u32Input, 5 ), strExpect );
	EXPECT_EQ( fmt( u64Input, 5 ), strExpect );
};

// 正整数, 指定宽度, 指定填充
TEST( TestConverts, IntWithWidthFill ) {
	unsigned char  uc8Input = 123;
	char           chrInput = 123;
	int8_t         i08Input = 123;
	int16_t        i16Input = 123;
	int32_t        i32Input = 123;
	int64_t        i64Input = 123;
	uint32_t       u32Input = 123;
	uint64_t       u64Input = 123;
	string         strExpect = "000123";

	EXPECT_EQ( fmt<int>( uc8Input, 6, 0, 0, '0' ), strExpect );
	EXPECT_EQ( fmt<int>( chrInput, 6, 0, 0, '0' ), strExpect );
	EXPECT_EQ( fmt<int>( i08Input, 6, 0, 0, '0' ), strExpect );
	EXPECT_EQ( fmt( i16Input, 6, 0, 0, '0' ), strExpect );
	EXPECT_EQ( fmt( i32Input, 6, 0, 0, '0' ), strExpect );
	EXPECT_EQ( fmt( i64Input, 6, 0, 0, '0' ), strExpect );
	EXPECT_EQ( fmt( u32Input, 6, 0, 0, '0' ), strExpect );
	EXPECT_EQ( fmt( u64Input, 6, 0, 0, '0' ), strExpect );
};

// 正整数, 指定宽度, 指定分租, 指定填充
TEST( TestConverts, IntWithWidthGroupFill ) {
	int16_t        i16Input = 1234;
	int32_t        i32Input = 1234;
	int64_t        i64Input = 1234;
	uint32_t       u32Input = 1234;
	uint64_t       u64Input = 1234;
	string         strExpect = "001'234";

	EXPECT_EQ( fmt( i16Input, 7, 0, 3, '0' ), strExpect );
	EXPECT_EQ( fmt( i32Input, 7, 0, 3, '0' ), strExpect );
	EXPECT_EQ( fmt( i64Input, 7, 0, 3, '0' ), strExpect );
	EXPECT_EQ( fmt( u32Input, 7, 0, 3, '0' ), strExpect );
	EXPECT_EQ( fmt( u64Input, 7, 0, 3, '0' ), strExpect );
};

// 浮点数默认精度会带3位小数
TEST( TestConverts, DefaultFloatHas3Frac ) {
	float  sfInput = 123.0;
	double dfInput = 123.0;
	string strExpect = "123.000";

	EXPECT_EQ( fmt( sfInput ), strExpect );
	EXPECT_EQ( fmt( dfInput ), strExpect );
};

// 浮点数指定4位精度
TEST( TestConverts, FloatWith4Frac ) {
	float    sf_Input = 123;
	double   df_Input = 123;
	string   strExpect = "123.0000";

	EXPECT_EQ( fmt( sf_Input, 0, 4 ), strExpect );
	EXPECT_EQ( fmt( df_Input, 0, 4 ), strExpect );
};

// 浮点数, 宽15位, 精度3位, 分组4位, 填'0'
TEST( TestConverts, FloatWidth15Frac3Group4Fill0 ) {
	float    sf_Input = 123456.789;
	double   df_Input = 123456789.2345;

	EXPECT_EQ( fmt( sf_Input, 15, 3, 4, '0' ),
			   string( "000012'3456.789" ) );
	EXPECT_EQ( fmt( df_Input, 15, 3, 4, '0' ),
			   string( "1'2345'6789.235" ) );
};

// 指定分组符
TEST( TestConverts, VarysTypeWithSeparaterChar ) {
	int8_t         i08Input = std::numeric_limits<int8_t>::max();
	uint8_t        ui8Input = std::numeric_limits<uint8_t>::max();
	int16_t        i16Input = std::numeric_limits<int16_t>::max();
	int32_t        i32Input = std::numeric_limits<int32_t>::max();
	int64_t        i64Input = std::numeric_limits<int64_t>::max();
	uint16_t       u16Input = std::numeric_limits<uint16_t>::max();
	uint32_t       u32Input = std::numeric_limits<uint32_t>::max();
	uint64_t       u64Input = std::numeric_limits<uint64_t>::max();
	float    sf_Input = 123456.789;
	double   df_Input = 123456789.2345;

	EXPECT_EQ( fmt<int>( i08Input, 10, 2, 4, '0', ',' ),
			   string( "0000000127" ) );
	EXPECT_EQ( fmt<int>( ui8Input, 10, 2, 4, '0', ',' ),
			   string( "0000000255" ) );
	EXPECT_EQ( fmt( i16Input, 10, 2, 4, '0', ',' ),
			   string( "00003,2767" ) );
	EXPECT_EQ( fmt( i32Input, 15, 2, 4, '0', '\'' ),
			   string( "00021'4748'3647" ) );
	EXPECT_EQ( fmt( i64Input, 25, 2, 4, '0', '\'' ),
			   string( "00922'3372'0368'5477'5807" ) );
	EXPECT_EQ( fmt( u16Input, 10, 2, 4, ' ', '\'' ),
			   string( "    6'5535" ) );
	EXPECT_EQ( fmt( u32Input, 15, 2, 4, '0', '\'' ),
			   string( "00042'9496'7295" ) );
	EXPECT_EQ( fmt( u64Input, 24, 3, 4, '0', '|' ),
			   string( "1844|6744|0737|0955|1615" ) );
	EXPECT_EQ( fmt( sf_Input, 15, 3, 4, '0', '`' ),
			   string( "000012`3456.789" ) );
	EXPECT_EQ( fmt( df_Input, 18, 3, 4, '0', '"' ),
			   string( "0001\"2345\"6789.235" ) );
};

// 浮点数会四舍五入到指定精度
TEST( TestConverts, RoundFloat ) {
	float    sf_Input = 1.23451;
	double   df_Input = 1.2345654321;

	EXPECT_EQ( fmt( sf_Input, 0, 3 ), string( "1.235" ) );
	EXPECT_EQ( fmt( sf_Input, 0, 4 ), string( "1.2345" ) );
	EXPECT_EQ( fmt( df_Input, 0, 5 ), string( "1.23457" ) );
	EXPECT_EQ( fmt( df_Input, 0, 6 ), string( "1.234565" ) );
	EXPECT_EQ( fmt( 11113.3350001, 11, 2, 4 ), string( "  1'1113.34" ) );
};

// 超宽全部为"***"
TEST( TestConverts, OverWide ) {
	int32_t  i32Input = 123456789;
	float    sf_Input = 12345;
	double   df_Input = 12345;
	string   strExpect = "**********";

	EXPECT_EQ( fmt( i32Input, 10, 4, 4, '0' ), strExpect );
	EXPECT_EQ( fmt( sf_Input, 10, 4, 4, '0' ), strExpect );
	EXPECT_EQ( fmt( df_Input, 10, 4, 4, '0' ), strExpect );
};

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
