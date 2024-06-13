#include "sharq-common.hpp"

void ParseCmdLineOpts( int argc, const char* const* const args );

RunMode_e	s_run_mode { ForDuration };
int			s_for_secs { 10 };
int			s_test_cnt { 1000 };
int			s_que_capa { 16 };
int			s_thrd_cnt { 1 };

Delays_t	s_all_delays;
mutex		s_upd_mutex;

void ThreadConsumer( MsgQ_t* mq_, int my_id_ ) {
	MsgQ_t&		mq = * mq_;
	Msg_u		msg;
	forward_list<int64_t> delays;
	while( true ) {
		// recv_bef = rdtscp();
		// msg.as_tstamp = steady_clock::now().time_since_epoch().count();
		if( mq.deque( msg ) ) {
			// delay += rdtscp() - recv_bef;
			delays.push_front(
				steady_clock::now().time_since_epoch().count() - msg.as_tstamp );
		} else if( mq.empty() && mq.get_flag() == STOP_IMMEDIATELY )
			break;
	}

	{
		lock_guard lkgd( s_upd_mutex );
		for( const auto d : delays )
			s_all_delays.push_back( d );
		cout << "====thread" << formatNumber( my_id_, 2, 0, 0, '0' )
			 << "结束" << endl;
	}
};

int main( int argc, char** args ) {
	ParseCmdLineOpts( argc, args );
	cout << "main:SHMRQ consumers." << endl;

	MsgQ_t msg_que { s_que_capa, SHARQ_NAME, };
	msg_que.set_flag( RUN_CONTINUOUSLY );

	forward_list<thread> consumers;
	for( int j = 0; j < s_thrd_cnt; ++j )
		consumers.emplace_front( ThreadConsumer, &msg_que, j );

	std::this_thread::sleep_for( seconds( s_for_secs ) );
	msg_que.set_flag( STOP_IMMEDIATELY );
	for( auto& thrd : consumers )
		thrd.join();

	StatisticResult_t result;
	StatisticAll(s_all_delays, result);
	cout << "\n总共接收:" << result.cnt
		 << "\n延迟平均:" << result.avg
		 << "\n延迟标差:" << result.std
		 << "\n延迟中位:" << result.med
		 << "\n延迟最大:" << result.max
		 << "\n延迟最小:" << result.min
		 << endl;

	return EXIT_SUCCESS;
};

[[noreturn]]
void ShowHelpAndExit( const string& msg = {} ) {
	if( ! msg.empty() )
		std::cerr << msg << std::endl;

	std::cerr << "目的: 测试ShAtmRQ的消费者" << R"TEXT(
用法:
	-h (--help)         : 显示用法后退出
	-f (--for-secs)     <持续秒数>(持续测试多少秒,与-c互斥)
	-c (--test-cnt)     <测试次数>(每个生产者发多少条消息就停止,与-f互斥)
	-p (--parallel)     <并发数>(启动多少个子线程并发执行)
	-q (--que-size)     <队列容量>

)TEXT" << std::endl;

	exit( EXIT_FAILURE );
};

void ParseCmdLineOpts( int argc, const char* const* const args ) {
	int cli_for_secs { -1 }, cli_test_cnt { -1 };
	bool opt_err = false;

	for( int i = 1; i < argc; ++i ) {
		string argv = trim( args[i] );

//-------- 通用选项 -------------------------------------------------------------
		if( argv == "-h" || argv == "--help" ) {
			ShowHelpAndExit();
		} else if( argv == "-f" || argv == "--for-secs" ) {
			if( !( opt_err = ++i >= argc ) )
				cli_for_secs = atoi( args[i] );
		} else if( argv == "-c" || argv == "--test-cnt" ) {
			if( !( opt_err = ++i >= argc ) )
				cli_test_cnt = atoi( args[i] );
		} else if( argv == "-p" || argv == "--parallel" ) {
			if( !( opt_err = ++i >= argc ) )
				s_thrd_cnt = atoi( args[i] );
		} else if( argv == "-q" || argv == "--que-size" ) {
			if( !( opt_err = ++i >= argc ) )
				s_que_capa = atoi( args[i] );

//-------- 未知选项 -------------------------------------------------------------
		} else
			ShowHelpAndExit( '"' + argv + "\"是无法识别的选项,无法继续!" );
		if( opt_err )
			ShowHelpAndExit( '"' + argv + "\"选项后面需要选项值,无法继续!" );
	};

	if( s_thrd_cnt <= 0 )
		ShowHelpAndExit( "子线程数不能为负或零, 无法工作!\n" );
	if( s_que_capa <= 0 )
		ShowHelpAndExit( "队列容量不能为负或零, 无法工作!\n" );
	if( cli_for_secs > 0 && cli_test_cnt > 0 )
		ShowHelpAndExit( "-f和-c不能同时指定, 无法工作!\n" );

	if( cli_for_secs > 0 ) {
		s_run_mode = ForDuration;
		s_for_secs = cli_for_secs;
	}
	if( cli_test_cnt > 0 ) {
		s_run_mode = ForCounting;
		s_test_cnt = cli_test_cnt;
		ExitWithLog( "按次数测试暂未实现!" );
	}
};

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
