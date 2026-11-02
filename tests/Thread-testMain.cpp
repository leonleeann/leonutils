#include "leonutils/CallStack.hpp"
#include <iomanip>
#include <iostream>
#include <leonlog/LeonLog.hpp>
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
	TRACE_POINT;

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
	TRACE_POINT;

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
	TRACE_POINT;

	lg_debg << "1s后尝试throwRunTimeError...";
	sleep_for( 1s );

	throw std::runtime_error( "RunTimeError" );
};

void DoThrowInt() {
	TRACE_POINT;

	lg_debg << "1s后尝试throw int123...";
	sleep_for( 1s );

	throw 123;
};

int main() {
//	RegistThread( "主线程" );
	CatchGlobalSig( DefaultSigAction );

	Thread_t thrd1( "ThreadTester" );
	thrd1.start( DoDivByZeroBody,
	[]() { lg_note << "被0除 测试开始 ========================" << std::endl; },
	[]() { lg_note << "被0除 测试结束 ========================" << std::endl; }
			   );
	sleep_for( 3s );
	thrd1.stop();

	// 只给启动回调, 看看它会不会崩溃
	thrd1.start( DoSegFaultBody,
	[]() { lg_note << "野指针 测试开始 ========================" << std::endl; }
			   );
	sleep_for( 3s );
	thrd1.stop();
	lg_note << "野指针 测试结束 ========================" << std::endl;

	// 只给停止回调, 看看它会不会崩溃
	lg_note << "抛异常 测试开始 ========================" << std::endl;
	thrd1.start( DoThrowRunTimeError, nullptr,
	[]() { lg_note << "抛异常 测试结束 ========================" << std::endl; }
			   );
	sleep_for( 3s );
	thrd1.stop();

	// 启停回调都不给 看看它会不会崩溃
	lg_note << "抛整数 测试开始 ========================" << std::endl;
	thrd1.start( DoThrowInt, nullptr, nullptr );
	sleep_for( 3s );
	thrd1.stop();
	lg_note << "抛整数 测试结束 ========================" << std::endl;

	// 测试它自停之后还能不能正常退出(不崩溃)
	auto stop_self = [&]() {
		lg_debg << "1s后尝试自停...";
		sleep_for( 1s );
		thrd1.stopSelf();
	};
	thrd1.start( stop_self,
	[]() { lg_note << "自停 测试开始 ========================" << std::endl; },
	[]() { lg_note << "自停 测试结束 ========================" << std::endl; }
			   );
	sleep_for( 2s );
	thrd1.stop();

	lg_note << "========测试结束========================" << std::endl;

	RevocGlogalSig();
};

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
