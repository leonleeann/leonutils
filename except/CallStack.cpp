#include <fmt/format.h>
#include <forward_list>
#include <iostream>
#include <string>

#include "CallStack.hpp"

using std::forward_list;
using std::string;

namespace leon_utl {

thread_local forward_list<const char*> tl_call_stack;

Tracer_t::Tracer_t( const char* point_ ): _name( point_ ) {
	tl_call_stack.emplace_front( point_ );
};

Tracer_t::~Tracer_t() {
	auto top = tl_call_stack.front();
	if( top == _name )
		tl_call_stack.pop_front();

/* 析构器里面不应抛异常!
	else
		throw std::runtime_error(
			fmt::format( "怎么可能退出\"{}\"时看到的栈顶函数\"{}\"不一致?", _name, top ) );
*/
};

void Tracer_t::ClearCallStack() {
	tl_call_stack.clear();
};

void Tracer_t::PrintCallStack( std::ostream& o ) {
	forward_list<const char*> reversed { tl_call_stack };
	reversed.reverse();

	string tabs;
	for( auto& fn : reversed )
		o << ( tabs += '\t' ) << fn << "()\n";
};

}; //namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
