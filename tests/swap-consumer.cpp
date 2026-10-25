#include "swap-common.hpp"

int main() {

	ShmBuffer_t buf;
	buf.make( BUF_NAME, BUF_BYTES, true, true );
	auto count = BUF_BYTES / sizeof( int64_t );

	auto prod_stat = static_cast<AtmState_t*>( buf.get() );
	auto cons_stat = static_cast<AtmState_t*>( buf.get() ) + count - 1;
	cons_stat->store( Pluged );
	auto base = static_cast<size_t*>( buf.get() );

	std::cout << "已对接Buffer,等待填充..." << std::endl;
	while( prod_stat->load() < Filled )
		sleep_for( 1ms );
	sleep_for( 3s );

	std::cout << "发现已填充,开始读取..." << std::endl;
	for( size_t j = 1; j < count - 1; ++j )
		if( base[j] != j )
			std::cerr << "重访数据不符!!!" << std::endl;
	sleep_for( 3s );

	std::cout << "读取完毕,开始释放..." << std::endl;
	auto beg_tp = steady_clock::now();
	buf.tagCold( cons_stat, 0, true );
	auto elapse = steady_clock::now() - beg_tp;
	cons_stat->store( Leaved );

	std::cout << "释放完毕,耗时:" << fmt( elapse.count(), 0, 0, 3, ' ', ',' )
			  << "ns. 等待对方释放..." << std::endl;
	while( prod_stat->load() < Leaved )
		sleep_for( 1ms );
	sleep_for( 3s );

	std::cout << "对方也已释,开始换出..." << std::endl;
	beg_tp = steady_clock::now();
	buf.pageOut( 0, 0, true );
	elapse = steady_clock::now() - beg_tp;
	cons_stat->store( Swaped );

	std::cout << "换出完毕,耗时:" << fmt( elapse.count(), 0, 0, 3, ' ', ',' )
			  << "ns. 等待对方换出..." << std::endl;
	while( prod_stat->load() < Swaped )
		sleep_for( 1ms );
	std::cout << "对方也已换出,等待退出..." << std::endl;
	sleep_for( 10s );

	std::cout << "将要重访数据..." << std::endl;
	for( size_t j = 1; j < count - 1; ++j )
		if( base[j] != j )
			std::cerr << "重访数据出错!!!" << std::endl;
	sleep_for( 10s );
	buf.delOsFile();

	return EXIT_SUCCESS;
};

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; ;
