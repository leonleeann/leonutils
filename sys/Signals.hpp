#pragma once
#include <setjmp.h>
#include <signal.h>

namespace leon_utl {

/*
 * 名词定义：
	1.常规信号：可由其它进程、线程发送，本进程自由选择哪个线程负责接收处理的OS信号。
		如:SIGHUP/SIGINT...
	2.异常信号：只能由触发线程自己处理的OS信号。
		如:SIGFPE/SIGSEGV等...此类信号通常都是由于本线程执行了非法指令(SIGILL)、访问了非
		法地址(SIGSEGV)、被零除(SIGFPE)等原因引发的异常,已经不能沿既定指令流继续。如果不做
		处理并完成longjump,整个进程就会崩溃。


 * 本模块为一个应用程序提供处理信号的统一方案,具体包括两部分:
	1.在主线程中统一接收、处理"常规信号"(如: SIGHUP, SIGINT, SIGQUIT等);
		由main()函数在初始化本模块时提供处理函数,也就是在调用 sigInitialize 时提
		供的参数 sig_action_f。
	2.在工作线程内处理它们自己触发的"异常信号"(如: SIGILL, SIGBUS, SIGFPE, SIGSEGV等)。
		发生这类信号,一般都无法继续执行,所以线程应事先设好 sig_recovery_point 变量,统一的
		信号处理函数其实就是一个siglongjmp(),给线程一个"安全退出/重启"的机会,避免整个进程
		崩溃。
*/

//====== 类型定义 ===============================================================
// 为增强可读性,干脆申明一个函数类型
using SigAction_f = void ( * )( int, siginfo_t*, void* );

//====== 函数申明 ===============================================================
// 设置统一信号捕获(全系统只应做一次！！！)
void SigInitialize( SigAction_f default_action );

// 为本线程放行"严重失败"信号(由 SigAction_f 来处理)
void EnablCritSig4Thread();
// 为本线程阻塞"严重失败"信号
void BlockCritSig4Thread();

std::string SigSet2Str( const sigset_t );

//====== 全局变量 ===============================================================
// 每线程独立的恢复点,当发生异常信号(SIGFPE/SIGSEGV等)无法继续执行时,就从恢复点继续执行.
extern thread_local sigjmp_buf sig_recovery_point;

}; //namespace leon_utl
