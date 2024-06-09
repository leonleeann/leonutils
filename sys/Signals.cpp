#include <fmt/format.h>
#include <iomanip>
#include <iostream>
#include <setjmp.h>
#include <string.h>	// strsignal
#include <string>

#include "Signals.hpp"

using std::string;

namespace leon_utl {

thread_local sigjmp_buf	sig_recovery_point;
thread_local bool		sig_being_processd = false;

// "严重失败信号"处理(不是常规信号) 能不能用[[noreturn]]还有待验证
[[noreturn]] void CriticalAction( int sig, siginfo_t*, void* ) {
//	if( sig_being_processd )
//		return;
	sig_being_processd = true;

	/* 出现严重错误的时候是否应该尽量避免创建新对象?
	string log_content = fmt::format( "失败信号:{}[{}]", strsignal( sig ), sig );
	std::cerr << log_content << std::endl;
	*/
	std::cerr << "失败信号:" << strsignal( sig ) << '[' << sig << ']' << std::endl;

	sig_being_processd = false;
	siglongjmp( sig_recovery_point, 1 );
};

void SigInitialize( SigAction_f def_ ) {
	struct sigaction new_act {};
	new_act.sa_flags = SA_SIGINFO | SA_NOCLDSTOP;
	new_act.sa_sigaction = def_;

	// 不打算在def_handler内处理的信号
	sigset_t blocks {};
	// 先是全集,后面逐项排除
	sigfillset( &blocks );

	/* 与blocks不同,new_act.sa_mask是在处理信号期间不希望传递的信号。因为所有的信号都不应
	 * 嵌套触发,所以在信号处理函数内部应直接屏蔽所有信号 */
	sigfillset( &new_act.sa_mask );

	sigaction( SIGHUP, &new_act, nullptr );   // 01
	sigdelset( &blocks, SIGHUP );

	sigaction( SIGINT, &new_act, nullptr );   // 02
	sigdelset( &blocks, SIGINT );

	sigaction( SIGQUIT, &new_act, nullptr );  // 03
	sigdelset( &blocks, SIGQUIT );

	sigaction( SIGTERM, &new_act, nullptr );  // 15
	sigdelset( &blocks, SIGTERM );

	sigaction( SIGCHLD, &new_act, nullptr );  // 17
	sigdelset( &blocks, SIGCHLD );

	// 在主线程中阻塞异常信号的传递(在主线程中即使触发这些异常,也没人处理了)
	pthread_sigmask( SIG_BLOCK, &blocks, nullptr );

	// 在fault_action中处理的异常信号
	new_act.sa_sigaction = CriticalAction;
	sigaction( SIGILL, &new_act, nullptr );   // 04
	sigaction( SIGFPE, &new_act, nullptr );   // 08
	sigaction( SIGSEGV, &new_act, nullptr );  // 11

	/* 好像实际运行中取到的常量值和/usr/include/x86_64-linux-gnu/bits/signum-generic.h
	   中定义的不一样(比如SIGBUS),这里只好用"神仙数"!
	 * 不打算处理的信号,暂时不管,列在此处备用
	   sigaction(  5, &new_act, nullptr );  // SIGTRAP
	   sigaction(  6, &new_act, nullptr );  // SIGABRT
	   sigaction(  7, &new_act, nullptr );  // SIGBUS
	   sigaction(  9, &new_act, nullptr );  // SIGKILL
	   sigaction( 10, &new_act, nullptr );  // SIGUSR1
	   sigaction( 12, &new_act, nullptr );  // SIGUSR2
	   sigaction( 13, &new_act, nullptr );  // SIGPIPE
	   sigaction( 14, &new_act, nullptr );  // SIGALRM
	   sigaction( 16, &new_act, nullptr );  // SIGSTKFLT
	   sigaction( 17, &new_act, nullptr );  // SIGCHLD
	   sigaction( 18, &new_act, nullptr );  // SIGCONT
	   sigaction( 19, &new_act, nullptr );  // SIGSTOP
	   sigaction( 20, &new_act, nullptr );  // SIGTSTP
	   sigaction( 21, &new_act, nullptr );  // SIGTTIN
	   sigaction( 22, &new_act, nullptr );  // SIGTTOU
	   sigaction( 23, &new_act, nullptr );  // SIGURG
	   sigaction( 24, &new_act, nullptr );  // SIGXCPU
	   sigaction( 25, &new_act, nullptr );  // SIGXFSZ
	   sigaction( 26, &new_act, nullptr );  // SIGVTALRM
	   sigaction( 27, &new_act, nullptr );  // SIGPROF
	   sigaction( 28, &new_act, nullptr );  // SIGWINCH
	   sigaction( 29, &new_act, nullptr );  // SIGPOLL
	   sigaction( 30, &new_act, nullptr );  // SIGPWR
	   sigaction( 31, &new_act, nullptr );  // SIGSYS
	 */
};

void EnablCritSig4Thread() {
	sigset_t blocks {};
	// 准备一个空集
	sigemptyset( &blocks );

	// 获取本线程的当前信号屏蔽集
	pthread_sigmask( SIG_SETMASK, nullptr, &blocks );

	// sigfillset( &blocks ); 不再从全集中逐项排除,改为从当前屏蔽集中精准排除
	sigdelset( &blocks, SIGILL );
	sigdelset( &blocks, SIGFPE );
	sigdelset( &blocks, SIGSEGV );

	// 在本线程中仅放行上述信号,其它信号配置不变
	pthread_sigmask( SIG_SETMASK, &blocks, nullptr );
};

void BlockCritSig4Thread() {
	sigset_t blocks {};
	// 准备一个空集
	sigemptyset( &blocks );

	// 获取本线程的当前信号屏蔽集
	pthread_sigmask( SIG_SETMASK, nullptr, &blocks );

	// 注意是 sigaddset, 放行的时候是 sigdelset
	sigaddset( &blocks, SIGILL );
	sigaddset( &blocks, SIGFPE );
	sigaddset( &blocks, SIGSEGV );

	// 不再从全集中逐项添加,只精准添加,其它信号配置不变
	pthread_sigmask( SIG_SETMASK, &blocks, nullptr );
};

string SigSet2Str( const sigset_t ss_ ) {
	string str_sigs = "{";
	for( int j = 1; j < SIGRTMIN; ++j )
		if( sigismember( &ss_, j ) )
			str_sigs += fmt::format( "\n{:02d}:{},", j, strsignal( j ) );

	return str_sigs + "\n}";
};

}; //namespace leon_utl
// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
