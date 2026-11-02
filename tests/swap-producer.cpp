#include "swap-common.hpp"

int main() {
	StartLog( "producer.log", {}, false, true, true );
	RegistThread( "Producer" );

	ShmBuffer_t buf;
	g_buf_bytes = buf.plug( BUF_NAME, true, true );
	lg_info << "实际对接shm:" << fmt( g_buf_bytes / 1024 / 1024, 0, 0, 3, ' ', ',' ) << "MiB";

	auto count = g_buf_bytes / sizeof( int64_t );
	auto prod_stat = static_cast<AtmState_t*>( buf.get() );
	auto cons_stat = static_cast<AtmState_t*>( buf.get() ) + count - 1;
	prod_stat->store( Pluged );
	auto base = static_cast<size_t*>( buf.get() );

	lg_info << "已对接Buffer,开始填充...";
	auto beg_tp = steady_clock::now();
	for( size_t j = 1; j < count - 1; ++j ) base[j] = j;
	auto elapse = steady_clock::now() - beg_tp;
	lg_info << "填充完毕,耗时:" << fmt( elapse.count(), 0, 0, 3, ' ', ',' )
			<< "ns. 开始标冷...";

	beg_tp = steady_clock::now();
	buf.tagPage( 0, 0, true );
	elapse = steady_clock::now() - beg_tp;
	prod_stat->store( Leaved );

	lg_info << "标冷完毕,耗时:" << fmt( elapse.count(), 0, 0, 3, ' ', ',' )
			<< "ns. 等待对方标冷...";
	while( cons_stat->load() < Leaved )
		sleep_for( 1ms );
//	sleep_for( 3s );

	lg_info << "对方也已标冷,开始换出...";
	beg_tp = steady_clock::now();
	buf.pageOut( 0, 0, true );
	elapse = steady_clock::now() - beg_tp;
	prod_stat->store( Swaped );

	lg_info << "换出完毕,耗时:" << fmt( elapse.count(), 0, 0, 3, ' ', ',' )
			<< "ns. 等待对方换出...";
	while( cons_stat->load() < Swaped )
		sleep_for( 1ms );
	lg_info << "对方也已换出, 静待10s...";
	sleep_for( 10s );

	lg_info << "将要重访数据...";
	for( size_t j = 2; j < count - 1; ++j )
		if( base[j] != j )
			ExitWithLog( "重访数据不符!!!" );
//	sleep_for( 5s );

	lg_info << "重访数据完毕, 我已退出.";
	StopLog( false, false );
	return EXIT_SUCCESS;
};

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; ;
