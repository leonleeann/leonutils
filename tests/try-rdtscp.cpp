#include <functional>
#include <iostream>
#include <leonutils/Converts.hpp>
#include <leonutils/Rdtscp.hpp>
#include <leonutils/Statistics.hpp>

using namespace leon_utl;
using GetTime_f = std::function<int64_t( void )>;

static constexpr int TRY_COUNT = 1000;

int64_t Benchmark( char_cp name_, GetTime_f do_it_ ) {

	NumbVect_t diffs;
	diffs.reserve( TRY_COUNT );

	// 为防止受测函数被编译器优化掉, 必须留存它的结果
	// 先运行几次, 热身
	int64_t garbage = do_it_(); garbage += do_it_(); garbage += do_it_();

	for( int i = 0; i < TRY_COUNT; ++i ) {
		auto beg = TscClock_t::TscLFence();
		garbage += do_it_();
		auto end = TscClock_t::TscLFence();
		diffs.emplace_back( end - beg );
	};

	Statistic_t stats { diffs };
	std::cout << name_
			  << ":max:" << fmt( stats.max, 0, 2 )
			  << ",min:" << fmt( stats.min, 0, 2 )
			  << ",med:" << fmt( stats.med, 0, 2 )
			  << ",avg:" << fmt( stats.avg, 0, 2 )
			  << ",std:" << fmt( stats.std, 0, 2 )
			  << std::endl;
	return garbage;
};

int main() {
	// 为防止受测函数被编译器优化掉, 必须留存计算结果
	int64_t garbage = 0;

	//---- 看看 TscClock 的时戳正确性 -------------------------------
	TscClock_t rdtscp_clock;
	std::cout << "开始校准..." << std::endl;
	rdtscp_clock.calibrate( 10s );
	std::cout << "校准后,freqGHz():" << rdtscp_clock.freqGHz() << std::endl;

	std::cout << "开始为核对而采样..." << std::endl;
	NumbVect_t diffs;	diffs.reserve( TRY_COUNT );
	for( int j = 0; j < TRY_COUNT; ++j ) {
		auto sys_now = system_clock::now().time_since_epoch();
		auto tsc_now = rdtscp_clock.nowFree().time_since_epoch();
		diffs.emplace_back( ( tsc_now - sys_now ).count() );
	}
	std::cout << "为核对而采样完成." << std::endl;
	Statistic_t stats { diffs };
	std::cout << "差异"
			  << ":max:" << fmt( stats.max, 0, 2 )
			  << ",min:" << fmt( stats.min, 0, 2 )
			  << ",med:" << fmt( stats.med, 0, 2 )
			  << ",avg:" << fmt( stats.avg, 0, 2 )
			  << ",std:" << fmt( stats.std, 0, 2 )
			  << std::endl;

	//---- 看看各种操作的性能 -------------------------------
	garbage += Benchmark( "system_clock::now()    ", []() {
		return system_clock::now().time_since_epoch().count();
	} );

	garbage += Benchmark( "steady_clock::now()    ", []() {
		return steady_clock::now().time_since_epoch().count();
	} );

	garbage += Benchmark( "TscClock_t::TscFree()  ", []() { return TscClock_t::TscFree(); } );
	garbage += Benchmark( "TscClock_t::TscLFence()", []() { return TscClock_t::TscLFence(); } );
	garbage += Benchmark( "TscClock_t::nowFree()  ", [&]() { return rdtscp_clock.nowFree().time_since_epoch().count(); } );
	garbage += Benchmark( "TscClock_t::nowLFence()", [&]() { return rdtscp_clock.nowLFence().time_since_epoch().count(); } );

	std::cout << "garbage:" << garbage << std::endl;
	return EXIT_SUCCESS;
};

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; ;
