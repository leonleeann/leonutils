#pragma once
#include <LeonLog>
#include <atomic>
#include <chrono>
#include <forward_list>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "buffer/ShmAtomicRQ.tpp"
#include "event/MemoryOrder.hpp"
#include "misc/Algorithms.hpp"
#include "misc/Converts.hpp"

using namespace leon_ext;
using namespace leon_log;
using namespace std::chrono;
using namespace std::chrono_literals;
using namespace std;

union Msg_u {
	int64_t	as_tstamp;
	char	as_buffer[ sizeof( uint64_t ) ];
};
using MsgQ_t = ShmAtmRQ_t<Msg_u>;

using Delays_t = vector<double>;

enum RunMode_e {
	// 按"测试多少次"运行
	ForCounting,

	// 按"测试多久"运行
	ForDuration,
};

constexpr int64_t	RUN_CONTINUOUSLY = 1;
constexpr int64_t	STOP_IMMEDIATELY = -1;

constexpr char		SHARQ_NAME [] = "shm_ring_que";

inline int64_t rdtscp( void ) {
	uint32_t eax, edx;
	asm volatile( "rdtscp"
				  : "=a"( eax ), "=d"( edx )
				  :
				  : "%ecx", "memory" );

	return ( static_cast<int64_t>( edx ) << 32 )
		   | static_cast<int64_t>( eax );
};

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
