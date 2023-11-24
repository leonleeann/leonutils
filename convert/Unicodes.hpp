#pragma once
#include <string>

namespace leon_utl {

// 制表符-细横线	"─":0x2500
static constexpr wchar_t TAB_HOR_LINE = 0x2500;
// 制表符-细竖线	"│":0x2502
static constexpr wchar_t TAB_VER_LINE = 0x2502;
// 制表符-左上角	"┌":0x250c
static constexpr wchar_t TAB_TOP_LEFT = 0x250C;
// 制表符-右上角	"┐":0x2510
static constexpr wchar_t TAB_TOP_RGHT = 0x2510;
// 制表符-左下角	"└":0x2514
static constexpr wchar_t TAB_BTM_LEFT = 0x2514;
// 制表符-右下角	"┘":0x2518
static constexpr wchar_t TAB_BTM_RGHT = 0x2518;
// 制表符-左丁字	"├":0x251c
static constexpr wchar_t TAB_LEFT_SEP = 0x251C;
// 制表符-右丁字	"┤":0x2524
static constexpr wchar_t TAB_RGHT_SEP = 0x2524;
// 制表符-顶丁字	"┬":0x252c
static constexpr wchar_t TAB_HEAD_SEP = 0x252C;
// 制表符-底丁字	"┴":0x2534
static constexpr wchar_t TAB_FOOT_SEP = 0x2534;
// 制表符-十字线	"┼":0x253c
static constexpr wchar_t TAB_CROS_SEP = 0x253C;

// GB18030转UTF8
std::string		gb_2_u8( const char* gb_str );
std::string		gb_2_u8( const std::string& gb_str );

// GB18030转wstring
std::wstring	gb_2_ws( const char* gb_str );
std::wstring	gb_2_ws( const std::string& gb_str );

// wstring转UTF8
std::string		ws_2_u8( const wchar_t* );
std::string		ws_2_u8( const std::wstring& );

// UTF8转wstring
std::wstring	u8_2_ws( const char* utf8_str );
std::wstring	u8_2_ws( const std::string& utf8_str );

// 在一个wstring中计数汉字个数
int chinese_chars( const std::wstring& );

// 在一个UTF8串中计数汉字个数
int chinese_chars( const char* utf8_str );
int chinese_chars( const std::string& utf8_str );

// 按照"字母数字占1个位宽,汉字占2个位宽"的原则,计算一段文本的显示宽度
int	displ_width( const std::string& utf8_str );
int	displ_width( const std::wstring& ws_str );

// 按照"字母数字占1个位宽,汉字占2个位宽"的原则,将一段文本串适配到期望显示宽度的wstring.不足加空格
std::wstring	adapt_width( int expect_width, const std::string& utf8_text );
std::wstring	adapt_width( int expect_width, const std::wstring& wstr_text );

// 同上,但是对中
std::wstring	adapt_centr( int expect_width, const std::wstring& wstr_text );
std::wstring	adapt_centr( int expect_width, const std::string& utf8_text );

}; //namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
