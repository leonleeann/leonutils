#include <chrono>
#include <iomanip>
#include <iostream>
#include <leonutils/Chrono.hpp>
#include <thread>

#include "leonutils/ShmBuffer.hpp"

using namespace leon_utl;
using namespace std::chrono;
using namespace std::chrono_literals;

constexpr size_t BUF_BYTES = 2048UL * 1024 * 1024;

int main() {

	ShmBuffer_t buf;
	buf.make( "test_shm_swapout", BUF_BYTES, true );

	int64_t* base = static_cast<int64_t*>( buf.get() );
	int64_t count = BUF_BYTES / sizeof( int64_t );

	std::cout << "已创建Buffer,开始填充..." << std::endl;
	auto beg_tp = steady_clock::now();

	for( int64_t j = 0; j < count; ++j )
		base[j] = j;

	auto elapse = steady_clock::now() - beg_tp;
	std::cout << "填充完成,耗时:" << elapse.count() << "ns." << std::endl;

	std::cout << "将要换出到交换内存..." << std::endl;
	beg_tp = steady_clock::now();

	auto swap_size = buf.swapout();

	elapse = steady_clock::now() - beg_tp;
	std::cout << "换出完成,耗时:" << elapse.count()
			  << "ns. 共:" << swap_size << "bytes.\n请观察df/top/free输出变化."
			  << std::endl;

	std::this_thread::sleep_for( 10s );

	for( int64_t j = 0; j < count; ++j )
		if( base[j] != j )
			std::cerr << "重访数据出错!!!" << std::endl;

	return EXIT_SUCCESS;
};

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; ;
