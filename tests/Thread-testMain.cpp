#include <leonlog/LeonLog.hpp>
#include <iomanip>
#include <iostream>
#include <signal.h>
#include <string.h>
#include <string>
#include <thread>

#include "leonutils/Signals.hpp"
#include "leonutils/Thread.hpp"

using namespace leon_utl;
using namespace leon_log;
using namespace std::this_thread;
using std::to_string;

void DefaultSigAction( int sig, siginfo_t*, void* ) {
	str_t log_content = "收到信号:" + str_t( strsignal( sig ) )
						 + '[' + to_string( sig ) + ']';
	lg_erro << log_content;
};

void DoDivByZeroBody() {
	lg_debg << "1s后尝试被0除...";
	sleep_for( 1s );

	/* 任何小聪明计算都不能确定地导致 SIGFPE, 只能导致 UB
	int j = 1; j ^= j; // 自己和自己异或，就是为了得个零，直接写字面量0要被编译器警告
	lg_debg << 123 / j; // 必须要使用除法运算的结果！！！否则除法运算会被优化掉！
	*/
	// 这才是正经办法
	raise( SIGFPE );
};

void DoSegFaultBody() {
	lg_debg << "1s后尝试SegFault...";
	sleep_for( 1s );

	/* g++-12 已经不让我们这样玩儿了，这样导致不了 Segment Fault
	char_cp msg { "这是一个不能修改的字符串" };
	char*	buf = const_cast<char*>( msg );
	buf[0] = 'A';
	lg_debg << buf;
	*/
	// 这才是正经办法
	raise( SIGSEGV );
};

void DoThrowRunTimeError() {
	lg_debg << "1s后尝试throwRunTimeError...";
	sleep_for( 1s );

	throw std::runtime_error( "RunTimeError" );
};

void DoThrowInt() {
	lg_debg << "1s后尝试throw int123...";
	sleep_for( 1s );

	throw 123;
};

int main() {
//	RegistThread( "主线程" );
	CatchGlobalSig( DefaultSigAction );

	Thread_t thrd1( "ThreadTester" );
	thrd1.start( DoDivByZeroBody );
	sleep_for( 3s );
	thrd1.stop();
	lg_note << "========================" << std::endl;

	thrd1.start( DoSegFaultBody );
	sleep_for( 3s );
	thrd1.stop();
	lg_note << "========================" << std::endl;

	thrd1.start( DoThrowRunTimeError );
	sleep_for( 3s );
	thrd1.stop();
	lg_note << "========================" << std::endl;

	thrd1.start( DoThrowInt );
	sleep_for( 3s );
	thrd1.stop();
	lg_note << "========================" << std::endl;

	RevocGlogalSig();
};

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
