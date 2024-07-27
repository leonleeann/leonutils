#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <iomanip>
#include <iostream>
#include <ratio>
#include <sstream>
#include <type_traits>  // is_floating_point_v, is_signed_v, is_unsigned_v

#include "leonutils/Algorithms.hpp"
#include "leonutils/Chrono.hpp"

using namespace leon_utl;
using std::is_signed_v;
using std::is_unsigned_v;
using std::ratio;

/***************************************************************************
 * timespec struct
***************************************************************************/
TEST( TestTimeSpecOp, Comparisons ) {
	timespec ts1 = {}, ts2;
	ts2 = ts1;

	EXPECT_FALSE( ts1 >  ts2 );
	EXPECT_FALSE( ts1 <  ts2 );
	EXPECT_TRUE( ts1 == ts2 );
	EXPECT_FALSE( ts1 != ts2 );
	EXPECT_TRUE( ts1 <= ts2 );
	EXPECT_TRUE( ts1 >= ts2 );

	++ ts2.tv_nsec;
	EXPECT_FALSE( ts1 > ts2 );
	EXPECT_TRUE( ts1 < ts2 );
	EXPECT_FALSE( ts1 == ts2 );
	EXPECT_TRUE( ts1 != ts2 );
	EXPECT_TRUE( ts1 <= ts2 );
	EXPECT_FALSE( ts1 >= ts2 );

	ts2.tv_nsec -= 10;
	EXPECT_TRUE( ts1 > ts2 );
	EXPECT_FALSE( ts1 < ts2 );
	EXPECT_FALSE( ts1 == ts2 );
	EXPECT_TRUE( ts1 != ts2 );
	EXPECT_FALSE( ts1 <= ts2 );
	EXPECT_TRUE( ts1 >= ts2 );
};

TEST( TestTimeSpecOp, SubtractEachOther ) {
	timespec ts1 = {}, ts2;
	ts2 = ts1;

	EXPECT_EQ( ts1 - ts2, 0 );

	++ ts2.tv_nsec;
	EXPECT_EQ( ts1 - ts2, -1 );
	EXPECT_EQ( ts2 - ts1,  1 );

	ts2.tv_nsec -= 10;
	EXPECT_EQ( ts1 - ts2,  9 );
	EXPECT_EQ( ts2 - ts1, -9 );
};

TEST( TestTimeSpecOp, AddOneInteger ) {
	timespec ts1 = { 123, std::numeric_limits<decltype( ts1.tv_nsec )>::max() },
			 ts2;
	decltype( ts1.tv_nsec ) remain = ts1.tv_nsec % NS_IN_1_SEC;

	ts2 = ts1 + 1;
	EXPECT_EQ( ts2.tv_nsec, ( remain + 1 ) % NS_IN_1_SEC );
	EXPECT_EQ(
		ts2.tv_sec,
		ts1.tv_sec + ts1.tv_nsec / NS_IN_1_SEC
		+ ( remain + 1 ) / NS_IN_1_SEC
	);

	ts2 = ts1 + 1000000001;
	EXPECT_EQ( ts2.tv_nsec, ( remain + 1000000001 ) % NS_IN_1_SEC );
	EXPECT_EQ(
		ts2.tv_sec,
		ts1.tv_sec + ts1.tv_nsec / NS_IN_1_SEC
		+ ( remain + 1000000001 ) / NS_IN_1_SEC
	);

	ts2 = ts1 + ( - 1 );
	EXPECT_EQ( ts2.tv_nsec, ( remain - 1 ) % NS_IN_1_SEC );
	EXPECT_EQ(
		ts2.tv_sec,
		ts1.tv_sec + ts1.tv_nsec / NS_IN_1_SEC
		+ ( remain - 1 ) / NS_IN_1_SEC
	);
};

TEST( TestTimeSpecOp, AddOneNegtive ) {
	timespec ts1 = { 123, std::numeric_limits<decltype( ts1.tv_nsec )>::min() },
			 ts2;

	ts2 = ts1 + ( -1 );
	ts1.tv_sec += ts1.tv_nsec / NS_IN_1_SEC;
	ts1.tv_nsec = ts1.tv_nsec % NS_IN_1_SEC - 1;
	EXPECT_LT( ts1.tv_nsec, 0 );
	if( ts1.tv_nsec < 0 ) {
		--ts1.tv_sec;
		ts1.tv_nsec += NS_IN_1_SEC;
	}
	EXPECT_GE( ts1.tv_nsec, 0 );
	EXPECT_LT( ts1.tv_nsec, NS_IN_1_SEC );
	EXPECT_EQ( ts1.tv_sec, ts2.tv_sec );
	EXPECT_EQ( ts1.tv_nsec, ts2.tv_nsec );

	ts1 = { 123, -999999999 };
	ts2 = ts1 + ( -999999999 );
	ts1 = { 121, 2 };
	EXPECT_EQ( ts1.tv_sec, ts2.tv_sec );
	EXPECT_EQ( ts1.tv_nsec, ts2.tv_nsec );
};

TEST( TestTimeSpecOp, AddOneWithSelf ) {
	timespec ts1 = { 123, std::numeric_limits<decltype( ts1.tv_nsec )>::min() },
			 ts2 = ts1;
	ts2 += -1;
	ts1.tv_sec += ts1.tv_nsec / NS_IN_1_SEC;
	ts1.tv_nsec = ts1.tv_nsec % NS_IN_1_SEC - 1;
	if( ts1.tv_nsec < 0 ) {
		--ts1.tv_sec;
		ts1.tv_nsec += NS_IN_1_SEC;
	}
	EXPECT_EQ( ts1.tv_sec, ts2.tv_sec );
	EXPECT_EQ( ts1.tv_nsec, ts2.tv_nsec );

	ts1 = { 123, -999999999 };
	ts2 = ts1;
	ts2 += -999999999;
	ts1 = { 121, 2 };
	EXPECT_EQ( ts1.tv_sec, ts2.tv_sec );
	EXPECT_EQ( ts1.tv_nsec, ts2.tv_nsec );

	ts1 = { 123, 999999999 };
	ts2 = ts1;
	ts2 += 1;
	ts1 = { 124, 0 };
	EXPECT_EQ( ts1.tv_sec, ts2.tv_sec );
	EXPECT_EQ( ts1.tv_nsec, ts2.tv_nsec );

	ts1 = { 123, -999999999 };
	ts2 = ts1;
	ts2 -= 1;
	ts1 = { 122, 0 };
	EXPECT_EQ( ts1.tv_sec, ts2.tv_sec );
	EXPECT_EQ( ts1.tv_nsec, ts2.tv_nsec );

	ts1 = { 123, 0 };
	ts2 = ts1;
	ts2 -= 1;
	ts1 = { 122, 999999999 };
	EXPECT_EQ( ts1.tv_sec, ts2.tv_sec );
	EXPECT_EQ( ts1.tv_nsec, ts2.tv_nsec );
};

TEST( TestFormatTimeP, WithPrec ) {
	tm tm1 = {};
	tm1.tm_gmtoff  = 3600 * 8;
	tm1.tm_year    = 2019 - 1900;
	tm1.tm_mon     = 6;
	tm1.tm_mday    = 29;
	tm1.tm_hour    = 15;
	tm1.tm_min     = 15;
	tm1.tm_sec     = 59;
	system_clock::time_point stpNow
		= system_clock::from_time_t( mktime( &tm1 ) ) + nanoseconds( 123456789 );

	ASSERT_EQ( format_time( stpNow, 9 ), str_t( "19/07/29 15:15:59.123456789" ) );
	ASSERT_EQ( format_time( stpNow, 6 ), str_t( "19/07/29 15:15:59.123456" ) );
	ASSERT_EQ( format_time( stpNow, 3 ), str_t( "19/07/29 15:15:59.123" ) );
	ASSERT_EQ( format_time( stpNow, size_t( 0 ) ), str_t( "19/07/29 15:15:59" ) );
	ASSERT_EQ( format_time( stpNow, size_t( 0 ), "%m/%d %H:%M" ),
			   str_t( "07/29 15:15" ) );
};

TEST( TestChrono, convertToFloating ) {
	auto ms_12345 = milliseconds{ 12345 };
	time_point<system_clock, milliseconds> in_ms { ms_12345 };
	ASSERT_TRUE( leon_utl::eq( dura2float( ms_12345 ), 12.345 ) );
	ASSERT_TRUE( leon_utl::eq( time2float( in_ms ), 12.345 ) );

	auto us_12345 = microseconds{ 12345 };
	time_point<system_clock, microseconds> in_us { us_12345 };
	ASSERT_TRUE( leon_utl::eq( dura2float( us_12345 ), 0.012345 ) );
	ASSERT_TRUE( leon_utl::eq( time2float( in_us ), 0.012345 ) );

	auto ns_12345 = nanoseconds{ 12345 };
	time_point<system_clock, nanoseconds> in_ns { ns_12345 };
	ASSERT_TRUE( leon_utl::eq( dura2float( ns_12345 ), 0.000012345 ) );
	ASSERT_TRUE( leon_utl::eq( time2float( in_ns ), 0.000012345 ) );

	auto ss_12345 = seconds{ 12345 };
	time_point<steady_clock, seconds> in_s { ss_12345 };
	ASSERT_TRUE( leon_utl::eq( dura2float( ss_12345 ), 12345.0 ) );
	ASSERT_TRUE( leon_utl::eq( time2float( in_s ), 12345.0 ) );
};

TEST( TestChrono, parseTime ) {
	tm tm1 = {};
	ASSERT_EQ( tm1.tm_hour, 0 );
	ASSERT_EQ( tm1.tm_min, 0 );
	ASSERT_EQ( tm1.tm_sec, 0 );
	parse_time( tm1, "13:28:45" );
	ASSERT_EQ( tm1.tm_hour, 13 );
	ASSERT_EQ( tm1.tm_min, 28 );
	ASSERT_EQ( tm1.tm_sec, 45 );
};

TEST( TestChrono, parseTime2 ) {
	milliseconds mil_sec {};
	ASSERT_EQ( mil_sec.count(), 0 );
	mil_sec = duration_cast<milliseconds>( parse_time( "00:00:05" ) );
	ASSERT_EQ( mil_sec.count(), 5000 );
	mil_sec = duration_cast<milliseconds>( parse_time( "00:00:05.123" ) );
	ASSERT_EQ( mil_sec.count(), 5123 );

	seconds secs {};
	ASSERT_EQ( secs.count(), 0 );
	secs = duration_cast<seconds>( parse_time( "00:00:05" ) );
	ASSERT_EQ( secs.count(), 5 );
	secs = duration_cast<seconds>( parse_time( "00:00:05.999" ) );
	ASSERT_EQ( secs.count(), 5 );

	nanoseconds nano_sec {};
	ASSERT_EQ( nano_sec.count(), 0 );
	nano_sec = parse_time( "00:00:05" );
	ASSERT_EQ( nano_sec.count(), 5000000000 );
	nano_sec = parse_time( "00:00:05.123456789" );
	ASSERT_EQ( nano_sec.count(), 5123456789 );
};

TEST( TestChrono, parseDate ) {
	tm tm1 = {};
	ASSERT_EQ( tm1.tm_year, 0 );
	ASSERT_EQ( tm1.tm_mon, 0 );
	parse_date( tm1, "20190729" );
	ASSERT_EQ( tm1.tm_year, 2019 - 1900 );
	ASSERT_EQ( tm1.tm_mon, 6 );

	ASSERT_EQ( parse_date( "20190729" ), make_time( 2019, 7, 29 ) );
};

TEST( TestFormatTimeP, WithFormat ) {
	system_clock::time_point tp {};
	// 注意时差! UTC 的 epoch 用咱的时区格式化出来就是: "1970-01-01 08:00:00"
	ASSERT_EQ( format_time( tp ), "1970-01-01 08:00:00" );

	tm tm1 = {};
	tm1.tm_gmtoff  = 3600 * 8;
	tm1.tm_year    = 2019 - 1900;
	tm1.tm_mon     = 6;
	tm1.tm_mday    = 29;
	tm1.tm_hour    = 15;
	tm1.tm_min     = 15;
	tm1.tm_sec     = 59;
	tp = system_clock::from_time_t( mktime( &tm1 ) ) + nanoseconds( 123456789 );

	ASSERT_EQ( format_time( tp ), "2019-07-29 15:15:59" );
	ASSERT_EQ( format_time( tp, DATE_FORMAT ), "20190729" );
	ASSERT_EQ( format_time( tp, TIME_FORMAT ), "15:15:59" );
	ASSERT_EQ( format_time( tp, "%Y-%m-%d %H:%M:%S" ), "2019-07-29 15:15:59" );
	ASSERT_EQ( format_time( tp, 9, "%Y-%m-%d %H:%M:%S" ), "2019-07-29 15:15:59.123456789" );
};

TEST( TestMakeSysTime, makeSysTime ) {
	auto stp1 = make_time( 2019, 7, 29, 23, 59, 59, 123, 456, 789 );
	ASSERT_EQ( format_time( stp1, 9 ), "19/07/29 23:59:59.123456789" );
};

TEST( TestMakeUSTime, makeUSTime ) {
	auto stp1 = make_time( 2019, 7, 29, 23, 59, 59, 123, 456 );
	ASSERT_EQ( format_time( stp1, 6 ), "19/07/29 23:59:59.123456" );
	ASSERT_EQ( format_time( stp1, 9 ), "19/07/29 23:59:59.123456000" );
};

TEST( TestFormatDuration, formatDura ) {
	ASSERT_EQ( format_dura( 86401s ), "1D 0H 0M 1S" );
	ASSERT_EQ( format_dura( 86399s ), "23H 59M 59S" );
	ASSERT_EQ( format_dura( 3601s ), "1H 0M 1S" );
	ASSERT_EQ( format_dura( 3600s ), "1H 0M 0S" );
	ASSERT_EQ( format_dura( 3599s ), "59M 59S" );

	ASSERT_EQ( format_secs( 3599001ms ), "3599.001s" );
};

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
