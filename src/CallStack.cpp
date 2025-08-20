#include <ostream>

#include "leonutils/CallStack.hpp"

using str_t = std::string;

namespace leon_utl {

thread_local std::array<const char*, STACK_CAPA> tl_call_stack;

/*
	0.2.0 改用定长数组实现:
		如果栈满了, 就移动 s_btm 一格, 因为实际使用中并不需要保留全部调用信息, 只需最近
		的 N 个.
*/

thread_local int	s_top {};
thread_local int	s_btm {};

Tracer_t::Tracer_t( const char* fn_ ): _name( fn_ ) {
	tl_call_stack[ ++s_top & STACK_MASK ] = fn_;

	int new_btm = s_top - STACK_MASK;
	s_btm = std::max( s_btm, new_btm );
};

Tracer_t::~Tracer_t() {
	if( s_top <= 0 || tl_call_stack[ s_top & STACK_MASK ] != _name )
		return;

	--s_top;
	s_btm = std::min( s_btm, s_top );
};

void Tracer_t::ClearCallStack() {
	s_btm = s_top = 0;
};

void Tracer_t::PrintCallStack( std::ostream& os_ ) {
	str_t tabs;

	int i = s_btm + 1;
	for( ; i <= s_top; ++i )
		os_ << ( tabs += '\t' ) << tl_call_stack[ i & STACK_MASK ] << "()\n";
};

}; //namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
