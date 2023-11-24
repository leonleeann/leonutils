#include <cmath>		// abs, ceil, floor, isnan, log, log10, log2, pow, round, sqrt
#include <cstring>		// strlen, strncmp, strncpy, memset, memcpy, memmove, strerror
#include <type_traits>  // is_floating_point_v, std::is_signed_v, std::is_unsigned_v

#include "Chrono.hpp"

namespace leon_utl {

using std::string;

// ts_secs_t & ts_nsec_t 都是有符号的64位整数,才能继续玩儿!
// static_assert( ts_nsec_t( -1 ) < ts_secs_t( 0 ) );
// static_assert( ts_nsec_t( -1 ) < ts_nsec_t( 0 ) );
static_assert( std::is_signed_v<ts_secs_t>, "timespec::tv_sec not a signed int" );
static_assert( std::is_signed_v<ts_nsec_t>, "timespec::tv_nsec not a signed int" );
static_assert( sizeof( timespec::tv_sec ) == 8, "timespec::tv_sec not a 64 bits int" );
static_assert( sizeof( timespec::tv_nsec ) == 8, "timespec::tv_nsec not a 64 bits int" );

thread_local const std::time_put<char>& tl_time_formater
	= std::use_facet<std::time_put<char>>( std::locale( "C" ) );

string format_time( const tm& tm_, const char* fmt_ ) {
	std::ostringstream oss;
	leon_utl::tl_time_formater.put( oss, oss, ' ', &tm_,
									fmt_, fmt_ + std::strlen( fmt_ ) );
	return oss.str();
};

string format_time( const std::time_t time_, const char* fmt_ ) {
	tm tm1;
	localtime_r( &time_, &tm1 );
	return format_time( tm1, fmt_ );
};

template <typename C, typename D>
string format_time( const time_point<C, D> tp_, const char* fmt_ ) {
	return format_time( C::to_time_t( std::chrono::floor<seconds>( tp_ ) ), fmt_ );
};

template <typename C, typename D>
string format_time( const time_point<C, D> tp_, size_t prec_,
					const char* fmt_ ) {
	// 精度最多9位,也即纳秒
	if( prec_ > 9 )
		prec_ = 9;

	auto tp1 = std::chrono::floor<seconds>( tp_ );
	string strResult = format_time( tp1, fmt_ );

	if( prec_ > 0 ) {
		// 只能是截断,不能是四舍五入或者任何收入！因为前面的结果已经定了，不能进位了
		nanoseconds::rep ns = duration_cast<nanoseconds>( tp_ - tp1 ).count()
							  / std::pow( 10., 9 - prec_ );
		strResult += '.' + leon_utl::format( ns, prec_, 0, 0, '0' );
	}
	return strResult;
};

template <typename C, typename D>
string format_date( const time_point<C, D> tp_ ) {
	return format_time( tp_, DATE_FORMAT );
};

template <typename R, typename P>
string format_dura( const duration<R, P> dura_ ) {
	auto ss = duration_cast<seconds>( dura_ ).count();
	auto dd = ss / 86400;
	ss %= 86400;
	auto hh = ss / 3600;
	ss %= 3600;
	auto mm = ss / 60;
	ss %= 60;

	bool append = false;
	std::ostringstream o;
	if( ( append = dd > 0 ) )
		o << dd << "D ";
	if( ( append = append || hh > 0 ) )
		o << hh << "H ";
	if( ( append = append || mm > 0 ) )
		o << mm << "M ";
	o << ss << "S";
	return o.str();
};

template <typename R, typename P>
string format_secs( const duration<R, P> dura_ ) {
	double ms = duration_cast<milliseconds>( dura_ ).count();
	return leon_utl::format( ms / 1000, 0, 3 ) + "s";
};

SysTime_t make_sys_time( int y_, int mo_, int d_,
						 int h_, int mi_, int s_,
						 int ms_, int us_, int ns_ ) {
	tm tm1 = {};
	tm1.tm_year = y_ - 1900;
	tm1.tm_mon  = mo_ - 1;
	tm1.tm_mday = d_;
	tm1.tm_hour = h_;
	tm1.tm_min  = mi_;
	tm1.tm_sec  = s_;

	return system_clock::from_time_t( mktime( &tm1 ) )
		   + milliseconds( ms_ ) + microseconds( us_ ) + nanoseconds( ns_ );
};

void delay( steady_clock::duration dura, steady_clock::time_point from ) {
	auto time_out = from + dura;
	while( steady_clock::now() < time_out )
		;
};

bool valid_date( const string& str_ ) {
// 把字符串转成日期，再转回来，如果一致就是有效的日期
	return format_date( parse_date<SysTime_t::duration>( str_.c_str() ) ) == str_;
};

// 显式地实例化一下,以免链接时找不到
template string format_time( const time_point<system_clock, hours>, const char* );
template string format_time( const time_point<system_clock, minutes>, const char* );
template string format_time( const time_point<system_clock, seconds>, const char* );
template string format_time( const time_point<system_clock, milliseconds>, const char* );
template string format_time( const time_point<system_clock, microseconds>, const char* );
template string format_time( const time_point<system_clock, nanoseconds>, const char* );

template string format_time( const time_point<system_clock, hours>, size_t, const char* );
template string format_time( const time_point<system_clock, minutes>, size_t, const char* );
template string format_time( const time_point<system_clock, seconds>, size_t, const char* );
template string format_time( const time_point<system_clock, milliseconds>, size_t, const char* );
template string format_time( const time_point<system_clock, microseconds>, size_t, const char* );
template string format_time( const time_point<system_clock, nanoseconds>, size_t, const char* );

template string format_date( const time_point<system_clock, hours> );
template string format_date( const time_point<system_clock, minutes> );
template string format_date( const time_point<system_clock, seconds> );
template string format_date( const time_point<system_clock, milliseconds> );
template string format_date( const time_point<system_clock, microseconds> );
template string format_date( const time_point<system_clock, nanoseconds> );

template string format_dura( const hours );
template string format_dura( const minutes );
template string format_dura( const seconds );
template string format_dura( const milliseconds );
template string format_dura( const microseconds );
template string format_dura( const nanoseconds );

template string format_secs( const hours );
template string format_secs( const minutes );
template string format_secs( const seconds );
template string format_secs( const milliseconds );
template string format_secs( const microseconds );
template string format_secs( const nanoseconds );

// template seconds make_time<seconds>( int, int, int, int, int, int, int, int, int );

}; //namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
