#include <atomic>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <leonutils/Converts.hpp>
#include <leonutils/Statistics.hpp>
#include <rpc/client.h>
#include <rpc/server.h>
#include <thread>
#include <vector>

//========== 类型申明 ===========================================================
using namespace leon_utl;
using namespace std::chrono;
using namespace std::chrono_literals;
using str_t = std::string;
using SysTime_t = system_clock::time_point;
using SysDura_t = system_clock::duration;
using atime_t = std::atomic<SysTime_t>;
using aint_t = std::atomic_int;

struct TimePair_t { SysTime_t send_at; SysTime_t recv_at; };
struct TimeDiff_t {
	int64_t send_at;	// 客户端发起时点(客户机时戳)
	int64_t serv_at;	// 服务器响应时点(服务器时戳)
	int64_t recv_at;	// 客户端收到时点(客户机时戳)
	int64_t latency;	// 总延迟(客户机 -> 服务器 -> 客户机)
};
using TimeDifs_t = std::vector<TimeDiff_t>;

//========== 函数前置申明 ========================================================
void	ParseArgs( int arg_cnt, const char* const* const arg_vals );

void	RunServer();
void	RunClient();

// RPC服务函数, 向对方返回本系统当前时间
int64_t	ServeTime( int64_t send_at );

// 计算并显示时差
void	Calculate();

//========== 全局变量 ===========================================================
TimeDifs_t	g_time_difs {};
str_t		g_sevr_addr { "127.0.0.1" };
int			g_listen_on { 7780 };
bool		g_as_server {};
SysDura_t	g_for_secs { 10s };
SysDura_t	g_interval { 10ms };

int main( int argc, const char* const* const args ) {
	ParseArgs( argc, args );

	if(g_as_server)
		RunServer();
	else
		RunClient();

	return EXIT_SUCCESS;
};

void RunServer() {
	rpc::server rpc_svr { g_sevr_addr, static_cast<uint16_t>( g_listen_on ) };
	rpc_svr.bind( "T", &ServeTime );

	// 开始单次 Session
	rpc_svr.bind( "B", [&]()->void {
		g_time_difs.clear();
		std::cout << "Session开始..." << std::endl;
	} );

	// 结束单次 Session
	rpc_svr.bind( "E", &Calculate );

	rpc_svr.run();
//	rpc_svr.async_run();
};

int64_t ServeTime( int64_t sent_ ) {
//	TimeDiff_t td { sent_, system_clock::now().time_since_epoch().count() };
//	g_time_difs.emplace_back( td );
	return system_clock::now().time_since_epoch().count();
};

void Calculate() {
	// 先看看总体延迟: 客户机 -> 服务器 -> 客户机
	NumbVect_t lags;
	for( auto& td : g_time_difs )
		lags.emplace_back( td.latency );

	Statistic_t result { lags };
	std::cout << "\n总体延迟:"
			  << "\n查询:" << fmt( result.cnt, 0, 0, 3, ' ', ',' ) << "次,"
			  << "\n最大:" << fmt( result.max, 0, 0, 3, ' ', ',' ) << "ns,"
			  << "\n最小:" << fmt( result.min, 0, 0, 3, ' ', ',' ) << "ns,"
			  << "\n中位:" << fmt( result.med, 0, 0, 3, ' ', ',' ) << "ns,"
			  << "\n平均:" << fmt( result.avg, 0, 0, 3, ' ', ',' ) << "ns,"
			  << "\n标差:" << fmt( result.std, 0, 0, 3, ' ', ',' ) << "ns,"
			  << std::endl;

	// 再看看扣除网络延迟后的时差: 客户机时间 - 服务器时间 - 单边传输延迟
	lags.clear();
	for( auto& td : g_time_difs )
		lags.emplace_back( td.recv_at - td.serv_at - result.med );

	result( lags );
	std::cout << "时差(客户机比服务器快的ns数):"
			  << "\n最大:" << fmt( result.max, 0, 0, 3, ' ', ',' ) << "ns,"
			  << "\n最小:" << fmt( result.min, 0, 0, 3, ' ', ',' ) << "ns,"
			  << "\n中位:" << fmt( result.med, 0, 0, 3, ' ', ',' ) << "ns,"
			  << "\n平均:" << fmt( result.avg, 0, 0, 3, ' ', ',' ) << "ns,"
			  << "\n标差:" << fmt( result.std, 0, 0, 3, ' ', ',' ) << "ns,"
			  << std::endl;
};

void RunClient() {
	std::cout << "Session开始(时差代表客户机比服务器快的ns数)..." << std::endl;

	rpc::client rpc_clt { g_sevr_addr, static_cast<uint16_t>( g_listen_on ) };
//	rpc_clt.call( "B" );

	auto send_at = system_clock::now().time_since_epoch().count();
	auto end_time = ( system_clock::now() + g_for_secs ).time_since_epoch().count();
	auto new_line = send_at + 10000000000;
	// 先热个身, 避免首次延迟纳入统计
	rpc_clt.call( "T", send_at );

	g_time_difs.clear();
	TimeDiff_t td;
	IncStat_t	trans_lag, time_diff;
	aint_t	phase0 {}, phase1 {};

	while( send_at < end_time ) {
		send_at = system_clock::now().time_since_epoch().count();
		phase0 = phase0.load() + 1;
		td.serv_at = rpc_clt.call( "T", send_at ).as<int64_t>();
		phase1 = phase1.load() + 1;
		td.recv_at = system_clock::now().time_since_epoch().count();
		td.send_at = send_at;
		td.latency = ( td.recv_at - td.send_at ) / 2;
		g_time_difs.emplace_back( td );
		trans_lag.update( td.latency );
		time_diff.update( td.recv_at - td.serv_at - td.latency );
		std::cout << "\r"
				  << "延迟:" << fmt( trans_lag.avg(), 10, 0, 3, ' ', ',' ) << "ns,  "
				  << "标差:" << fmt( trans_lag.std(), 10, 0, 3, ' ', ',' ) << "ns,  "
				  << "时差:" << fmt( time_diff.avg(), 10, 0, 3, ' ', ',' ) << "ns,  "
				  << "标差:" << fmt( time_diff.std(), 10, 0, 3, ' ', ',' ) << "ns,  "
				  << "次数:" << fmt( time_diff._cnt,  10, 0, 3, ' ', ',' ) << ", "
				  << phase0.load() << ',' << phase1.load() << std::flush;
		if( send_at > new_line ) {
//			trans_lag = {};
			time_diff = {};
			std::cout << std::endl;
			new_line = send_at + 10000000000;
		}
		std::this_thread::sleep_for( 20ms );
	};

//	rpc_clt.call( "E" );
	Calculate();
};

void Help( const str_t& hint_ ) {
	if( ! hint_.empty() )
		std::cerr << hint_ << '\n' << std::endl;

	std::cerr << R"TXT(用法:
公共选项:
	-h (--help)         : 显示用法后退出
	-p (--port)         <服务器监听端口>(默认:7780)

服务器选项:
	-l (--listen)       <服务器监听地址>(默认:127.0.0.1)

客户端选项:
	-c (--client)       <连接服务器地址>(默认:127.0.0.1)
	-f (--for-seconds)  <持续时间>(默认:10s)
	-i (--interval-ms)  <查询间隔>(默认:10ms)

)TXT" << std::endl;

	exit( EXIT_FAILURE );
};

void ParseArgs( int cnt_, const char* const* const args_ ) {
	bool opt_err = false;

	for( int i = 1; i < cnt_; ++i ) {
		str_t argv = args_[i];
		if( argv == "-h" || argv == "--help" ) {
			Help( "" );

		} else if( argv == "-p" || argv == "--port" ) {
			if( !( opt_err = ++i >= cnt_ ) )
				g_listen_on = atoi( args_[i] );

		} else if( argv == "-l" || argv == "--listen" ) {
			g_as_server = true;
			if( !( opt_err = ++i >= cnt_ ) )
				g_sevr_addr = args_[i];

		} else if( argv == "-c" || argv == "--client" ) {
			g_as_server = false;
			if( !( opt_err = ++i >= cnt_ ) )
				g_sevr_addr = args_[i];

		} else if( argv == "-f" || argv == "--for-seconds" ) {
			if( !( opt_err = ++i >= cnt_ ) )
				g_for_secs = seconds( atoi( args_[i] ) );

		} else if( argv == "-i" || argv == "--interval-ms" ) {
			if( !( opt_err = ++i >= cnt_ ) )
				g_interval = milliseconds( atoi( args_[i] ) );

		} else
			Help( '"' + argv + "\"是无法识别的选项,无法继续!" );

		if( opt_err )
			Help( '"' + argv + "\"选项后面需要选项值,无法继续!" );
	}

};

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; ;
