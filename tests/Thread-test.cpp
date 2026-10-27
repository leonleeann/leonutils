#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <leonlog/LeonLog.hpp>
#include <atomic>
#include <thread>

#include "MemoryOrder.hpp"
#include "Thread.hpp"
#include "misc/Signals.hpp"

using namespace leon_utl;
using namespace leon_log;
using namespace std::chrono_literals;
using namespace std::this_thread;
using namespace std;
using namespace testing;
using std::atomic_bool;
using std::function;
using std::thread;

TEST( TestThread, initialStatus ) {
	Thread_t thrd1( "TestThread.initialStatus" );
	ASSERT_EQ( thrd1.name(), "TestThread.initialStatus" );
	ASSERT_FALSE( thrd1.shouldRun() );
	ASSERT_FALSE( thrd1.isRunning() );
};

TEST( TestThread, starting ) {
	Thread_t thrd1( "TestThread.starting" );

	// 线程启动就置位标志,test者看到标志就验证
// 	atomic_bool run_ack { false };
	thrd1.start( []() {
// 		run_ack.store( true, mo_release );
		sleep_for( 2ms );
	} );
	ASSERT_TRUE( thrd1.shouldRun() );
	ASSERT_TRUE( thrd1.isRunning() );

	thrd1.stop();
	ASSERT_FALSE( thrd1.shouldRun() );
	ASSERT_FALSE( thrd1.isRunning() );
};

TEST( TestThread, stopping ) {
	Thread_t thrd1( "TestThread.stopping" );

	// 测试者需要停止时先置位标志,线程看到标志就退出
	atomic_bool ask_stop { false };
	thrd1.start( [&ask_stop]() {
		while( ! ask_stop.load() ) sleep_for( 1ns );
	} );
	ASSERT_TRUE( thrd1.shouldRun() );
	ASSERT_TRUE( thrd1.isRunning() );

	ask_stop.store( true, mo_release );
	thrd1.stop( 1s );
	ASSERT_FALSE( thrd1.shouldRun() );
	ASSERT_FALSE( thrd1.isRunning() );
};

TEST( TestThread, killing ) {
	Thread_t thrd1( "TestThread.killing" );

	thrd1.start( []() {
		while( true ) sleep_for( 1000s );
	} );
	ASSERT_TRUE( thrd1.shouldRun() );
	ASSERT_TRUE( thrd1.isRunning() );

	thrd1.stop( 1ms );
	ASSERT_FALSE( thrd1.shouldRun() );
	ASSERT_FALSE( thrd1.isRunning() );
};

/*
TEST( TestThread, captureStrException ) {
	Thread_t thrd1( "TestThread.raising" );

	thrd1.start( []() {
		sleep_for( 2ms );
		throw std::runtime_error( "raised for testing" );
	} );
	thrd1.stop();
};

TEST( TestThread, captureIntException ) {
	Thread_t thrd1( "TestThread.raising" );

	thrd1.start( []() {
		sleep_for( 2ms );
		throw int( 123 );
	} );
	thrd1.stop();
};

TEST( TestThread, segfault ) {
	Thread_t thrd1( "TestThread.raising" );

	char* nul_pt = nullptr;
	thrd1.start( [nul_pt]() {
		lg_debg << nul_pt[0];
	} );

	thrd1.stop();
};

TEST( TestThread, floatError ) {
	Thread_t thrd1( "TestThread.raising" );

	int zero = 0;
	thrd1.start( [zero]() {
		lg_debg << 1 / zero;
	} );

	thrd1.stop();
};
*/

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
