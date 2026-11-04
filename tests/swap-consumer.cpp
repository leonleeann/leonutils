#include "swap-common.hpp"

int main( int argc_, const char* const* const args_ ) {
	ParseCli( argc_, args_ );
	StartLog( "consumer.log", {}, false, true, true );
	RegistThread( "Consumer" );

	ShmBuffer_t buf;
	lg_info << "期望创建shm:" << fmt( g_buf_bytes / 1024 / 1024, 0, 0, 3, ' ', ',' ) << "MiB";
	auto alloc_bytes = buf.make( BUF_NAME, g_buf_bytes, true, true );
	lg_info << "实际创建shm:" << fmt( alloc_bytes / 1024 / 1024, 0, 0, 3, ' ', ',' ) << "MiB";

	auto count = g_buf_bytes / sizeof( int64_t );
	auto prod_stat = static_cast<AtmState_t*>( buf.get() );
	auto cons_stat = static_cast<AtmState_t*>( buf.get() ) + count - 1;
	cons_stat->store( Pluged );
	auto base = static_cast<size_t*>( buf.get() );

	lg_info << "已创建Buffer,等待填充...";
	while( prod_stat->load() < Filled )
		sleep_for( 1ms );
//	sleep_for( 3s );

	lg_info << "发现已填充,开始读取...";
	for( size_t j = 1; j < count - 1; ++j ) {
		if( base[j] != j )
			ExitWithLog( "重访数据不符!!!" );
		buf.tagCold( base + j, 1, false );
	}
//	sleep_for( 3s );

	lg_info << "读取完毕,开始标冷...";
	auto beg_tp = steady_clock::now();
	buf.tagPage( 0, 0, true );
	auto elapse = steady_clock::now() - beg_tp;
	cons_stat->store( Leaved );

	lg_info << "标冷完毕,耗时:" << fmt( elapse.count(), 0, 0, 3, ' ', ',' )
			<< "ns. 等待对方标冷...";
	while( prod_stat->load() < Leaved )
		sleep_for( 1ms );
//	sleep_for( 3s );

	lg_info << "对方也已标冷,开始换出...";
	beg_tp = steady_clock::now();
	buf.pageOut( 0, 0, true );
	elapse = steady_clock::now() - beg_tp;
	cons_stat->store( Swaped );

	lg_info << "换出完毕,耗时:" << fmt( elapse.count(), 0, 0, 3, ' ', ',' )
			<< "ns. 等待对方换出...";
	while( prod_stat->load() < Swaped )
		sleep_for( 1ms );
	lg_info << "对方也已换出, 静待10s...";
	sleep_for( 10s );

	if( g_warm_again ) {
		lg_info << "将要换入数据...";
		beg_tp = steady_clock::now();
		buf.swapIn();
		elapse = steady_clock::now() - beg_tp;
		lg_info << "换入完毕,耗时:" << fmt( elapse.count(), 0, 0, 3, ' ', ',' )
				<< "ns.";
	}

	lg_info << "将要重访数据...";
	for( size_t j = 1; j < count - 1; ++j )
		if( base[j] != j )
			ExitWithLog( "重访数据不符!!!" );
	lg_info << "重访数据完毕, 静待10s...";
	sleep_for( 10s );
	buf.delOsFile();

	lg_info << "实验完毕, 我已退出.";
	StopLog( false, false );
	return EXIT_SUCCESS;
};

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; ;
