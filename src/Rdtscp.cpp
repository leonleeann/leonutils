#include <thread>

#include "leonutils/CallStack.hpp"
#include "leonutils/Exceptions.hpp"
#include "leonutils/Rdtscp.hpp"

namespace leon_utl {

TscClock_t::Anchor_t TscClock_t::GetAnchor( int cnt_ ) {
	TRACE_POINT;

	if( cnt_ < 1 )
		throw bad_usage( "TscClock_t:1次都不尝试,取什么锚点?" );

	// 尝试多次, 取耗时最短的那次结果, 理论上两类时戳最靠近
	Anchor_t result {};

	for( int i = 0; i < cnt_; i++ ) {
		auto beg = TscLFence();
		auto now = system_clock::now();
		auto end = TscLFence();
		auto dif = end - beg;
		if( dif < result.jitter ) {
			result.tstamp = now;
			result.cycles = ( beg + end ) / 2;
			result.jitter = dif;
		}
	}

	return result;
};

void TscClock_t::calibrate( SysDura_t for_ ) {
	TRACE_POINT;

	// 取锚点时尝试多少次
	static constexpr int ANCHOR_TRIES = 10000;

	_anc0 = GetAnchor( ANCHOR_TRIES );
	std::this_thread::sleep_for( for_ );
	_anc1 = GetAnchor( ANCHOR_TRIES );
	_setup();
};

void TscClock_t::calibrate() {
	TRACE_POINT;

	// 只有新采集的锚点比历史锚点更优, 并且留存的两个锚点相距更远, 才会保留新锚点
	auto new_anc = GetAnchor( 1 );

	// 只要新锚点比 _anc1 更优, 那肯定更新 _anc1. 因为时间跨度肯定也更大了.
	if( new_anc.jitter <= _anc1.jitter ) {
		_anc1 = new_anc;
		_setup();
		return;
	}

	if( new_anc.jitter > _anc0.jitter )
		return;

	// 只比 _anc0 更优, 那就尽量保留时间跨度大的两个锚点.
	auto span0 = std::abs( _anc1.cycles - _anc0.cycles );
	auto span1 = std::abs( _anc1.cycles - new_anc.cycles );
	if( span1 > span0 ) {
		_anc0 = new_anc;
		_setup();
	}
};

void TscClock_t::_setup() {
	TRACE_POINT;

	if( _anc0.cycles > _anc1.cycles )
		std::swap( _anc0, _anc1 );

	double nanoss = ( _anc1.tstamp - _anc0.tstamp ).count();
	double cycles = _anc1.cycles - _anc0.cycles;
	_ghz = cycles / nanoss;
};

};	// namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
