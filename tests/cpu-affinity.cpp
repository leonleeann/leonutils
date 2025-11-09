#include <iomanip>
#include <iostream>
#include <leonutils/CpuAffinity.hpp>
#include <thread>

using namespace leon_utl;
using namespace std::chrono;
using namespace std::chrono_literals;

// 模拟线程, 把CPU占满
int s_gabagy {};
void ThreadBody( system_clock::duration for_ ) {
	std::cout << "线程设置GetSchedCpuMask:" << GetSchedCpuMask() << std::endl;
	std::cout << "线程设置GetThreadCpuMask:" << GetThreadCpuMask() << std::endl;

	auto end_time = system_clock::now() + for_;
	while( system_clock::now() < end_time )
		++s_gabagy;
};

int main() {
	std::cout << "原始设置GetSchedCpuMask:" << GetSchedCpuMask() << std::endl;
	std::cout << "原始设置GetThreadCpuMask:" << GetThreadCpuMask() << std::endl;

	std::cout << "==== 现在试验ProcessOnlyCPU:将主线程绑到CPU2上,再看子线程亲和性 ===="
			  << std::endl;
	ProcessOnlyCPU( "2" );
	std::cout << "全局设置GetSchedCpuMask:" << GetSchedCpuMask() << std::endl;
	std::cout << "全局设置GetThreadCpuMask:" << GetThreadCpuMask() << std::endl;
	std::cout << "启动子线程..." << std::endl;

	std::thread child2 { ThreadBody, 5s };
	child2.join();
	std::cout << "子线程已退." << std::endl;

	std::cout << "==== 现在试验PthreadOnlyCPU:将主线程绑到CPU3上,再看子线程亲和性 ===="
			  << std::endl;
	PthreadOnlyCPU( "3" );
	std::cout << "全局设置GetSchedCpuMask:" << GetSchedCpuMask() << std::endl;
	std::cout << "全局设置GetThreadCpuMask:" << GetThreadCpuMask() << std::endl;
	std::cout << "启动子线程..." << std::endl;

	std::thread child3 { ThreadBody, 5s };
	child3.join();
	std::cout << "子线程已退." << std::endl;

	return EXIT_SUCCESS;
};

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; ;
