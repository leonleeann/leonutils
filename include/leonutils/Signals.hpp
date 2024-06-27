#pragma once
#include <bitset>
#include <functional>
#include <setjmp.h>
#include <signal.h>
#include <string>

namespace leon_utl {

using str_t = std::string;

/*
 * 名词定义:
	1.常规信号: 可由其它进程、线程发送，本进程自由选择哪个线程负责接收处理的OS信号。
		如:SIGHUP/SIGINT...
	2.崩溃信号: 应由触发线程自己处理的OS信号。
		如:SIGFPE/SIGSEGV等...此类信号通常都是由于本线程执行了非法指令(SIGILL)、访问了非
		法地址(SIGSEGV)、被零除(SIGFPE)等原因引发的异常,已经不能沿既定指令流继续。如果不做
		处理并完成 longjump, 整个进程就会崩溃。

 * 本模块为一个应用程序提供处理信号的统一方案,具体包括两部分:
	1.在主线程中统一接收、处理"常规信号"(如: SIGHUP, SIGINT, SIGQUIT等);
		由main()函数在初始化本模块时提供处理函数,也就是在调用 CatchGlobalSig 时提
		供的参数 default_action.
	2.在工作线程内处理它们自己触发的"崩溃信号"(如: SIGILL, SIGBUS, SIGFPE, SIGSEGV).
		发生这类信号,一般都无法继续执行,所以线程应事先设好 tl_recovery_point 变量,统一
		的信号处理函数其实就是一个 siglongjmp(), 给线程一个"安全退出/重启"的机会,避免整
		个进程崩溃。
*/

//====== 类型定义 ===============================================================
// using SigAction_f = void ( * )( int, siginfo_t*, void* );
using SigAction_f = std::function<void( int, siginfo_t*, void* )>;
using SigAction_t = struct sigaction;

//====== 函数申明 ===============================================================
// 整个进程信号处理已接管
bool GlobalSigCatching();
// 接管整个进程的信号处理(全系统只应做一次!)
void CatchGlobalSig( SigAction_f default_action );
// 撤销整个进程的信号处理(全系统只应做一次!)
void RevocGlogalSig();

// 当前线程"崩溃保护"已启用
bool CrashCatching();
// 启用当前线程"崩溃保护", longjump 到 tl_recovery_point
void CatchCrashSig();
// 停用当前线程"崩溃保护", 整个 app 会崩溃
void RevocCrashSig();

// 转换信号集到字符串,方便输出
str_t SigSet2Str( const sigset_t );

//====== 全局变量 ===============================================================
// 每线程独立的恢复点,当发生崩溃信号(SIGFPE/SIGSEGV等)无法继续执行时,就从恢复点继续执行.
extern thread_local sigjmp_buf tl_recovery_point;

}; //namespace leon_utl

std::bitset<32> SigMask( sigset_t );
std::ostream& operator<<( std::ostream&, const leon_utl::SigAction_t& );

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
