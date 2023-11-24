#pragma once
// #include <cmath>        // abs, ceil, floor, isnan, log, log10, pow, round, sqrt
// #include <cstring>      // strlen, strncmp, strncpy
// #include <ctime>        // timespec
// #include <locale>       // time_put
// #include <sstream>      // ostringstream

#include "ChronoTypes.hpp"
#include "convert/Converts.hpp"

namespace leon_utl {

using ts_secs_t = decltype( timespec::tv_sec );
using ts_nsec_t = decltype( timespec::tv_nsec );

//========== 两个timespec时间结构的比较及运算 ======================================
inline bool operator > ( const timespec& ts1, const timespec& ts2 ) {
	return ( ( ts1.tv_sec  > ts2.tv_sec ) ||
			 ( ts1.tv_sec == ts2.tv_sec && ts1.tv_nsec > ts2.tv_nsec ) );
};
inline bool operator < ( const timespec& ts1, const timespec& ts2 ) {
	return ( ( ts1.tv_sec  < ts2.tv_sec ) ||
			 ( ts1.tv_sec == ts2.tv_sec && ts1.tv_nsec < ts2.tv_nsec ) );
};
inline bool operator ==( const timespec& ts1, const timespec& ts2 ) {
	return ( ts1.tv_sec == ts2.tv_sec && ts1.tv_nsec == ts2.tv_nsec );
};
inline bool operator !=( const timespec& ts1, const timespec& ts2 ) {
	return !( ts1 == ts2 );
};
inline bool operator <=( const timespec& ts1, const timespec& ts2 ) {
	return ( ts1 == ts2 || ts1 < ts2 );
};
inline bool operator >=( const timespec& ts1, const timespec& ts2 ) {
	return ( ts1 == ts2 || ts1 > ts2 );
};

constexpr ts_nsec_t NS_IN_1_SEC = 1000 * 1000 * 1000;

inline ts_nsec_t operator - ( const timespec& ts1, const timespec& ts2 ) {
	return ( ( ts1.tv_sec - ts2.tv_sec ) * NS_IN_1_SEC
			 + ts1.tv_nsec - ts2.tv_nsec );
};

inline timespec operator + ( const timespec& ts, ts_nsec_t ns ) {
	timespec tsResult = {
		ts.tv_sec + ts.tv_nsec / 1000000000 + ns / 1000000000,
		ts.tv_nsec % 1000000000
	};

	ts_nsec_t sum = ns % 1000000000 + tsResult.tv_nsec;
	tsResult.tv_sec += sum / 1000000000;
	sum = sum % 1000000000;

	// 负值修正
	tsResult.tv_nsec = ( sum + 1000000000 ) % 1000000000;
	tsResult.tv_sec += ( sum - tsResult.tv_nsec ) / 1000000000;
	return tsResult;
};

inline timespec operator - ( const timespec& ts, ts_nsec_t ns ) {
	return ts + ( - ns );
};

inline timespec& operator+=( timespec& ts, ts_nsec_t ns ) {
	ts = ts + ns;
	return ts;
};

inline timespec& operator-=( timespec& ts, ts_nsec_t ns ) {
	ts = ts - ns;
	return ts;
};

//========== 时点运算 ===========================================================

//---------- 时点对齐到整日(按本地时区算) ---------------------------
inline void floor( tm* local_tm ) {
	local_tm->tm_hour = local_tm->tm_min = local_tm->tm_sec = 0;
	return;
};

inline time_t floor( time_t local_time ) {
	tm tm1 = {};
	localtime_r( &local_time, &tm1 );
	leon_utl::floor( &tm1 );
	return mktime( &tm1 );
};

//---------- 返回给定时点在当前时区当日的起始之处(00:00) ----------
template <typename D>
inline time_point<system_clock, D> floor( const time_point<system_clock, D>& tp ) {
	/* 不能用 time_point_cast<days>() 函数对齐到86400秒的整倍数!因为除了
	 * 格林威治时区以外,其它时区的时间从epoch到每日00:00的计数都不是86400秒
	 * 的整倍数,如果强行floor就不能得到一个每日本地00:00的时点. */

	return time_point_cast<D>(
			   system_clock::from_time_t(
				   leon_utl::floor( system_clock::to_time_t( tp ) ) ) );
};

//========== 时点转换 ===========================================================
//---------- 将时点转成一个自epoch以来已经过的秒数(浮点表达) ----------
template<typename Clock, typename Rep, std::intmax_t M, std::intmax_t N>
inline double time2float(
	std::chrono::time_point<Clock,
	std::chrono::duration<Rep, std::ratio<M, N> > > tp ) {
	double result = tp.time_since_epoch().count();
	return result * M / N;
};

// 将时期值转为浮点数(代表自epoch以来的秒数)
template<typename Clock, typename Dura>
inline double secs_since_epoch( time_point<Clock, Dura> tp ) {
	auto ns_tp = time_point_cast<nanoseconds>( tp );
	return ns_tp.time_since_epoch().count() / static_cast<double>( NS_IN_1_SEC );
};

//========== 日期时间parsing ====================================================
//-------- parse时间 到 tm --------
inline constexpr const char* parse_time( tm& tm, const char* str ) {
	str = parse_int( tm.tm_hour, str, 2 );
	if( *str < '0' || *str > '9' )
		++str;
	str = parse_int( tm.tm_min, str, 2 );
	if( *str < '0' || *str > '9' )
		++str;
	str = parse_int( tm.tm_sec, str, 2 );
	return str;
};

template<typename ToDuration>
inline ToDuration parse_time( const char* str ) {
	int hour, mins, secs;
	str = parse_int( hour, str, 2 );
	if( *str < '0' || *str > '9' )
		++str;
	str = parse_int( mins, str, 2 );
	if( *str < '0' || *str > '9' )
		++str;
	str = parse_int( secs, str, 2 );

	SysTime_t::duration result = hours( hour ) + minutes( mins ) + seconds( secs );
	// 没有秒以下的部分了
	if( *str != '.' )
		return duration_cast<ToDuration>( result );

	// hour = mins = secs = 0;
	++str;
	for( mins = 0, secs = 0; mins < 9; ++mins )
		if( *str >= '0' && *str <= '9' )
			secs = secs * 10 + ( *str++ - '0' );
		else
			secs = secs * 10;

	return duration_cast<ToDuration>( result + nanoseconds( secs ) );
};

//-------- parse日期 到 tm --------
inline constexpr const char* parse_date( tm& tm, const char* str ) {
	str = parse_int( tm.tm_year, str, 4 );
	tm.tm_year -= 1900;
	if( *str < '0' || *str > '9' )
		++str;
	str = parse_int( tm.tm_mon, str, 2 );
	tm.tm_mon -= 1;
	if( *str < '0' || *str > '9' )
		++str;
	str = parse_int( tm.tm_mday, str, 2 );
	return str;
};

//-------- 日期 -> time_point --------
template<typename Duration>
inline time_point<system_clock, Duration> parse_date( const char* str ) {
	tm tm1 {};
	parse_date( tm1, str );
	return time_point_cast<Duration>( system_clock::from_time_t( mktime( &tm1 ) ) );
};

//-------- 同时parse日期时间 --------
inline constexpr const char* parse_datetime( tm& tm, const char* str ) {
	str = parse_date( tm, str );
	if( *str < '0' || *str > '9' )
		++str;
	str = parse_time( tm, str );
	return str;
};

//-------- 日期+时间 -> time_point ----
template<typename Duration>
inline time_point<system_clock, Duration> parse_datetime( const char* str ) {
	tm tm1 {};
	parse_datetime( tm1, str );
	return time_point_cast<Duration>( system_clock::from_time_t( mktime( &tm1 ) ) );
};

//========== 日期时间格式化 ======================================================
static const char* const FULL_FORMAT = "%Y-%m-%d %H:%M:%S";
static const char* const DATE_FORMAT = "%Y%m%d";
static const char* const HUMAN_DATE = "%Y-%m-%d";
static const char* const TIME_FORMAT = "%H:%M:%S";
static const char* const NEAT_FORMAT = "%y/%m/%d %H:%M:%S";

extern thread_local const std::time_put<char>& tl_time_formater;

// 格式化 tm
std::string format_time( const tm&, const char* = FULL_FORMAT );
// 格式化 time_t
std::string format_time( const std::time_t, const char* = FULL_FORMAT );
// 格式化 time_point, 指定格式
template <typename C, typename D>
std::string format_time( const time_point<C, D>, const char* = FULL_FORMAT );

// 格式化 time_point, 指定精度
template <typename C, typename D>
std::string format_time( const time_point<C, D>, size_t precision,
						 const char* = NEAT_FORMAT );

template <typename C, typename D>
std::string format_date( const time_point<C, D> );

template <typename R, typename P>
std::string format_dura( const duration<R, P> );

template <typename R, typename P>
std::string format_secs( const duration<R, P> );

//========== 构造日期时间值 ======================================================
SysTime_t make_sys_time( int y, int mon, int d,
						 int h = 0, int mi = 0, int s = 0,
						 int ms = 0, int us = 0, int ns = 0 );

template <typename D>
inline time_point<system_clock, D> make_time(
	int y,		int mo,		int d,
	int h = 0,	int mi = 0,	int s = 0,
	int ms = 0,	int us = 0,	int ns = 0 ) {
	return time_point_cast<D>( make_sys_time( y, mo, d, h, mi, s, ms, us, ns ) );
};

void delay( steady_clock::duration		dura,
			steady_clock::time_point	from = steady_clock::now() );

bool valid_date( const std::string& );

}; // namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
