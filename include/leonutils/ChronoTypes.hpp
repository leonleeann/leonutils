#pragma once
#include <chrono>

using namespace std::chrono;
using namespace std::chrono_literals;
using ms = std::chrono::milliseconds;
using us = std::chrono::microseconds;
using ns = std::chrono::nanoseconds;

namespace leon_utl {

// 只有日期部分
using SysDate_t = time_point<system_clock, duration<int, std::ratio<86400, 1>>>;

// 全系统统一时戳
using SysTime_t = system_clock::time_point;
// 最高精度,纳秒单位
using SysDura_t = system_clock::duration;

// 精确计时的时候用的时戳
using MonTime_t = steady_clock::time_point;
using MonDura_t = steady_clock::duration;

// 一刻钟, 900秒
using Quarter_t = duration<int64_t, std::ratio<900>>;

/*
using Dura01Min_T = duration<int, ratio< 60>>;
using Dura02Min_T = duration<int, ratio<120>>;
using Dura03Min_T = duration<int, ratio<180>>;
using Dura05Min_T = duration<int, ratio<300>>;
using Dura10Min_T = duration<int, ratio<600>>;
using Dura15Min_T = duration<int, ratio<900>>;

using TimeP15Min_T = time_point<steady_clock, Dura15Min_T>;
*/

}; // namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
