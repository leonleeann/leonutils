#include <atomic>
#include <iomanip>
#include <iostream>
#include <map>
#include <setjmp.h>
#include <sstream>
#include <string.h>	// strsignal

#include "leonutils//Exceptions.hpp"
#include "leonutils/Converts.hpp"
#include "leonutils/Signals.hpp"

std::bitset<32> SigMask( sigset_t set_ ) {
	std::bitset<32> mask {};
	for( int i = 0; i < 32; ++i )
		mask[i] = sigismember( &set_, i ) != 0;
	return mask;
};

std::ostream& operator<<( std::ostream& os_, const leon_utl::SigAction_t& sa_ ) {

	uintptr_t func = reinterpret_cast<uintptr_t>( sa_.sa_sigaction );
	os_ << "{ flag:" << std::hex << std::setfill( '0' )
		<< std::setw( sizeof( sa_.sa_flags ) * 2 ) << sa_.sa_flags
		<< ", func:" << std::setw( 16 ) << func
		<< ", mask:" << SigMask( sa_.sa_mask )
		<< '}';
	return os_;
};

namespace leon_utl {

// 把不同信号处理行为备份到一个 map 中, 方便查找
using SigActions_t = std::map<int, SigAction_t>;

// 各种信号的默认处理器备份
SigActions_t		s_sa_backups {};
// 进程信号屏蔽集备份
sigset_t			s_blocks_bak {};

// 全局信号处理已被本模块接管
std::atomic_bool	s_taked_over {};

// 当前线程"崩溃信号"处理已开放
thread_local bool		tl_taked_over {};
// 当前线程信号屏蔽集备份
thread_local sigset_t	tl_blocks_bak {};

// 线程 long jump 目标点
thread_local sigjmp_buf	tl_recovery_point;
// 崩溃计次
thread_local int		tl_crashing_count { 0 };

bool GlobalSigCatching() {
	return s_taked_over.load();
};

// "崩溃信号"处理函数
[[noreturn]] void CrashAction( int sig_, siginfo_t*, void* ) {
	// 尽量不在这里面创建新对象
	std::cerr << "tid:" << gettid() << "发生崩溃:" << sig_
			  << "(\"" << strsignal( sig_ ) << "\")." << std::endl;
	siglongjmp( tl_recovery_point, ++tl_crashing_count );
};

// 备份单个信号处理
void Backup( int sig_ ) {
	auto& bak = s_sa_backups[sig_];
	sigaction( sig_, nullptr, &bak );

//	std::cerr << pad_r( strsignal( sig_ ), 28 ) << "备份为:" << bak << std::endl;
};

// 接管单个信号处理
void Takeover( int sig_, SigAction_t& new_, sigset_t& blocks_ ) {
	sigdelset( &blocks_, sig_ );
	sigaction( sig_, &new_, nullptr );

//	std::cerr << pad_r( strsignal( sig_ ), 28 ) << "设置为:" << new_ << std::endl;
};

void CatchGlobalSig( SigAction_f def_ ) {
	if( s_taked_over.load() )
		throw bad_usage( "全局信号处理已经设置,不能重复设置!" );

	s_sa_backups.clear();
	SigAction_t new_act {};

	// 我们不需要知道子进程"暂停/恢复",否则太多了,所以加上 SA_NOCLDSTOP
	new_act.sa_flags = SA_SIGINFO | SA_NOCLDSTOP;
	new_act.sa_sigaction = def_;

	// 不打算在 def_ 内处理的信号
	sigset_t blocks {};
	// 先是全集,后面逐项排除
	sigfillset( &blocks );
	/* 与blocks不同, new_act.sa_mask 是在处理信号期间不希望传递的信号。因为所有的信号
		都不应嵌套触发, 所以在信号处理函数内部应直接屏蔽所有信号 */
	sigfillset( &new_act.sa_mask );

	Backup( SIGHUP );	// 01
	Backup( SIGINT );	// 02
	Backup( SIGQUIT );	// 03
	Backup( SIGTERM );	// 15
	Backup( SIGCHLD );	// 17
	Backup( SIGILL );	// 04
	Backup( SIGFPE );	// 08
	Backup( SIGSEGV );	// 11

	Takeover( SIGHUP,  new_act, blocks );	// 01
	Takeover( SIGINT,  new_act, blocks );	// 02
	Takeover( SIGQUIT, new_act, blocks );	// 03
	Takeover( SIGTERM, new_act, blocks );	// 15
	Takeover( SIGCHLD, new_act, blocks );	// 17

	// 在主线程中阻塞"崩溃信号"的传递(在主线程中即使触发这些异常,也没人处理了)
	pthread_sigmask( SIG_BLOCK, &blocks, &s_blocks_bak );

	// 在 CrashAction 中处理的崩溃信号
	new_act.sa_sigaction = CrashAction;
	Takeover( SIGILL,  new_act, blocks );	// 04
	Takeover( SIGABRT, new_act, blocks );	// 06
	Takeover( SIGFPE,  new_act, blocks );	// 08
	Takeover( SIGSEGV, new_act, blocks );	// 11

	s_taked_over = true;
//	std::cerr << "全局信号捕获mask已由:\n" << SigMask( s_blocks_bak )
//			  << "\n设置为:\n" << SigMask( blocks ) << std::endl;
};

/* 好像实际运行中取到的常量值和 /usr/include/x86_64-linux-gnu/bits/signum-generic.h
	中定义的不一样(比如SIGBUS),这里只好用"神仙数"!
	!!! 所有可能的信号值,列在此处备用, 不要删 !!!
	Takeover( SIGHUP   , new_act, blocks );	// 01
	Takeover( SIGINT   , new_act, blocks );	// 02
	Takeover( SIGQUIT  , new_act, blocks );	// 03
	Takeover( SIGILL   , new_act, blocks );	// 04
	Takeover( SIGTRAP  , new_act, blocks );	// 05
	Takeover( SIGABRT  , new_act, blocks );	// 06
	Takeover( SIGBUS   , new_act, blocks );	// 07
	Takeover( SIGFPE   , new_act, blocks );	// 08
	Takeover( SIGKILL  , new_act, blocks );	// 09
	Takeover( SIGUSR1  , new_act, blocks );	// 10
	Takeover( SIGSEGV  , new_act, blocks );	// 11
	Takeover( SIGUSR2  , new_act, blocks );	// 12
	Takeover( SIGPIPE  , new_act, blocks );	// 13
	Takeover( SIGALRM  , new_act, blocks );	// 14
	Takeover( SIGTERM  , new_act, blocks );	// 15
	Takeover( SIGSTKFLT, new_act, blocks );	// 16
	Takeover( SIGCHLD  , new_act, blocks );	// 17
	Takeover( SIGCONT  , new_act, blocks );	// 18
	Takeover( SIGSTOP  , new_act, blocks );	// 19
	Takeover( SIGTSTP  , new_act, blocks );	// 20
	Takeover( SIGTTIN  , new_act, blocks );	// 21
	Takeover( SIGTTOU  , new_act, blocks );	// 22
	Takeover( SIGURG   , new_act, blocks );	// 23
	Takeover( SIGXCPU  , new_act, blocks );	// 24
	Takeover( SIGXFSZ  , new_act, blocks );	// 25
	Takeover( SIGVTALRM, new_act, blocks );	// 26
	Takeover( SIGPROF  , new_act, blocks );	// 27
	Takeover( SIGWINCH , new_act, blocks );	// 28
	Takeover( SIGPOLL  , new_act, blocks );	// 29
	Takeover( SIGPWR   , new_act, blocks );	// 30
	Takeover( SIGSYS   , new_act, blocks );	// 31
*/

void RevocGlogalSig() {
	if( ! s_taked_over.load() )
		throw bad_usage( "全局信号捕获并未设置,不能恢复!" );

	for( const auto& [sig, sa] : s_sa_backups ) {
		sigaction( sig, &sa, nullptr );
//		std::cerr << pad_r( strsignal( sig ), 28 ) << "恢复为:" << sa << std::endl;
	}
	s_sa_backups.clear();

	sigset_t cur {};
	pthread_sigmask( SIG_BLOCK, &s_blocks_bak, &cur );
//	pthread_sigmask( SIG_BLOCK, nullptr, &s_blocks_bak );
	s_taked_over = false;
//	std::cerr << "全局信号捕获mask已由:\n" << SigMask( cur )
//			  << "\n恢复为:\n" << SigMask( s_blocks_bak ) << std::endl;
};

bool CrashCatching() {
	return tl_taked_over;
};

void CatchCrashSig() {
	if( tl_taked_over )
		return;

	sigset_t blocks {};
	// 准备一个空集
	sigemptyset( &blocks );
	tl_blocks_bak = blocks;

	// 获取本线程的当前信号屏蔽集
	pthread_sigmask( SIG_SETMASK, nullptr, &blocks );

	// sigfillset( &blocks ); 不再从全集中逐项排除,改为从当前屏蔽集中精准排除
	sigdelset( &blocks, SIGILL );
	sigdelset( &blocks, SIGABRT );
	sigdelset( &blocks, SIGFPE );
	sigdelset( &blocks, SIGSEGV );

	// 在本线程中仅放行上述信号,其它信号配置不变
	pthread_sigmask( SIG_SETMASK, &blocks, &tl_blocks_bak );

	tl_taked_over = true;
	std::cerr << "线程崩溃保护已设置." << std::endl;
};

void RevocCrashSig() {
	if( !tl_taked_over )
		return;

	pthread_sigmask( SIG_SETMASK, &tl_blocks_bak, nullptr );

	tl_taked_over = false;
	std::cerr << "线程崩溃保护已撤销." << std::endl;
};

str_t SigSet2Str( const sigset_t ss_ ) {
	std::ostringstream oss;
	oss << "{";

	for( int j = 1; j < SIGRTMIN; ++j )
		if( sigismember( &ss_, j ) )
//			oss << fmt::format( "\n{:02d}:{},", j, strsignal( j ) );
			oss << '\n' << format( j, 2, 0 ) << ':' << strsignal( j );
	oss << "\n}";

	return oss.str();
};

}; //namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
