#include "swap-common.hpp"

int main() {

	ShmBuffer_t buf;
	buf.plug( BUF_NAME, true, true );
	auto count = BUF_BYTES / sizeof( int64_t );

	auto prod_stat = static_cast<AtmState_t*>( buf.get() );
	auto cons_stat = static_cast<AtmState_t*>( buf.get() ) + count - 1;
	prod_stat->store( Pluged );
	auto base = static_cast<size_t*>( buf.get() );

	std::cout << "已创建Buffer,开始填充..." << std::endl;
	for( size_t j = 1; j < count - 1; ++j )
		base[j] = j;

	std::cout << "填充完毕,开始释放..." << std::endl;
	auto beg_tp = steady_clock::now();
	buf.tagPage( 0, 0, true );
	auto elapse = steady_clock::now() - beg_tp;
	prod_stat->store( Leaved );

	std::cout << "释放完毕,耗时:" << fmt( elapse.count(), 0, 0, 3, ' ', ',' )
			  << "ns. 等待对方释放..." << std::endl;
	while( cons_stat->load() < Leaved )
		sleep_for( 1ms );
	sleep_for( 3s );

	std::cout << "对方也已释,开始换出..." << std::endl;
	beg_tp = steady_clock::now();
	buf.pageOut( 0, 0, true );
	elapse = steady_clock::now() - beg_tp;
	prod_stat->store( Swaped );

	std::cout << "换出完毕,耗时:" << fmt( elapse.count(), 0, 0, 3, ' ', ',' )
			  << "ns. 等待对方换出..." << std::endl;
	while( cons_stat->load() < Swaped )
		sleep_for( 1ms );
	std::cout << "对方也已换出." << std::endl;
	sleep_for( 10s );

	std::cout << "将要重访数据..." << std::endl;
	for( size_t j = 2; j < count - 1; ++j )
		if( base[j] != j )
			std::cerr << "重访数据出错!!!" << std::endl;
	sleep_for( 5s );

	return EXIT_SUCCESS;
};

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; ;
