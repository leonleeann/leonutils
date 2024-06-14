#include <iomanip>
#include <iostream>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

#include "ChildProc.hpp"

namespace leon_utl {

pid_t ForkExecv( const str_t& bin_, const ChildArgs_t& args_ ) {
	/* 进到本函数时,日志系统应该是关闭的. 为了让 主、 子 进程输出互不干扰,
		本函数内不用 LeonLog. 而且, 子进程日志输出到 stderr
	 */

	pid_t child = fork();
	if( child < 0 ) {
		std::cerr << "fork失败.无法继续!" << std::endl;
		exit( EXIT_FAILURE );
	}
	if( child != 0 )
		// 这是父进程
		return child;

	// 这是子进程
	char* c_args[( args_.size() + 1 ) * 2];
	int i { 0 };

	// 第0个参数是 可执行文件名 本身
	c_args[i++] = const_cast<char*>( bin_.c_str() );

	// 形式: --opt-name opt-value
	for( auto&[k, v] : args_ )
		if( v.empty() ) {
			// it's a flag without value
			c_args[i++] = const_cast<char*>( k.c_str() );
		} else {
			c_args[i++] = const_cast<char*>( k.c_str() );
			c_args[i++] = const_cast<char*>( v.c_str() );
		}

	// 末尾补一个空指针,表示数组结束
	c_args[i] = nullptr;

	// 执行进去了就不会返回来了
	execv( bin_.c_str(), c_args );
	// 如果回来,必然是错误!
	std::cerr << "exec:" << bin_ << "出错!" << std::endl;
	exit( EXIT_FAILURE );
};

void SignlProc( pid_t pid_, int sig_ ) {
	kill( pid_, sig_ );
};

pid_t AnyExited() {
	return waitpid( -1, NULL, WNOHANG );
};

};	// namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
