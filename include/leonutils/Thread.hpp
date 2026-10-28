#pragma once
#include <functional>
#include <string>

#include "leonutils/ChronoTypes.hpp"

using str_t = std::string;	using str_cp = const str_t*; using str_cr = const str_t&;

namespace leon_utl {

/* Thread_t 封装一个线程
	主要完成:
		1.有名;
		2.启停方便,超时即杀
		3.可多次反复使用(非一次性对象)
		3.日志输出启停状态
		4.捕获异常 + 处理信号,尽量不死(必须要主线程已初始化好全局信号捕获)
*/
class Thread_t {
public:
	static const ns DEFAULT_WAIT_TIME;

	Thread_t( Thread_t& ) = delete;
	Thread_t( Thread_t&& ) = delete;
	Thread_t( const Thread_t& ) = delete;
	Thread_t( const Thread_t&& ) = delete;
	Thread_t& operator=( Thread_t& ) = delete;
	Thread_t& operator=( Thread_t&& ) = delete;
	Thread_t& operator=( const Thread_t& ) = delete;
	Thread_t& operator=( const Thread_t&& ) = delete;

	// 除了创建_imp,其实啥也不干,尽量让本对象可以 static
	explicit Thread_t( str_cr name );
	~Thread_t();

	str_cr name() const;

	bool start(
		std::function<void()> to_do_body,		// 要在线程里面干什么(会反复调用)
		str_cr only_runs_on_cpus = "",			// 只在哪些cpu上运行(为空就不设)
		ns stop_timeout = DEFAULT_WAIT_TIME,	// 多久没启动算超时(会杀掉子线程)
		int max_errors = 3						// 崩溃几次就不再继续(子线程退出)
	);

	void stop( ns stop_timeout = DEFAULT_WAIT_TIME );

	// 我正在运行(已启动)
	bool isRunning() const;

	// 我应该运行(使用者给的to_do_body里面应该定期检查本函数,以获知外界是否要求本线程停止)
	bool shouldRun() const;

protected:
	// 内部实现
	struct	Imp_t;
	Imp_t*	_imp;
};

};	// namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
