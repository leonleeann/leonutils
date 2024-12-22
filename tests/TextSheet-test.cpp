#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "leonutils/TextSheet.hpp"

using namespace leon_utl;
using namespace std;

TEST( TestTextSheet, withNumbers ) {
	TextSheet_t testee { "文本表格" };
	testee.clear();

	testee.addCol( "第1列",  9, 0, 3 );
	testee.addCol( "第2列", 10, 1 );
	testee.addCol( "第3列", 11, 2, 4, '\'' );

	testee.addRow( "第1行" );
	testee.addRow( "第2行" );

	testee.skeleton();

	ASSERT_EQ( testee.cols(), 3 );
	ASSERT_EQ( testee.rows(), 2 );

	testee.fill( 0, 0, 1111 );
	testee.fill( 0, 1, 1112 );
	testee.fill( 0, 2, 11113.33500001 );

	testee.fill( 1, 0, 22221 );
	testee.fill( 1, 1, 22222 );
	testee.fill( 1, 2, 2345678 );

	str_t result = testee.makeUtf8();
	str_t refer = R"TXT(
┌──────────┬─────────┬──────────┬───────────┐
│ 文本表格 │  第1列  │  第2列   │   第3列   │
├──────────┼─────────┼──────────┼───────────┤
│ 第1行    │   1,111 │   1112.0 │ 1'1113.34 │
├──────────┼─────────┼──────────┼───────────┤
│ 第2行    │  22,221 │  22222.0 │ ********* │
└──────────┴─────────┴──────────┴───────────┘)TXT";
	// std::cerr << result;
	ASSERT_EQ( result, refer );
};

TEST( TestTextSheet, hiding1Col ) {
	TextSheet_t testee { "隐藏单列" };
	testee.clear();

	testee.addCol( "第1列", 7, 0 );
	testee.addCol( "第2列", 7, 0, 0, ',', false );
	testee.addCol( "第3列", 7, 0 );

	testee.addRow( "第1行" );
	testee.addRow( "第2行" );
	testee.addRow( "第3行" );

	testee.skeleton();

	ASSERT_EQ( testee.cols(), 3 );
	ASSERT_EQ( testee.rows(), 3 );

	testee.fill( 0, 0, 1111 );
	testee.fill( 0, 1, 1112 );
	testee.fill( 0, 2, 1113 );

	testee.fill( 1, 0, 2221 );
	testee.fill( 1, 1, 2222 );
	testee.fill( 1, 2, 2223 );

	testee.fill( 2, 0, 3331 );
	testee.fill( 2, 1, 3332 );
	testee.fill( 2, 2, 3333 );

	str_t result = testee.makeUtf8();
	str_t refer = R"TXT(
┌──────────┬───────┬───────┐
│ 隐藏单列 │ 第1列 │ 第3列 │
├──────────┼───────┼───────┤
│ 第1行    │  1111 │  1113 │
├──────────┼───────┼───────┤
│ 第2行    │  2221 │  2223 │
├──────────┼───────┼───────┤
│ 第3行    │  3331 │  3333 │
└──────────┴───────┴───────┘)TXT";
	// std::cerr << result;
	ASSERT_EQ( result, refer );
};

TEST( TestTextSheet, hiding1Row ) {
	TextSheet_t testee { "隐藏单行" };
	testee.clear();

	testee.addCol( "第1列", 7, 0 );
	testee.addCol( "第2列", 7, 0 );
	testee.addCol( "第3列", 7, 0 );

	testee.addRow( "第1行" );
	testee.addRow( "第2行", false );
	testee.addRow( "第3行" );

	testee.skeleton();

	ASSERT_EQ( testee.cols(), 3 );
	ASSERT_EQ( testee.rows(), 3 );

	testee.fill( 0, 0, 1111 );
	testee.fill( 0, 1, 1112 );
	testee.fill( 0, 2, 1113 );

	testee.fill( 1, 0, 2221 );
	testee.fill( 1, 1, 2222 );
	testee.fill( 1, 2, 2223 );

	testee.fill( 2, 0, 3331 );
	testee.fill( 2, 1, 3332 );
	testee.fill( 2, 2, 3333 );

	str_t result = testee.makeUtf8();
	str_t refer = R"TXT(
┌──────────┬───────┬───────┬───────┐
│ 隐藏单行 │ 第1列 │ 第2列 │ 第3列 │
├──────────┼───────┼───────┼───────┤
│ 第1行    │  1111 │  1112 │  1113 │
├──────────┼───────┼───────┼───────┤
│ 第3行    │  3331 │  3332 │  3333 │
└──────────┴───────┴───────┴───────┘)TXT";
	// std::cerr << result;
	ASSERT_EQ( result, refer );
};

TEST( TestTextSheet, hidingRowCol ) {
	TextSheet_t testee { "隐藏行列" };
	testee.clear();

	testee.addCol( "第1列", 7, 0 );
	testee.addCol( "第2列", 7, 0 );
	testee.addCol( "第3列", 7, 0 );

	testee.addRow( "第1行" );
	testee.addRow( "第2行" );
	testee.addRow( "第3行" );

	testee.skeleton();

	ASSERT_EQ( testee.cols(), 3 );
	ASSERT_EQ( testee.rows(), 3 );

	testee.fill( 0, 0, 1111 );
	testee.fill( 0, 1, 1112 );
	testee.fill( 0, 2, 1113 );

	testee.fill( 1, 0, 2221 );
	testee.fill( 1, 1, 2222 );
	testee.fill( 1, 2, 2223 );

	testee.fill( 2, 0, 3331 );
	testee.fill( 2, 1, 3332 );
	testee.fill( 2, 2, 3333 );

	str_t result = testee.makeUtf8();
	str_t refer = R"TXT(
┌──────────┬───────┬───────┬───────┐
│ 隐藏行列 │ 第1列 │ 第2列 │ 第3列 │
├──────────┼───────┼───────┼───────┤
│ 第1行    │  1111 │  1112 │  1113 │
├──────────┼───────┼───────┼───────┤
│ 第2行    │  2221 │  2222 │  2223 │
├──────────┼───────┼───────┼───────┤
│ 第3行    │  3331 │  3332 │  3333 │
└──────────┴───────┴───────┴───────┘)TXT";
	// std::cerr << result;
	ASSERT_EQ( result, refer );
};

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
