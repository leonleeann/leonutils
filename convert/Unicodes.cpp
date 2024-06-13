#include <codecvt>
#include <cstring>		// strlen, strncmp, strncpy, memset, memcpy, memmove, strerror
#include <locale>

#include "Converts.hpp"
#include "Unicodes.hpp"

using std::codecvt;
using std::codecvt_utf8;
using std::locale;
using std::use_facet;
using std::wstring_convert;

namespace leon_utl {

static constexpr wchar_t CHINESE_BEG = 0x4E00;
static constexpr wchar_t CHINESE_END = 0x9FFF;

using w2c_cvt_t = codecvt<wchar_t, char, mbstate_t>;

const locale cg_loc_gb( "zh_CN.GB18030" );

thread_local const w2c_cvt_t& crtl_cvt_gb2w = use_facet<w2c_cvt_t>( cg_loc_gb );

thread_local wstring_convert<codecvt_utf8<wchar_t> > tl_cvt_w2u8;

inline str_t gb2u8_raw( const char* gb_str_, size_t len_ ) {
	wchar_t  aw_buf[ len_ + 1 ];
	wchar_t* aw_end = aw_buf;
	mbstate_t mt = {};
	const char* gb_end = gb_str_;

	if( crtl_cvt_gb2w.in( mt, gb_str_, gb_str_ + len_,
						  gb_end, aw_buf, aw_buf + len_, aw_end ) ==
			std::codecvt_base::result::ok )
		return tl_cvt_w2u8.to_bytes( aw_buf, aw_end );
	else
		return "";
};

inline wstr_t gb2w_raw( const char* gb_str_, size_t len_ ) {
	wchar_t  aw_buf[ len_ + 1 ];
	wchar_t* aw_end = aw_buf;
	mbstate_t mt = {};
	const char* gb_end = gb_str_;

	if( crtl_cvt_gb2w.in( mt, gb_str_, gb_str_ + len_,
						  gb_end, aw_buf, aw_buf + len_, aw_end ) ==
			std::codecvt_base::result::ok )
		return aw_buf;
	else
		return L"";
};

inline str_t w2u8_raw( const wchar_t* wstr_, size_t len_ ) {
	return tl_cvt_w2u8.to_bytes( wstr_, wstr_ + len_ );
};

inline wstr_t u82w_raw( const char* cstr_, size_t len_ ) {
	return tl_cvt_w2u8.from_bytes( cstr_, cstr_ + len_ );
};

str_t gb_2_u8( const char* gb_str_ ) {
	return gb2u8_raw( gb_str_, std::strlen( gb_str_ ) );
};

str_t gb_2_u8( const str_t& gb_str_ ) {
	return gb2u8_raw( gb_str_.c_str(), gb_str_.size() );
};

wstr_t gb_2_ws( const char* gb_str_ ) {
	return gb2w_raw( gb_str_, std::strlen( gb_str_ ) );
};

wstr_t gb_2_ws( const str_t& gb_str_ ) {
	return gb2w_raw( gb_str_.c_str(), gb_str_.size() );
};

str_t ws_2_u8( const wchar_t* wstr_ ) {
	return w2u8_raw( wstr_, std::wcslen( wstr_ ) );
};

str_t ws_2_u8( const wstr_t& wstr_ ) {
	return w2u8_raw( wstr_.c_str(), wstr_.size() );
};

wstr_t u8_2_ws( const char* cstr_ ) {
	return u82w_raw( cstr_, std::strlen( cstr_ ) );
};

wstr_t u8_2_ws( const str_t& u8_str_ ) {
	return u82w_raw( u8_str_.c_str(), u8_str_.size() );
};

int chinese_chars( const wstr_t& wstr_ ) {
	int cnt = 0;
	for( wchar_t wc : wstr_ )
		cnt += ( wc >= CHINESE_BEG && wc <= CHINESE_END );

	return cnt;
};

int chinese_chars( const char* u8_str_ ) {
	wstr_t ws = u82w_raw( u8_str_, std::strlen( u8_str_ ) );
	return chinese_chars( ws );
};

int chinese_chars( const str_t& u8_str_ ) {
	wstr_t ws = u82w_raw( u8_str_.c_str(), u8_str_.size() );
	return chinese_chars( ws );
};

int displ_width( const str_t& u8_ ) {
	return displ_width( u8_2_ws( u8_ ) );
};

int displ_width( const wstr_t& ws_ ) {
	return static_cast<int>( ws_.size() ) + chinese_chars( ws_ );
};

wstr_t adapt_width( int exp_w_, const wstr_t& wtxt_ ) {
	int chinese_cnt = chinese_chars( wtxt_ );
	int other_chars = static_cast<int>( wtxt_.size() ) - chinese_cnt;

	// 每个汉字要占两个显示宽度,所以要乘2
	int disp_width = chinese_cnt * 2 + other_chars;
	int width_diff = exp_w_ - disp_width;
	if( width_diff <= 0 )
		return wtxt_;

	wstr_t widen = L' ' + wtxt_ + wstr_t( width_diff, L' ' );
	return widen.substr( 0, chinese_cnt + other_chars + width_diff );
};

wstr_t adapt_width( int exp_w_, const str_t& u8_txt_ ) {
	return adapt_width( exp_w_, u8_2_ws( u8_txt_ ) );
};

wstr_t adapt_centr( int exp_w_, const wstr_t& w_txt_ ) {
	int chinese_cnt = chinese_chars( w_txt_ );
	int other_chars = static_cast<int>( w_txt_.size() ) - chinese_cnt;

	// 每个汉字要占两个显示宽度,所以要乘2
	int disp_width = chinese_cnt * 2 + other_chars;
	int width_diff = exp_w_ - disp_width;
	if( width_diff <= 0 )
		return w_txt_;

	int left_pad = width_diff / 2;
	int rght_pad = width_diff - left_pad;

	wstr_t widen = wstr_t( left_pad, L' ' ) + w_txt_ + wstr_t( rght_pad, L' ' );
	return widen.substr( 0, chinese_cnt + other_chars + width_diff );
};

wstr_t adapt_centr( int exp_w_, const str_t& u8_txt_ ) {
	return adapt_centr( exp_w_, u8_2_ws( u8_txt_ ) );
};

}; //namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
