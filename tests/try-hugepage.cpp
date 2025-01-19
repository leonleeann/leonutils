#include <chrono>
#include <iomanip>
#include <iostream>
#include <leonutils/Chrono.hpp>
#include <thread>

#include "leonutils/HugePage.hpp"

using namespace leon_utl;
using namespace std::chrono;
using namespace std::chrono_literals;

constexpr size_t BUF_BYTES = 2048UL * 1024 * 1024;

int main() {

	HugePage_t buf;
	buf.make( BUF_BYTES );

	int64_t* base = static_cast<int64_t*>( buf.get() );
	int64_t count = BUF_BYTES / sizeof( int64_t );

	auto beg_tp = system_clock::now();
	std::cout << "已创建Buffer,开始填0..." << std::endl;
	for( int64_t j = 0; j < count; ++j )
		base[j] = 0;

	auto elapse = system_clock::now() - beg_tp;
	std::cout << "填0完成,耗时:" << elapse.count() << "ns." << std::endl;
	std::this_thread::sleep_for( 30s );

	return EXIT_SUCCESS;
};

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; ;
