#include <csetjmp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <iostream>
#include <ostream>
#include <string>

#include "leonutils/CallStack.hpp"

using namespace leon_utl;
using std::ostringstream;
using std::string;

thread_local sigjmp_buf sig_recovery_point;

struct TestExceptions_F : public testing::Test {
	std::jmp_buf jmp_ptr;

	void SetUp() override {
		Tracer_t::ClearCallStack();
	};

	void do_return() {
		Tracer_t tg( __func__ );
	};

	[[noreturn]] void no_return() {
		Tracer_t tg( __func__ );
		std::longjmp( jmp_ptr, 1 );
	};

	void call_noreturn() {
		Tracer_t tg( __func__ );
		no_return();
	};
};

TEST_F( TestExceptions_F, normallyReturn ) {
	ostringstream o;
	Tracer_t::PrintCallStack( o );
	ASSERT_TRUE( o.str().empty() );

	Tracer_t::ClearCallStack();
	o.str( "" );
	do_return();
	Tracer_t::PrintCallStack( o );
	ASSERT_TRUE( o.str().empty() );
};

TEST_F( TestExceptions_F, diedAtLevel1 ) {
	ostringstream o;
	if( setjmp( jmp_ptr ) == 0 )
		no_return();
	else {
		Tracer_t::PrintCallStack( o );
		ASSERT_EQ( o.str(), "\tno_return()\n" );
	}
};

TEST_F( TestExceptions_F, diedAtLevel2 ) {
	ostringstream o;
	if( setjmp( jmp_ptr ) == 0 )
		call_noreturn();
	else {
		Tracer_t::PrintCallStack( o );
		ASSERT_EQ( o.str(), "\tcall_noreturn()\n\t\tno_return()\n" );
	}
};
