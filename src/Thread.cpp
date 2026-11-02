#include <atomic>
#include <iomanip>
#include <iostream>
#include <leonlog/LeonLog.hpp>
#include <thread>

#include "leonutils/CallStack.hpp"
#include "leonutils/CpuAffinity.hpp"
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
	str_t			_on_cpus;
	// 系统线程对象
	std::thread		_my_thrd;

	// 需要在线程内做的事
	CallBack_f		_loop_body;
	CallBack_f		_beg_body;
	CallBack_f		_end_body;

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
	RegistThread( _my_name );
	PthreadOnlyCPU( _on_cpus );
	if( _beg_body != nullptr )
		_beg_body();

	_err_cnt = 0;

	bool st = false;
	if( ! _running.compare_exchange_strong( st, true ) ) {
		lg_erro << "我被重复启动!";
		return;
	}
	lg_debg << "我[" << _my_thrd.native_handle() << "]已启动,绑核结果:{"
			<< GetThreadCpuMask() << "}.";

	CatchCrashSig();	// 必须要在 sigsetjmp 之前, 否则捕获不了!!!
	// 异常恢复点
	if( sigsetjmp( tl_recovery_point, 1 ) != 0 ) {
		++_err_cnt;
		{
			Log_t log( LogLevel_e::Error );
			log << "我[" << _my_thrd.native_handle()
				<< "]遭遇了SegFault或FPE异常,次数:" << _err_cnt << ",CallStack:\n";
			Tracer_t::PrintCallStack( log );
		}
		if( _err_cnt > _max_err )
			_shldrun = false;
	}
	Tracer_t::ClearCallStack();
	lg_debg << "崩溃捕获已设...";

	while( _shldrun.load( mo_acquire ) )
		try {
			_loop_body();
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
		lg_debg << "我[" << _my_thrd.native_handle() << "]已退出.";
	_running = false;

	if( _end_body != nullptr )
		_end_body();

	RevocCrashSig();
};

void Thread_t::Imp_t::killThread() {
	TRACE_POINT;

	pthread_cancel( _my_thrd.native_handle() );
	_my_thrd.detach();
	_running = false;
	_my_thrd = {};
};

Thread_t::Thread_t( str_cr name_ ) {
	TRACE_POINT;

	_imp = new Imp_t;
	_imp->_my_name = name_;
};

Thread_t::~Thread_t() {
	TRACE_POINT;

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
	TRACE_POINT;

	return _imp->_my_name;
};

bool Thread_t::isRunning() const {
	TRACE_POINT;

	return _imp->_running.load( mo_acquire );
};

bool Thread_t::shouldRun() const {
	TRACE_POINT;

	return _imp->_shldrun.load( mo_acquire );
};

bool Thread_t::start( CallBack_f loop_, CallBack_f beg_, CallBack_f end_,
					  str_cr cpus_, ns timeout_, int max_err_ ) {
	TRACE_POINT;

	if( _imp->_shldrun.load() ) {
		lg_erro << "重复启动" << _imp->_my_name;
		return true;
	}

	_imp->_on_cpus = cpus_;
	_imp->_loop_body = loop_;
	_imp->_beg_body = beg_;
	_imp->_end_body = end_;
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
	TRACE_POINT;

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
	} else if( _imp->_my_thrd.joinable() )
		_imp->_my_thrd.join();

	/* 这是错误的, 从来没有运行过的线程对象不应该 join/detach!!!
	else _imp->_my_thrd.detach(); */

	lg_debg << "已停止" << _imp->_my_name;
};

void Thread_t::stopSelf() {
	TRACE_POINT;
	_imp->_shldrun.store( false, mo_release );
};

};	// namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
