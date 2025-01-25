#pragma once
#include <map>
#include <string>
#include <sys/types.h>

template<typename K, typename V, typename C = std::less<K>,
		 typename A = std::allocator<std::pair<const K, V> > >
using map_t = std::map<K, V, C, A>;
using str_t = std::string;

namespace leon_utl {

/* 为了测试某些模块是否: 在正确的时机, 以正确的参数, fork() + execv() 了正确的执行文件,
	只好在此类 syscall 之前加一层, 以便在单元测试时能够截获对这些函数的调用.
	包括:
		1. ForkExecv 截获 fork() + execv();
		2. SignlProc 截获 kill();
		3. AnyExited 截获 waitpid();

	在产品代码中, ForkExecv 和 SignlProc 的实现, 直接调用 fork() + execv() 和 kill.
	在测试代码中, ForkExecv 和 SignlProc 的实现, 转而调用 mock 对象.
*/

// 使用map而非vector来保存参数,是为了排除参数顺序的干扰,便于测试时对照整个容器
using CliArgs_t = map_t<str_t, str_t>;

class ProcCtrl_t {
public:
	static ProcCtrl_t* CurImp();

	// 任何抽象类的析构器都必须是虚函数!
	virtual ~ProcCtrl_t() = default;

	// 只创建子进程(就是系统调用 fork)
	virtual pid_t	forkOnly() = 0;

	// 创建子进程, 并执行指定的可执行文件
	virtual pid_t	forkExec( const str_t& bin_path, const CliArgs_t& ) = 0;

	// 给某进程发信号
	virtual void	sendSign( pid_t, int ) = 0;

	// 获知某个子进程已死, 并为其收尸(waitpid(-1))
	virtual pid_t	waitExit() = 0;
};

};	// namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
