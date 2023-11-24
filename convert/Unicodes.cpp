#include <codecvt>
#include <cstring>		// strlen, strncmp, strncpy, memset, memcpy, memmove, strerror
#include <locale>

#include "Converts.hpp"
#include "Unicodes.hpp"

using std::codecvt;
using std::codecvt_utf8;
using std::locale;
using std::string;
using std::use_facet;
using std::wstring;
using std::wstring_convert;

namespace leon_utl {

static constexpr wchar_t CHINESE_BEG = 0x4E00;
static constexpr wchar_t CHINESE_END = 0x9FFF;

typedef codecvt< wchar_t, char, mbstate_t > w2c_cvt_t;

const locale cg_locGB( "zh_CN.GB18030" );

thread_local const w2c_cvt_t& crtl_cvtGB2W = use_facet< w2c_cvt_t >( cg_locGB );

thread_local wstring_convert< codecvt_utf8< wchar_t > > tl_cvtW2U8;

inline string gb2u8_raw( const char* gb_str, size_t len ) {
	wchar_t  awBuf[ len + 1 ];
	wchar_t* awEnd = awBuf;
	mbstate_t mt = {};
	const char* gbEnd = gb_str;

	if( crtl_cvtGB2W.in( mt, gb_str, gb_str + len,
						 gbEnd, awBuf, awBuf + len, awEnd ) ==
			std::codecvt_base::result::ok )
		return tl_cvtW2U8.to_bytes( awBuf, awEnd );
	else
		return "";
};

inline wstring gb2w_raw( const char* gb_str, size_t len ) {
	wchar_t  awBuf[ len + 1 ];
	wchar_t* awEnd = awBuf;
	mbstate_t mt = {};
	const char* gbEnd = gb_str;

	if( crtl_cvtGB2W.in( mt, gb_str, gb_str + len,
						 gbEnd, awBuf, awBuf + len, awEnd ) ==
			std::codecvt_base::result::ok )
		return awBuf;
	else
		return L"";
};

inline string w2u8_raw( const wchar_t* w_str, size_t len ) {
	return tl_cvtW2U8.to_bytes( w_str, w_str + len );
};

inline wstring u82w_raw( const char* c_str, size_t len ) {
	return tl_cvtW2U8.from_bytes( c_str, c_str + len );
};

string gb_2_u8( const char* gb_str ) {
	return gb2u8_raw( gb_str, std::strlen( gb_str ) );
};

string gb_2_u8( const string& gb_str ) {
	return gb2u8_raw( gb_str.c_str(), gb_str.size() );
};

wstring gb_2_ws( const char* gb_str ) {
	return gb2w_raw( gb_str, std::strlen( gb_str ) );
};

wstring gb_2_ws( const string& gb_str ) {
	return gb2w_raw( gb_str.c_str(), gb_str.size() );
};

string ws_2_u8( const wchar_t* w_str ) {
	return w2u8_raw( w_str, std::wcslen( w_str ) );
};

string ws_2_u8( const wstring& w_str ) {
	return w2u8_raw( w_str.c_str(), w_str.size() );
};

wstring u8_2_ws( const char* c_str ) {
	return u82w_raw( c_str, std::strlen( c_str ) );
};

wstring u8_2_ws( const std::string& u8_str ) {
	return u82w_raw( u8_str.c_str(), u8_str.size() );
};

int chinese_chars( const wstring& w_str_ ) {
	int cnt = 0;
	for( wchar_t wc : w_str_ )
		cnt += ( wc >= CHINESE_BEG && wc <= CHINESE_END );

	return cnt;
};

int chinese_chars( const char* u8_str_ ) {
	wstring ws = u82w_raw( u8_str_, std::strlen( u8_str_ ) );
	return chinese_chars( ws );
};

int chinese_chars( const string& u8_str_ ) {
	wstring ws = u82w_raw( u8_str_.c_str(), u8_str_.size() );
	return chinese_chars( ws );
};

int displ_width( const string& u8_ ) {
	return displ_width( u8_2_ws( u8_ ) );
};

int displ_width( const wstring& ws_ ) {
	return static_cast<int>( ws_.size() ) + chinese_chars( ws_ );
};

wstring adapt_width( int exp_w_, const wstring& wtxt_ ) {
	int chinese_cnt = chinese_chars( wtxt_ );
	int other_chars = static_cast<int>( wtxt_.size() ) - chinese_cnt;

	// 每个汉字要占两个显示宽度,所以要乘2
	int disp_width = chinese_cnt * 2 + other_chars;
	int width_diff = exp_w_ - disp_width;
	if( width_diff <= 0 )
		return wtxt_;

	wstring widen = L' ' + wtxt_ + wstring( width_diff, L' ' );
	return widen.substr( 0, chinese_cnt + other_chars + width_diff );
};

wstring adapt_width( int exp_w_, const string& u8_txt_ ) {
	return adapt_width( exp_w_, u8_2_ws( u8_txt_ ) );
};

wstring adapt_centr( int exp_w_, const wstring& w_txt_ ) {
	int chinese_cnt = chinese_chars( w_txt_ );
	int other_chars = static_cast<int>( w_txt_.size() ) - chinese_cnt;

	// 每个汉字要占两个显示宽度,所以要乘2
	int disp_width = chinese_cnt * 2 + other_chars;
	int width_diff = exp_w_ - disp_width;
	if( width_diff <= 0 )
		return w_txt_;

	int left_pad = width_diff / 2;
	int rght_pad = width_diff - left_pad;

	wstring widen = wstring( left_pad, L' ' ) + w_txt_ + wstring( rght_pad, L' ' );
	return widen.substr( 0, chinese_cnt + other_chars + width_diff );
};

wstring adapt_centr( int exp_w_, const string& u8_txt_ ) {
	return adapt_centr( exp_w_, u8_2_ws( u8_txt_ ) );
};

}; //namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
