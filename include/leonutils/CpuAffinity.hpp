#pragma once
#include <string>

using str_t = std::string;

namespace leon_utl {

// 获取当前可用 CPU 总数
int GetCpuCount();

/* 显示当前线程可以运行在哪些CPU上
	目前不清楚 sched 和 pthread 两套API的效果到底有何不同,
	暂假定前者可以设置整个进程的全局限制, 而后者仅能限制单个线程.
*/
str_t GetSchedCpuMask();	// sched_getaffinity
str_t GetThreadCpuMask();	// pthread_getaffinity_np

/* 限制线程可运行于哪些 CPU
	1.目前不清楚 sched 和 pthread 两套API的效果到底有何不同, 暂假定前者可以设置
		整个进程的全局限制, 而后者仅能限制单个线程.
	2.以下两个函数接口一致, 传入参数是逗号分隔的 cpu_id, 不是 mask!!!
*/
void ProcessOnlyCPU( const str_t& cpu_ids );	// sched_setaffinity
void PthreadOnlyCPU( const str_t& cpu_ids );	// pthread_setaffinity_np

};	// namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; ;
