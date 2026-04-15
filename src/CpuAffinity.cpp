#include <cerrno>		// errno
#include <cstring>		// strlen, strncmp, strncpy, memset, memcpy, memmove, strerror
#include <iomanip>
#include <iostream>
#include <leonutils/CpuAffinity.hpp>
#include <leonutils/SetOps.hpp>
#include <pthread.h>
#include <sched.h>
#include <set>
#include <unistd.h>

using IntSet_t = set_t<int>;

namespace leon_utl {

int GetCpuCount() {

	auto cpu_cnt = sysconf( _SC_NPROCESSORS_ONLN );
	if( cpu_cnt < 1 ) {
		std::cerr << "获取CPU数失败:" << std::strerror( errno ) << std::endl;
		return -1;
	}

	return cpu_cnt;
};

str_t _Mask2Str( const cpu_set_t* mask_, int total_ ) {
	str_t result;
	result.reserve( total_ );

	for( int i = 0; i < total_; ++i )
		result.append( 1, CPU_ISSET( i, mask_ ) ? '1' : '0' );

	return result;
};

str_t GetSchedCpuMask() {
	auto cpu_cnt = GetCpuCount();
	if( cpu_cnt < 0 )
		return {};

	cpu_set_t cpu_mask;
	CPU_ZERO( &cpu_mask );
	if( sched_getaffinity( 0, sizeof( cpu_set_t ), &cpu_mask ) ) {
		std::cerr << "sched_getaffinity失败:" << std::strerror( errno ) << std::endl;
		return {};
	}

	return _Mask2Str( &cpu_mask, cpu_cnt );
};

str_t GetThreadCpuMask() {
	auto cpu_cnt = GetCpuCount();
	if( cpu_cnt < 0 )
		return {};

	cpu_set_t cpu_mask;
	CPU_ZERO( &cpu_mask );
	if( pthread_getaffinity_np( pthread_self(), sizeof( cpu_set_t ), &cpu_mask ) ) {
		std::cerr << "pthread_getaffinity_np失败:" << std::strerror( errno ) << std::endl;
		return {};
	}

	return _Mask2Str( &cpu_mask, cpu_cnt );
};

cpu_set_t _MakeSet( const IntSet_t& ids_ ) {

	cpu_set_t cpu_set;
	CPU_ZERO( &cpu_set );

	for( auto id : ids_ )
		CPU_SET( id, &cpu_set );

	return cpu_set;
};

void ProcessOnlyCPU( str_cr ids_ ) {

	auto id_set = split2set<int>( ids_, ',' );
	if( id_set.empty() ) {
		std::cerr << "不可能1个CPU也不用!" << std::endl;
		return;
	}

	if( id_set.contains( -1 ) ) {
		std::cerr << "-1代表使用全部CPU,那还来绑核干嘛?!" << std::endl;
		return;
	}

	auto cpu_set = _MakeSet( id_set );

	if( sched_setaffinity( 0, sizeof( cpu_set_t ), &cpu_set ) )
		std::cerr << "sched_setaffinity失败:" << std::strerror( errno ) << std::endl;
};

void PthreadOnlyCPU( str_cr ids_ ) {

	auto id_set = split2set<int>( ids_, ',' );
	if( id_set.empty() ) {
		std::cerr << "不可能1个CPU也不用!" << std::endl;
		return;
	}

	if( id_set.contains( -1 ) ) {
		std::cerr << "-1代表使用全部CPU,那还来绑核干嘛?!" << std::endl;
		return;
	}

	auto cpu_set = _MakeSet( id_set );

	if( pthread_setaffinity_np( pthread_self(), sizeof( cpu_set_t ), &cpu_set ) )
		std::cerr << "pthread_setaffinity_np失败:" << std::strerror( errno ) << std::endl;
};

};	// namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; ;
