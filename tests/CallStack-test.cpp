#include <csetjmp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sstream>

#include "leonutils/CallStack.hpp"

using namespace leon_utl;
using oss_t = std::ostringstream;
using str_t = std::string;

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
	oss_t oss;
	Tracer_t::PrintCallStack( oss );
	ASSERT_TRUE( oss.str().empty() );

	Tracer_t::ClearCallStack();
	oss.str( "" );
	do_return();
	Tracer_t::PrintCallStack( oss );
	ASSERT_TRUE( oss.str().empty() );
};

TEST_F( TestExceptions_F, diedAtLevel1 ) {
	oss_t oss;
	if( setjmp( jmp_ptr ) == 0 )
		no_return();
	else {
		Tracer_t::PrintCallStack( oss );
		ASSERT_EQ( oss.str(), "\tno_return()\n" );
	}
};

TEST_F( TestExceptions_F, diedAtLevel2 ) {
	oss_t oss;
	if( setjmp( jmp_ptr ) == 0 )
		call_noreturn();
	else {
		Tracer_t::PrintCallStack( oss );
		ASSERT_EQ( oss.str(), "\tcall_noreturn()\n\t\tno_return()\n" );
	}
};
