#include "sharq-common.hpp"

void ParseCmdLineOpts( int argc, const char* const* const args );

int			s_que_capa { 16 };
int			s_thrd_cnt { 1 };
int			s_intrv_ns { -1 };

Delays_t	s_all_delays;
mutex		s_upd_mutex;

void ThreadProducer( MsgQ_t* mq_, int my_id_ ) {
	MsgQ_t&		mq = * mq_;
	Msg_u		msg;
	forward_list<int64_t> delays;
	while( mq.get_flag() == RUN_CONTINUOUSLY ) {
		// msg.as_tstamp = rdtscp();
		msg.as_tstamp = steady_clock::now().time_since_epoch().count();
		if( mq.enque( msg ) ) {
			// delay += rdtscp() - msg.as_tstamp;
			delays.push_front(
				steady_clock::now().time_since_epoch().count() - msg.as_tstamp );
		}
		if( s_intrv_ns >= 0 )
			std::this_thread::sleep_for( nanoseconds( s_intrv_ns ) );
		// std::this_thread::yield();
	}

	{
		lock_guard lkgd( s_upd_mutex );
		for( const auto d : delays )
			s_all_delays.push_back( d );
		cout << "====thread" << format( my_id_, 2, 0, 0, '0' )
			 << "结束" << endl;
	}
};

int main( int argc, char** args ) {
	ParseCmdLineOpts( argc, args );
	cout << "main:SHMRQ producers. capacity:" << s_que_capa
		 << endl;
	MsgQ_t msg_que;
	msg_que.plug( SHARQ_NAME );

	forward_list<thread> producers;
	for( int j = 0; j < s_thrd_cnt; ++j )
		producers.emplace_front( ThreadProducer, &msg_que, j );

	for( auto& thrd : producers )
		thrd.join();

	Statistic_t result { s_all_delays };
	cout << "\n总共发送:" << result.cnt
		 << "\n用时平均:" << result.avg
		 << "\n用时标差:" << result.std
		 << "\n用时中位:" << result.med
		 << "\n用时最大:" << result.max
		 << "\n用时最小:" << result.min
		 << endl;

	return EXIT_SUCCESS;
};

[[noreturn]]
void ShowHelpAndExit( const string& msg = {} ) {
	if( ! msg.empty() )
		std::cerr << msg << std::endl;

	std::cerr << "目的: 测试ShAtmRQ的生产者" << R"TEXT(
用法:
	-h (--help)         : 显示用法后退出
	-i (--intrv-ns)     <间隔纳秒>(每个线程发个消息后睡多久,默认不睡)
	-p (--parallel)     <并发数>(启动多少个子线程并发执行)
	-q (--que-size)     <队列容量>

)TEXT" << std::endl;

	exit( EXIT_FAILURE );
};

void ParseCmdLineOpts( int argc, const char* const* const args ) {
	bool opt_err = false;

	for( int i = 1; i < argc; ++i ) {
		string argv = trim( args[i] );

//-------- 通用选项 -------------------------------------------------------------
		if( argv == "-h" || argv == "--help" ) {
			ShowHelpAndExit();
		} else if( argv == "-i" || argv == "--intrv-ns" ) {
			if( !( opt_err = ++i >= argc ) )
				s_intrv_ns = atoi( args[i] );
		} else if( argv == "-p" || argv == "--parallel" ) {
			if( !( opt_err = ++i >= argc ) )
				s_thrd_cnt = atoi( args[i] );
		} else if( argv == "-q" || argv == "--que-size" ) {
			if( !( opt_err = ++i >= argc ) )
				s_que_capa = atoi( args[i] );

//-------- 未知选项 -------------------------------------------------------------
		} else
			ShowHelpAndExit( '"' + argv + "\" 是无法识别的选项,无法继续!" );
		if( opt_err )
			ShowHelpAndExit( '"' + argv + "\"选项后面需要选项值,无法继续!" );
	};

	if( s_thrd_cnt <= 0 )
		ShowHelpAndExit( "子线程数不能为负或零, 无法工作!\n" );
	if( s_que_capa <= 0 )
		ShowHelpAndExit( "队列容量不能为负或零, 无法工作!\n" );
};

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; ;
