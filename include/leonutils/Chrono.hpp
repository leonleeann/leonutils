#pragma once
#include "leonutils/ChronoTypes.hpp"
#include "leonutils/Converts.hpp"

using namespace std::chrono;
using namespace std::chrono_literals;
using ts_nsec_t = decltype( timespec::tv_nsec );
using ts_secs_t = decltype( timespec::tv_sec );
static_assert( sizeof( ts_nsec_t ) == 8 );
constexpr ts_nsec_t NS_IN_1_SEC = 1000 * 1000 * 1000;

//========== 两个timespec时间结构的比较及运算 ======================================
inline bool operator > ( const timespec& ts1_, const timespec& ts2_ ) {
	return ( ( ts1_.tv_sec  > ts2_.tv_sec ) ||
			 ( ts1_.tv_sec == ts2_.tv_sec && ts1_.tv_nsec > ts2_.tv_nsec ) );
};
inline bool operator < ( const timespec& ts1_, const timespec& ts2_ ) {
	return ( ( ts1_.tv_sec  < ts2_.tv_sec ) ||
			 ( ts1_.tv_sec == ts2_.tv_sec && ts1_.tv_nsec < ts2_.tv_nsec ) );
};
inline bool operator ==( const timespec& ts1_, const timespec& ts2_ ) {
	return ( ts1_.tv_sec == ts2_.tv_sec && ts1_.tv_nsec == ts2_.tv_nsec );
};
inline bool operator !=( const timespec& ts1_, const timespec& ts2_ ) {
	return !( ts1_ == ts2_ );
};
inline bool operator <=( const timespec& ts1_, const timespec& ts2_ ) {
	return ( ts1_ == ts2_ || ts1_ < ts2_ );
};
inline bool operator >=( const timespec& ts1_, const timespec& ts2_ ) {
	return ( ts1_ == ts2_ || ts1_ > ts2_ );
};

inline ts_nsec_t operator - ( const timespec& ts1_, const timespec& ts2_ ) {
	return ( ( ts1_.tv_sec - ts2_.tv_sec ) * NS_IN_1_SEC
			 + ts1_.tv_nsec - ts2_.tv_nsec );
};

inline timespec operator + ( const timespec& ts_, ts_nsec_t ns_ ) {
	timespec result = {
		ts_.tv_sec + ts_.tv_nsec / NS_IN_1_SEC + ns_ / NS_IN_1_SEC,
		ts_.tv_nsec % NS_IN_1_SEC,
	};

	ts_nsec_t sum = ns_ % NS_IN_1_SEC + result.tv_nsec;
	result.tv_sec += sum / NS_IN_1_SEC;
	sum = sum % NS_IN_1_SEC;

	// 负值修正
	result.tv_nsec = ( sum + NS_IN_1_SEC ) % NS_IN_1_SEC;
	result.tv_sec += ( sum - result.tv_nsec ) / NS_IN_1_SEC;
	return result;
};

inline timespec operator- ( const timespec& ts_, ts_nsec_t ns_ ) {
	return ts_ + ( - ns_ );
};

inline timespec& operator+=( timespec& ts_, ts_nsec_t ns_ ) {
	ts_ = ts_ + ns_;
	return ts_;
};

inline timespec& operator-=( timespec& ts_, ts_nsec_t ns_ ) {
	ts_ = ts_ - ns_;
	return ts_;
};

namespace leon_utl {

//========== 时点运算 ===========================================================

//---------- 时点对齐到整日(按本地时区算) ---------------------------
inline void floor( tm* local_ ) {
	local_->tm_hour = local_->tm_min = local_->tm_sec = 0;
	return;
};

inline time_t floor( time_t local_ ) {
	tm tm1 = {};
	localtime_r( &local_, &tm1 );
	leon_utl::floor( &tm1 );
	return mktime( &tm1 );
};

//---------- 返回给定时点在当前时区当日的起始之处(00:00:00) ----------
template <typename D>
inline time_point<system_clock, D> floor( const time_point<system_clock, D>& tp_ ) {
	/* 不能用 time_point_cast<days>() 函数对齐到86400秒的整倍数! 因为除了格林威治
		时区以外, 其它时区的时间从epoch到每日00:00的计数都不是86400秒的整倍数,如果强
		行 floor 就不能得到一个每日本地00:00的时点. */

	return time_point_cast<D>(
			   system_clock::from_time_t(
				   leon_utl::floor( system_clock::to_time_t( tp_ ) ) ) );
};

//========== 时点转换 ===========================================================
//---------- 将时点转成一个自epoch以来已经过的秒数(浮点表达) ----------
template<typename Clock, typename Rep, std::intmax_t M, std::intmax_t N>
inline double time2float( time_point<Clock, duration<Rep, std::ratio<M, N>>> tp_ ) {
	double result = tp_.time_since_epoch().count();
	return result * M / N;
};

// 将时期值转为浮点数(代表自epoch以来的秒数)
template<typename Rep, std::intmax_t M, std::intmax_t N>
inline double dura2float( duration<Rep, std::ratio<M, N>> d_ ) {
	double result = d_.count();
	return result * M / N;
};

/*------------------------------------------------------------------------------
	postgresql 的时戳字段只能精确到微秒, 需要额外字段专门保存纳秒值.
	而且用浮点数送过去害怕有误差, 所以转为以秒为单位的整数传入, 另外用两个整数分别传入
	微秒值和纳秒值. 所以需要一个分解函数和struct, 省得到处写. */
struct SsUsNs_t {	// 没有毫秒值, 毫秒值 * 1000 和微秒存在一起
	SsUsNs_t() = default;
	// 将时间值分解为 秒数部分(代表自epoch以来的秒数), 微秒部分, 纳秒部分
	SsUsNs_t( SysTime_t tp_ ) {
		auto all = tp_.time_since_epoch().count();
		ss = all / NS_IN_1_SEC;
		all %= NS_IN_1_SEC;
		us = all / 1000;
		ns = all % 1000;
	};

	int64_t	ss;
	int32_t	us;
	int32_t	ns;
};
//------------------------------------------------------------------------------

//========== 日期时间parsing ====================================================
//-------- parse时间 到 tm --------
inline constexpr const char* parse_time( tm& tm_, const char* str_ ) {
	str_ = parse_int( tm_.tm_hour, str_, 2 );
	if( *str_ < '0' || *str_ > '9' )
		++str_;
	str_ = parse_int( tm_.tm_min, str_, 2 );
	if( *str_ < '0' || *str_ > '9' )
		++str_;
	str_ = parse_int( tm_.tm_sec, str_, 2 );
	return str_;
};

inline SysDura_t parse_time( const char* str_ ) {
	int hh, mi, ss;
	str_ = parse_int( hh, str_, 2 );
	if( *str_ < '0' || *str_ > '9' )
		++str_;
	str_ = parse_int( mi, str_, 2 );
	if( *str_ < '0' || *str_ > '9' )
		++str_;
	str_ = parse_int( ss, str_, 2 );

	SysDura_t result = hours( hh ) + minutes( mi ) + seconds( ss );
	// 没有秒以下的部分了
	if( *str_ != '.' )
		return result;

	// 现在 ss 存放 纳秒值, mi 作为位数记录, 固定9位
	++str_;
	for( mi = 0, ss = 0; mi < 9; ++mi ) {
		char c = str_[mi];
		if( c >= '0' && c <= '9' )
			ss = ss * 10 + ( c - '0' );
		else
			ss = ss * 10;
	}
	return result + nanoseconds( ss );
};

//-------- parse日期 到 tm --------
inline constexpr const char* parse_date( tm& tm_, const char* str_ ) {
	str_ = parse_int( tm_.tm_year, str_, 4 );
	tm_.tm_year -= 1900;
	if( *str_ < '0' || *str_ > '9' )
		++str_;
	str_ = parse_int( tm_.tm_mon, str_, 2 );
	tm_.tm_mon -= 1;
	if( *str_ < '0' || *str_ > '9' )
		++str_;
	str_ = parse_int( tm_.tm_mday, str_, 2 );
	return str_;
};

//-------- 日期 -> time_point --------
inline SysTime_t parse_date( const char* str_ ) {
	tm tm1 {};
	parse_date( tm1, str_ );
	return system_clock::from_time_t( mktime( &tm1 ) );
};

//-------- 同时parse日期时间 --------
inline constexpr const char* parse_datetime( tm& tm_, const char* str_ ) {
	str_ = parse_date( tm_, str_ );
	if( *str_ < '0' || *str_ > '9' )
		++str_;
	str_ = parse_time( tm_, str_ );
	return str_;
};

//-------- 日期+时间 -> time_point ----
inline SysTime_t parse_datetime( const char* str_ ) {
	tm tm1 {};
	str_ = parse_date( tm1, str_ );
	if( *str_ < '0' || *str_ > '9' )
		++str_;
	return system_clock::from_time_t( mktime( &tm1 ) ) + parse_time( str_ );
};

//========== 日期时间格式化 ======================================================
static const char* const FULL_FORMAT = "%Y-%m-%d %H:%M:%S";
static const char* const DATE_FORMAT = "%Y%m%d";
static const char* const HUMAN_DATE = "%Y-%m-%d";
static const char* const TIME_FORMAT = "%H:%M:%S";
static const char* const NEAT_FORMAT = "%y/%m/%d %H:%M:%S";

extern thread_local const std::time_put<char>& tl_time_formater;

// 格式化 tm
str_t fmt_tm( const tm&, const char* fmt = FULL_FORMAT );
// 格式化 time_t
str_t fmt_tmt( const std::time_t, const char* fmt = FULL_FORMAT );
// 格式化 time_point, 指定格式
template <typename C, typename D>
str_t fmt( const time_point<C, D>, const char* fmt = FULL_FORMAT );

// 格式化 time_point, 指定精度
template <typename C, typename D>
str_t fmt( const time_point<C, D>, size_t precision,
		   const char* fmt = NEAT_FORMAT );

template <typename C, typename D>
str_t fmt_date( const time_point<C, D> );

template <typename R, typename P>
str_t fmt_dura( const duration<R, P> );

template <typename R, typename P>
str_t fmt_secs( const duration<R, P> );

//========== 构造日期时间值 ======================================================
SysTime_t make_time( int y, int mon, int d,
					 int h = 0, int mi = 0, int s = 0,
					 int ms = 0, int us = 0, int ns = 0 );

void delay( MonDura_t lasting, MonTime_t from = steady_clock::now() );

bool valid_date( const str_t& );

}; // namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
