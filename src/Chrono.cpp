#include <cmath>		// abs, ceil, floor, isnan, log, log10, log2, pow, round, sqrt
#include <cstring>		// strlen, strncmp, strncpy, memset, memcpy, memmove, strerror
#include <type_traits>  // is_floating_point_v, std::is_signed_v, std::is_unsigned_v

#include "leonutils/Chrono.hpp"

using oss_t = std::ostringstream;

namespace leon_utl {

// ts_secs_t & ts_nsec_t 都是有符号的64位整数,才能继续玩儿!
// static_assert( ts_nsec_t( -1 ) < ts_secs_t( 0 ) );
// static_assert( ts_nsec_t( -1 ) < ts_nsec_t( 0 ) );
static_assert( std::is_signed_v<ts_secs_t>, "timespec::tv_sec not a signed int" );
static_assert( std::is_signed_v<ts_nsec_t>, "timespec::tv_nsec not a signed int" );
static_assert( sizeof( timespec::tv_sec ) == 8, "timespec::tv_sec not a 64 bits int" );
static_assert( sizeof( timespec::tv_nsec ) == 8, "timespec::tv_nsec not a 64 bits int" );

thread_local const std::time_put<char>& tl_time_formater
	= std::use_facet<std::time_put<char>>( std::locale( "C" ) );

str_t fmt_tm( const tm& tm_, const char* fmt_ ) {
	oss_t oss;
	tl_time_formater.put( oss, oss, ' ', &tm_, fmt_, fmt_ + std::strlen( fmt_ ) );
	return oss.str();
};

str_t fmt_tmt( const std::time_t time_, const char* fmt_ ) {
	tm tm1;
	localtime_r( &time_, &tm1 );
	return fmt_tm( tm1, fmt_ );
};

template <typename C, typename D>
str_t fmt( const time_point<C, D> tp_, const char* fmt_ ) {
	return fmt_tmt( C::to_time_t( std::chrono::floor<seconds>( tp_ ) ), fmt_ );
};

template <typename C, typename D>
str_t fmt( const time_point<C, D> tp_, size_t prec_, const char* fmt_ ) {
	// 精度最多9位,也即纳秒
	if( prec_ > 9 )
		prec_ = 9;

	auto tp1 = std::chrono::floor<seconds>( tp_ );
	str_t result = fmt( tp1, fmt_ );

	if( prec_ > 0 ) {
		// 只能是截断,不能是四舍五入或者任何收入！因为前面的结果已经定了，不能进位了
		ns::rep nss = duration_cast<ns>( tp_ - tp1 ).count()
					  / std::pow( 10., 9 - prec_ );
		result += '.' + fmt( nss, prec_, 0, 0, '0' );
	}
	return result;
};

template <typename C, typename D>
str_t fmt_date( const time_point<C, D> tp_ ) {
	return fmt( tp_, DATE_FORMAT );
};

template <typename R, typename P>
str_t fmt_dura( const duration<R, P> dura_ ) {
	auto ss = duration_cast<seconds>( dura_ ).count();
	auto dd = ss / 86400;
	ss %= 86400;
	auto hh = ss / 3600;
	ss %= 3600;
	auto mm = ss / 60;
	ss %= 60;

	bool append = false;
	oss_t o;
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
str_t fmt_secs( const duration<R, P> dura_ ) {
	double mss = duration_cast<ms>( dura_ ).count();
	return fmt( mss / 1000, 0, 3 ) + "s";
};

SysTime_t make_time( int y_, int mo_, int d_,
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
		   + ms( ms_ ) + us( us_ ) + ns( ns_ );
};

void delay( MonDura_t for_, MonTime_t from_ ) {
	auto deadline = from_ + for_;
	while( steady_clock::now() < deadline )
		;
};

bool valid_date( str_cr str_ ) {
// 把字符串转成日期，再转回来，如果一致就是有效的日期
	return fmt_date( parse_date( str_.c_str() ) ) == str_;
};

// 显式地实例化一下,以免链接时找不到
template str_t fmt( const time_point<system_clock, hours>, const char* );
template str_t fmt( const time_point<system_clock, minutes>, const char* );
template str_t fmt( const time_point<system_clock, ss>, const char* );
template str_t fmt( const time_point<system_clock, ms>, const char* );
template str_t fmt( const time_point<system_clock, us>, const char* );
template str_t fmt( const time_point<system_clock, ns>, const char* );

template str_t fmt( const time_point<system_clock, hours>, size_t, const char* );
template str_t fmt( const time_point<system_clock, minutes>, size_t, const char* );
template str_t fmt( const time_point<system_clock, ss>, size_t, const char* );
template str_t fmt( const time_point<system_clock, ms>, size_t, const char* );
template str_t fmt( const time_point<system_clock, us>, size_t, const char* );
template str_t fmt( const time_point<system_clock, ns>, size_t, const char* );

template str_t fmt_date( const time_point<system_clock, hours> );
template str_t fmt_date( const time_point<system_clock, minutes> );
template str_t fmt_date( const time_point<system_clock, ss> );
template str_t fmt_date( const time_point<system_clock, ms> );
template str_t fmt_date( const time_point<system_clock, us> );
template str_t fmt_date( const time_point<system_clock, ns> );

template str_t fmt_dura( const hours );
template str_t fmt_dura( const minutes );
template str_t fmt_dura( const ss );
template str_t fmt_dura( const ms );
template str_t fmt_dura( const us );
template str_t fmt_dura( const ns );

template str_t fmt_secs( const hours );
template str_t fmt_secs( const minutes );
template str_t fmt_secs( const seconds );
template str_t fmt_secs( const ms );
template str_t fmt_secs( const us );
template str_t fmt_secs( const ns );

// template seconds make_time<seconds>( int, int, int, int, int, int, int, int, int );

}; //namespace leon_utl

namespace std {

ost_t& operator<<( ost_t& os_, leon_utl::SysTime_t tp_ ) {
	return os_ << leon_utl::fmt( tp_, 0, leon_utl::NEAT_FORMAT );
};

} // namespace std

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
