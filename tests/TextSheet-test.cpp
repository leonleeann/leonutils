#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "leonutils/TextSheet.hpp"

using namespace leon_utl;
using namespace std;

TEST( TestTextSheet, withIntegers ) {
	TextSheet_t testee { "文本表格", 9, 0 };
	testee.clear();

	testee.addCol( "第1列" );
	testee.addCol( "第2列" );
	testee.addCol( "第3列" );

	testee.addRow( "第1行" );
	testee.addRow( "第2行" );

	testee.skeleton();

	ASSERT_EQ( testee.cols(), 3 );
	ASSERT_EQ( testee.rows(), 2 );

	testee.fill( 0, 0, 11 );
	testee.fill( 0, 1, 12 );
	testee.fill( 0, 2, 13 );

	testee.fill( 1, 0, 21 );
	testee.fill( 1, 1, 22 );
	testee.fill( 1, 2, 23 );

	str_t result = testee.makeUtf8();
	str_t refer = R"TXT(
┌──────────┬─────────┬─────────┬─────────┐
│ 文本表格 │  第1列  │  第2列  │  第3列  │
├──────────┼─────────┼─────────┼─────────┤
│ 第1行    │      11 │      12 │      13 │
├──────────┼─────────┼─────────┼─────────┤
│ 第2行    │      21 │      22 │      23 │
└──────────┴─────────┴─────────┴─────────┘)TXT";
	// std::cerr << result;
	ASSERT_EQ( result, refer );
};

TEST( TestTextSheet, withDoubles ) {
	TextSheet_t testee { "浮点数", 10, 2 };
	testee.clear();

	testee.addCol( "第1列" );
	testee.addCol( "第2列" );
	testee.addCol( "第3列" );

	testee.addRow( "第1行" );
	testee.addRow( "第2行" );

	testee.skeleton();

	ASSERT_EQ( testee.cols(), 3 );
	ASSERT_EQ( testee.rows(), 2 );

	testee.fill( 0, 0, 1000.1 );
	testee.fill( 0, 1, 1000.2 );
	testee.fill( 0, 2, 1000.3 );

	testee.fill( 1, 0, 2000.1 );
	testee.fill( 1, 1, 2000.2 );
	testee.fill( 1, 2, 2000.3 );

	str_t result = testee.makeUtf8();
	str_t refer = R"TXT(
┌────────┬──────────┬──────────┬──────────┐
│ 浮点数 │  第1列   │  第2列   │  第3列   │
├────────┼──────────┼──────────┼──────────┤
│ 第1行  │ 1,000.10 │ 1,000.20 │ 1,000.30 │
├────────┼──────────┼──────────┼──────────┤
│ 第2行  │ 2,000.10 │ 2,000.20 │ 2,000.30 │
└────────┴──────────┴──────────┴──────────┘)TXT";
	// std::cerr << result;
	ASSERT_EQ( result, refer );
};

TEST( TestTextSheet, hybirds ) {
	TextSheet_t testee { "混杂内容", 8, 1 };
	testee.clear();

	testee.addCol( "第1列" );
	testee.addCol( "第2列" );

	testee.addRow( "浮点值" );
	testee.addRow( "整数值" );
	testee.addRow( "字符串" );

	testee.skeleton();

	ASSERT_EQ( testee.cols(), 2 );
	ASSERT_EQ( testee.rows(), 3 );

	testee.fill( 0, 0, 1.1 );
	testee.fill( 0, 1, 1.2 );
	testee.fill( 1, 0, 201 );
	testee.fill( 1, 1, 202 );
	testee.fillStr( 2, 0, "abc" );
	testee.fillStr( 2, 1, "def" );

	str_t result = testee.makeUtf8();
	str_t refer = R"TXT(
┌──────────┬────────┬────────┐
│ 混杂内容 │ 第1列  │ 第2列  │
├──────────┼────────┼────────┤
│ 浮点值   │    1.1 │    1.2 │
├──────────┼────────┼────────┤
│ 整数值   │    201 │    202 │
├──────────┼────────┼────────┤
│ 字符串   │ abc    │ def    │
└──────────┴────────┴────────┘)TXT";
	// std::cerr << result;
	ASSERT_EQ( result, refer );
};

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
