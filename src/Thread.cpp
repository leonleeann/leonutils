#include <atomic>
#include <iomanip>
#include <iostream>
#include <leonlog/LeonLog.hpp>
#include <thread>

#include "leonutils/CallStack.hpp"
#include "leonutils/Exceptions.hpp"
#include "leonutils/MemoryOrder.hpp"
#include "leonutils/Signals.hpp"
#include "leonutils/Thread.hpp"

using namespace leon_log;
using namespace std::this_thread;
using std::atomic_bool;
using std::function;
using std::thread;

namespace leon_utl {

const ns Thread_t::DEFAULT_WAIT_TIME = 1s;

struct Thread_t::Imp_t {
	str_t			_my_name;
	// 系统线程对象
	std::thread		_my_thrd;

	// 需要在线程内做的事
	function<void()> _my_job;

	// 超过此失败次数, 就退出
	int				_max_err { 3 };
	int				_err_cnt {};

	// 本线程正在运行
	atomic_bool		_running { false };

	// 外部告知本线程应该继续运行
	atomic_bool		_shldrun { false };

//======== 方法 =========================================
	// 线程体(传递给std::thread的)
	void threadBody();
	void killThread();
};

void Thread_t::Imp_t::threadBody() {
	leon_log::RegistThread( _my_name );
	_err_cnt = 0;

	bool st = false;
	if( ! _running.compare_exchange_strong( st, true ) ) {
		lg_erro << "我被重复启动!";
		return;
	}
	lg_debg << "我[" << _my_thrd.native_handle() << "]已启动";

	CatchCrashSig();	// 必须要在 sigsetjmp 之前, 否则捕获不了!!!
	// 异常恢复点
	if( sigsetjmp( tl_recovery_point, 1 ) != 0 ) {
		if( ++_err_cnt > _max_err )
			_shldrun = false;
		lg_erro << "我[" << _my_thrd.native_handle()
				<< "]遭遇了SegFault或FPE异常,次数:" << _err_cnt;
	}

	while( _shldrun.load( mo_acquire ) )
		try {
			_my_job();
		} catch( const std::exception& e ) {
			if( ++_err_cnt > _max_err )
				_shldrun = false;
			lg_erro << "遭遇异常:" << e.what() << ",次数:" << _err_cnt;
		} catch( ... ) {
			if( ++_err_cnt > _max_err )
				_shldrun = false;
			lg_erro << "遭遇未知异常,次数:" << _err_cnt;
		}

	if( _err_cnt >= _max_err )
		lg_fatl << "我[" << _my_thrd.native_handle() << "]遭遇多次异常,只能退出.";
	else
		lg_debg << "我[" << _my_thrd.native_handle() << "]已正常结束.";
	_running = false;
	RevocCrashSig();
};

void Thread_t::Imp_t::killThread() {
	pthread_cancel( _my_thrd.native_handle() );
	_my_thrd.detach();
	_running = false;
	_my_thrd = {};
};

Thread_t::Thread_t( str_cr name_ ) {
	_imp = new Imp_t;
	_imp->_my_name = name_;
};

Thread_t::~Thread_t() {
	if( _imp->_running ) {
		/* 如果没有stop本对象就销毁,是个错误.但没法靠抛异常来提醒使用者了,因为析构器内不宜抛异常!
		   throw leon_utl::bad_usage( "没有停止" + _imp->my_name + "!!!" ); */
		lg_erro << _imp->_my_name << '[' << _imp->_my_thrd.native_handle()
				<< "]没有得到正确停止!";
	}
	stop();

	delete _imp;
};

str_cr Thread_t::name() const {
	return _imp->_my_name;
};

bool Thread_t::isRunning() const {
	return _imp->_running.load( mo_acquire );
};

bool Thread_t::shouldRun() const {
	return _imp->_shldrun.load( mo_acquire );
};

bool Thread_t::start( function<void()> to_do_body_, ns timeout_, int max_err_ ) {
	if( _imp->_shldrun.load() ) {
		lg_erro << "重复启动" << _imp->_my_name;
		return true;
	}

	_imp->_my_job = to_do_body_;
	_imp->_max_err = max_err_;
	_imp->_shldrun = true;
	lg_debg << "将要启动" << _imp->_my_name << "...";
	_imp->_my_thrd = std::thread( &Thread_t::Imp_t::threadBody, _imp );

	// 等待子线程启动
	auto dead_line = steady_clock::now() + timeout_;
#ifdef DEBUG
	dead_line += 300s; // DEBUG单步跟踪时,要人工检查各变量,所以需要很长时间来启动
#endif
	while( ! _imp->_running.load() && steady_clock::now() < dead_line )
		sleep_for( 1us );

	// 如果1s之后, 子线程都还未启动, 就认为它出问题了, 就要杀了它
	if( ! _imp->_running.load() ) {
		_imp->killThread();
		_imp->_shldrun = false;
		lg_erro << _imp->_my_name << "启动失败!";
		return false;
	}

	lg_debg << _imp->_my_name << "已启动";
	return true;
};

void Thread_t::stop( ns timeout_ ) {
	if( ! _imp->_shldrun.load( mo_acquire ) )
		return;

	lg_debg << "将要停止" << _imp->_my_name << "...";
	_imp->_shldrun.store( false, mo_release );

	// 等待子线程停止
	auto dead_line = steady_clock::now() + timeout_;
	while( _imp->_running.load( mo_acquire ) && steady_clock::now() < dead_line )
		sleep_for( 1us );

	// 如果1s之后, 子线程都还在运行, 就认为它出问题了, 就要杀了它
	if( _imp->_running.load() ) {
		lg_erro << _imp->_my_name << "停止超时,开始强杀...";
		_imp->killThread();
	} else
		_imp->_my_thrd.join();

	lg_debg << "已停止" << _imp->_my_name;
};

};	// namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
