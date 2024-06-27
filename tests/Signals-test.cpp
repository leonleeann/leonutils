#include <atomic>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <iomanip>
#include <iostream>
#include <signal.h>
#include <unistd.h>

#include "leonutils/Converts.hpp"
#include "leonutils/Signals.hpp"

using namespace std::chrono;
using namespace std::chrono_literals;
using namespace std;
using namespace testing;

namespace leon_utl {

constexpr nanoseconds CHK_TIMEOUT = 1000s;

atomic_int s_last_sig { 0 };

void DefaultAction( int sig_, siginfo_t*, void* ) {
	std::cerr << "tid:" << gettid() << "遭遇信号:" << sig_ << ':'
			  << strsignal( sig_ ) << std::endl;
	s_last_sig = sig_;
};

bool SurviveWith( int sig_ ) {
	if( sigsetjmp( tl_recovery_point, 1 ) == 0 )
		// 没有保护的话,到此就崩溃了,外面得不到返回值:true, 只会得到一个非0的 exit_code.
		raise( sig_ );

	/* 如果保护起作用了, 外面就能拿得到这里的返回值:true */
	return true;
};

TEST( TestSigHandling, crashWithoutCatching ) {
	/* GTest/GMock 对崩溃信息做了替换, 所以我们拿到的是 GTest/GMock 生成的信息.
		比如:
			"Terminated by signal 4 (core dumped)"
			"Terminated by signal 8 (core dumped)"
			"Terminated by signal 11 (core dumped)"
		而系统的信息是:
			"Illegal instruction (core dumped)"
			"Floating point exception (core dumped)"
			"Segmentation fault (core dumped)"

		所以像下面这样是匹配不到的:
		EXPECT_EXIT( SurviveWith( SIGFPE ), KilledBySignal( SIGFPE ),
			"Floating point exception (core dumped)" );
		干脆就用空串,反而能匹配.
	*/

	// 没有信号处理时, 遇到信号应该崩溃
	EXPECT_EXIT( SurviveWith( SIGINT ),		KilledBySignal( SIGINT ), "" );
	EXPECT_EXIT( SurviveWith( SIGILL ),		KilledBySignal( SIGILL ), "" );
	EXPECT_EXIT( SurviveWith( SIGFPE ),		KilledBySignal( SIGFPE ), "" );
	EXPECT_EXIT( SurviveWith( SIGSEGV ),	KilledBySignal( SIGSEGV ), "" );
};

struct SigHandling_F : public ::testing::Test {
	pid_t		_thread_id;
	atomic_int	_crash_cnt {};
	atomic_bool	_running {};
	atomic_bool	_exited {};

	void threadBody() {
		_thread_id = gettid();
		std::cerr << "子线程pid:" << getpid() << ",\ttid:" << _thread_id << std::endl;
		CatchCrashSig();

		_crash_cnt = sigsetjmp( tl_recovery_point, 1 );
		if( _crash_cnt.load() >= 9 ) {
			if( CrashCatching() )
				RevocCrashSig();
			_exited = true;
			return;
		}
		// 崩溃次数不够,就继续运行
		_running = true;
		while( true )
			std::this_thread::sleep_for( 1s );
	};

	bool waitRunning() {
		auto dead_line = system_clock::now() + CHK_TIMEOUT;
		while( ! _running.load() && system_clock::now() < dead_line )
			std::this_thread::yield();
		return _running.load();
	};

	bool waitExited() {
		auto dead_line = system_clock::now() + CHK_TIMEOUT;
		while( ! _exited.load() && system_clock::now() < dead_line ) {
			std::this_thread::yield();
		}
		return _exited.load();
	};

	void SetUp() override {
		if( CrashCatching() )
			RevocCrashSig();
		if( GlobalSigCatching() )
			RevocGlogalSig();
		s_last_sig = 0;
	};

	void TearDown() override {
		if( CrashCatching() )
			RevocCrashSig();
		if( GlobalSigCatching() )
			RevocGlogalSig();
	};
};

TEST_F( SigHandling_F, backupAndRestore ) {
	SigAction_t sa_bak {}, sa_new {};

	// 初始应该没有任何 handler
	ASSERT_FALSE( GlobalSigCatching() );

	sigaction( SIGHUP, nullptr, &sa_bak );
	std::cerr << pad_r( strsignal( SIGHUP ), 28 ) << "备份为:" << sa_bak << std::endl;
	ASSERT_EQ( sa_bak.sa_flags, 0 );
	ASSERT_EQ( sa_bak.sa_sigaction, nullptr );

	// 设置
	CatchGlobalSig( DefaultAction );
	ASSERT_TRUE( GlobalSigCatching() );

	// 检查
	sigaction( SIGHUP, nullptr, &sa_new );
	std::cerr << pad_r( strsignal( SIGHUP ), 28 ) << "设置为:" << sa_new << std::endl;

	// 不知为何, sa_flags 高位上总会多个 0x04000000.
	sa_bak.sa_flags |= 0x04000000;
	ASSERT_EQ( sa_new.sa_flags, sa_bak.sa_flags | SA_SIGINFO | SA_NOCLDSTOP );
	/* Signals 模块有一个"接口层", 这里获取的处理函数不可能是我们自定义的了.
	ASSERT_EQ( sa_new.sa_sigaction, DefaultAction ); */
	ASSERT_NE( sa_new.sa_sigaction, sa_bak.sa_sigaction );

	// 恢复
	RevocGlogalSig();

	// 检查
	sigaction( SIGHUP, nullptr, &sa_new );
	std::cerr << pad_r( strsignal( SIGHUP ), 28 ) << "恢复为:" << sa_new << std::endl;
	ASSERT_EQ( sa_new.sa_flags, sa_bak.sa_flags );
	ASSERT_EQ( sa_new.sa_sigaction, nullptr );
};

TEST_F( SigHandling_F, withCrashSigs ) {
	// 有崩溃保护时, 遇到信号应该存活, 这里就是正常退出
	CatchGlobalSig( DefaultAction );
	CatchCrashSig();
	ASSERT_TRUE( SurviveWith( SIGILL ) );
	ASSERT_TRUE( SurviveWith( SIGFPE ) );
	ASSERT_TRUE( SurviveWith( SIGSEGV ) );
	ASSERT_TRUE( SurviveWith( SIGHUP ) );	ASSERT_EQ( s_last_sig.load(), SIGHUP );
	ASSERT_TRUE( SurviveWith( SIGINT ) );	ASSERT_EQ( s_last_sig.load(), SIGINT );
	ASSERT_TRUE( SurviveWith( SIGQUIT ) );	ASSERT_EQ( s_last_sig.load(), SIGQUIT );
	ASSERT_TRUE( SurviveWith( SIGTERM ) );	ASSERT_EQ( s_last_sig.load(), SIGTERM );
	ASSERT_TRUE( SurviveWith( SIGCHLD ) );	ASSERT_EQ( s_last_sig.load(), SIGCHLD );
};

// 测试在子线程里面保护
TEST_F( SigHandling_F, protectThread ) {
	std::cerr << "主线程pid:" << getpid() << std::endl;
	auto thrd = thread( &SigHandling_F::threadBody, this );

	// 有崩溃保护时, 遇到信号应该存活, 所以线程会持续运行
	CatchGlobalSig( DefaultAction );

	// 3次 SIGILL
	for( int i = 0; i < 3; ++i ) {
		ASSERT_TRUE( waitRunning() );
		ASSERT_FALSE( _exited.load() );
		_running = false;
		kill( _thread_id, SIGILL );
	}

	// 3次 SIGFPE
	for( int i = 0; i < 3; ++i ) {
		ASSERT_TRUE( waitRunning() );
		ASSERT_FALSE( _exited.load() );
		_running = false;
		kill( _thread_id, SIGFPE );
	}

	// 3次 SIGSEGV
	for( int i = 0; i < 3; ++i ) {
		ASSERT_TRUE( waitRunning() );
		ASSERT_FALSE( _exited.load() );
		_running = false;
		kill( _thread_id, SIGSEGV );
	}
	ASSERT_TRUE( waitExited() );
	ASSERT_FALSE( _running.load() );
	ASSERT_EQ( _crash_cnt.load(), 9 );

	thrd.join();
};

}; //namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
