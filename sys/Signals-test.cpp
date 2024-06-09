#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <iomanip>
#include <iostream>
#include <signal.h>

#include "Signals.hpp"

using namespace std;
using namespace testing;

namespace leon_utl {

struct SignalsHandling_F : public ::testing::Test {

	void setupSigHandling() {
		// 设置信号处理
		SigInitialize( nullptr );

		// 放行线程内异常信号
		EnablCritSig4Thread();
	};

	void doDivByZero() {
		/* 任何小聪明计算都不能确定地导致 SIGFPE, 只能导致 UB
			int j = 1;
			j ^= j; // 自己和自己异或，就是为了得个零，直接写字面量0要被编译器警告
			cout << 123 / j << endl; // 必须要使用除法运算的结果！！！否则除法运算会被优化掉！
		*/

		// 这才是正经办法
		raise( SIGFPE );
	};

	void doSegFault() {
		/* g++-12 已经不让我们这样玩儿了，这样导致不了 Segment Fault
			int* pi = nullptr;
			*pi = 0;
			cout << pi[0] << endl;
		*/

		// 这才是正经办法
		raise( SIGSEGV );
	};
};

TEST_F( SignalsHandling_F, dyingWhenDivByZero ) {
	ASSERT_EXIT( {
		doDivByZero();
		exit( EXIT_SUCCESS );
	}, KilledBySignal( SIGFPE ), "" );
};

TEST_F( SignalsHandling_F, surviWhenDivByZero ) {
	ASSERT_EXIT( {
		// 设置信号处理
		setupSigHandling();
		// 设置恢复点
		if( sigsetjmp( sig_recovery_point, 1 ) == 0 )
			doDivByZero();

		exit( EXIT_SUCCESS );
	}, ExitedWithCode( EXIT_SUCCESS ), "" );
};

TEST_F( SignalsHandling_F, dyingWhenSegFault ) {
	ASSERT_EXIT( {
		doSegFault();
		exit( EXIT_SUCCESS );
	}, KilledBySignal( SIGSEGV ), "" );
};

TEST_F( SignalsHandling_F, surviWhenSegFault ) {
	ASSERT_EXIT( {
		// 设置信号处理
		setupSigHandling();
		// 设置恢复点
		if( sigsetjmp( sig_recovery_point, 1 ) == 0 )
			doSegFault();

		exit( EXIT_SUCCESS );
	}, ExitedWithCode( EXIT_SUCCESS ), "" );
};

}; //namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
