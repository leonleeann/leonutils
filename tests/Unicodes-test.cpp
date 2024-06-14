#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "leonutils/Unicodes.hpp"

using namespace leon_utl;
using namespace std;

TEST( TestUnicodes, wstringWithU8 ) {
	string	u8 = "汉字";
	wstring	ws = L"汉字";
	ASSERT_EQ( u8_2_ws( u8 ), ws );
	ASSERT_EQ( ws_2_u8( ws ), u8 );
};

TEST( TestUnicodes, countChinese ) {
	string	u8 = "3个汉字";
	wstring	ws = L"3个汉字";
	ASSERT_EQ( chinese_chars( u8 ), 3 );
	ASSERT_EQ( chinese_chars( ws ), 3 );

	u8 = "四个汉字";
	ws = L"四个汉字";
	ASSERT_EQ( chinese_chars( u8 ), 4 );
	ASSERT_EQ( chinese_chars( ws ), 4 );
};

TEST( TestUnicodes, countWidth ) {
	string	u8 = "3个汉字";
// 	wstring	ws = L"3个汉字";
	ASSERT_EQ( displ_width( u8 ), 7 );
// 	ASSERT_EQ( displ_width( ws ), 7 );

	u8 = "四个汉字abc";
// 	ws = L"四个汉字abc";
	ASSERT_EQ( displ_width( u8 ), 11 );
// 	ASSERT_EQ( displ_width( ws ), 11 );
};

TEST( TestUnicodes, adaptWidth ) {
	string	u8 = "3个汉字";
	wstring	ws = L"3个汉字";
	wstring	result = L" 3个汉字    ";
	ASSERT_EQ( adapt_width( 12, u8 ), result );
// 	ASSERT_EQ( adapt_width( 12, ws ), result );

	u8 = "四个汉字";
	ws = L"四个汉字";
	result = L" 四个汉字   ";
	ASSERT_EQ( adapt_width( 12, u8 ), result );
// 	ASSERT_EQ( displ_width( 12, ws ), result );
};

TEST( TestUnicodes, adaptCenter ) {
	string	u8 = "3个汉字";
	wstring	ws = L"3个汉字";
	wstring	result = L"  3个汉字   ";
	ASSERT_EQ( adapt_centr( 12, u8 ), result );
	ASSERT_EQ( adapt_centr( 12, ws ), result );

	u8 = "四个汉字";
	ws = L"四个汉字";
	result = L"  四个汉字  ";
	ASSERT_EQ( adapt_centr( 12, u8 ), result );
	ASSERT_EQ( adapt_centr( 12, ws ), result );

	u8 = "很多汉字不需要填充";
	ws = L"很多汉字不需要填充";
	result = L"很多汉字不需要填充";
	ASSERT_EQ( adapt_centr( 10, u8 ), result );
	ASSERT_EQ( adapt_centr( 10, ws ), result );
};

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
