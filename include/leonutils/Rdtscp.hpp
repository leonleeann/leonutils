#pragma once
#include <x86intrin.h>
#include <leonutils/ChronoTypes.hpp>
#include <leonutils/UnionTypes.hpp>

/* 用汇编指令 rdtscp 来模拟系统时戳
目的:
	1.全系统到处需要获取日历时戳(自1970-01-01以来的纳秒数), 试试这个方式能否降低开销;
	2.为测量"小片代码"性能而提供比 std::chrono 更精确的、极速的测时手段.

思路:
	rdtscp 能够提供"自开机以来的主频时钟周期数", 在校准(calibrate)方法中确定两个参考值:
		a.时间锚点(rdtscp时戳和日历时戳的大致偏移);
		b.频率缩放(每纳秒大约有多少个rdtscp周期).
	有了以上基准, 每次需要系统时戳时, 就不需要再去调用 std::chrono::system_clock::now().
	因为它又慢, 又不稳定, 不适合用于测量"小片代码"耗时.
*/

namespace leon_utl {

class TscClock_t {
//==== 类接口 ===================================================================
public:
	struct Anchor_t {
		SysTime_t	tstamp {};
		int64_t		cycles { INT64_MAX };
		int64_t		jitter { INT64_MAX };
	};

	// 自由读取 rdtscp 周期数
	[[gnu::always_inline]]
	static int64_t TscFree() {
		// uint32_t cpu_id; uint64_t cycles = __rdtscp( &cpu_id ); _mm_lfence();
		// return cycles;
		U64_u tsc;
		asm volatile( "rdtscp" : "=a"( tsc.dw0 ), "=d"( tsc.dw1 ) :: "ecx" );
		return tsc.asI;
	}

	// 比 TscFree 多一条 lfence, 避免后续指令提前执行
	[[gnu::always_inline]]
	static int64_t TscLFence() {
		U64_u tsc;
		asm volatile( "rdtscp" : "=a"( tsc.dw0 ), "=d"( tsc.dw1 ) :: "ecx" );
		asm volatile( "lfence" ::: "memory" );
		return tsc.asI;
	}

	// 获得一个锚点
	static Anchor_t GetAnchor( int try_times );

//==== 对象接口 =================================================================

//---- 生产服务函数: 当前时间(模拟日历时戳) -----------------------------------------
	[[gnu::always_inline]]
	SysTime_t nowFree() const {
		auto cycles = TscFree() - _anc0.cycles;
		auto nanoss = static_cast<ns::rep>( cycles / _ghz + 0.5 );
		return _anc0.tstamp + ns( nanoss );
	};

//---- 生产服务函数: 开始时间(多一条lfence, 避免后续指令提前执行) ----------------------
	[[gnu::always_inline]]
	SysTime_t nowLFence() const {
		auto cycles = TscLFence() - _anc0.cycles;
		auto nanoss = static_cast<ns::rep>( cycles / _ghz + 0.5 );
		return _anc0.tstamp + ns( nanoss );
	};

	/* 初次校准(必做):
		在给定时长的头尾, 分别取两次"锚点", 来估算"每纳秒大约有多少周期".
		调用过本函数后, 本对象就可使用了.
	*/
	void calibrate( SysDura_t for_time );

	/* 维持校准(选做):
		调用过"初次校准"后, 持续调用本函数可完成:
		1.发现更好的锚点;
		2.持续校准期越长, 对主频估算越准.
	*/
	void calibrate();

	double freqGHz() const { return _ghz; }

private:
	void _setup();

	// 基准锚点
	Anchor_t	_anc0 {}, _anc1 {};

	// 每纳秒大约有多少个rdtscp周期(相当于主频多少 GHz)
	double		_ghz {};
};

};	// namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
