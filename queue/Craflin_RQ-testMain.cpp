#include <LeonLog>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <random>
#include <thread>

#include "CraflinRQ.tpp"
#include "misc/Converts.hpp"

using namespace leon_ext;
using namespace leon_log;
using namespace std;

struct Msg_t {
	int msg_id;
	int messag;
};
using Buffer_t = unique_ptr<Msg_t[]>;
using MsgQue_t = CraflinRQ_t<Msg_t>;
using MsgMap_t = map<int, int>;

void ShowUsageAndExit();
void parseCmdLineOpts( int argc, const char* const* const args );

LogLevel_e g_logl = LogLevel_e::Debug;
static size_t g_elements = 1024;
static size_t g_que_size = 64;
int g_prodcr_cnt = 1;
int g_consmr_cnt = 1;
atomic_bool more_data { true };

void producer_f( MsgQue_t* pq, MsgMap_t* sent_msgs,
				 size_t fr, size_t to, int myid ) {
	registThreadName( "生产者" + to_string( myid ) );
	std::random_device   rd( "/dev/urandom" );   // 初始随机数生成器
	std::mt19937         rand_gen( rd() );       // 随机数生成器
	std::uniform_int_distribution<int>  dist;    // 均匀分布
	lg_debg << "开始发送:[" << fr << ", :" << to << "].";
	Msg_t msg;
	for( size_t sent = fr; sent < to; ++sent ) {
		msg.msg_id = sent;
		msg.messag = dist( rand_gen );
		while( !pq->enque( msg ) )
			this_thread::yield();
		( *sent_msgs )[msg.msg_id] = msg.messag;
	}
	lg_debg << "生产数:" << sent_msgs->size();
};

void consumer_f( MsgQue_t* pq, Msg_t* pMsg, size_t* pCnt, int myid ) {
	registThreadName( "消费者" + to_string( myid ) );

	*pCnt = 0;
	do {
		if( pq->deque( *pMsg ) ) {
			++pMsg;
			++( *pCnt );
		} else {
			this_thread::yield();
		}
	} while( more_data.load() );

	while( pq->deque( *pMsg ) ) {
		++pMsg;
		++( *pCnt );
		lg_debg << "确实发生清盘!";
	}

	lg_debg << "消费数:" << *pCnt;
};

int main( int argc, const char* const* const args ) {
	parseCmdLineOpts( argc, args );
	startLogging( string( args[0] ) + ".log", g_logl, 9, 1048576 );

	MsgQue_t que( g_que_size );
	auto t = que.capa();
	lg_debg << "期望队长:" << g_que_size << ",实际队长:" << t
// 			<< ",mask:" << hex << que.mask()
			<< endl;
	g_que_size = t;

	t = pow( 2, ceil( log2( g_elements ) ) );
	t = max<size_t>( 16, min<size_t>( 0x80000000 / sizeof( Msg_t ), t ) );
	std::cout << "期望元素数:" << g_elements << ",实际元素数:" << t << endl;
	g_elements = t;

	// destin data
	Buffer_t up_dst[g_consmr_cnt];
	thread consumers[g_consmr_cnt];
	size_t deques[g_consmr_cnt];
	for( int j = 0; j < g_consmr_cnt; ++j ) {
		up_dst[j] = make_unique<Msg_t[]>( g_elements * g_prodcr_cnt );
		consumers[j] = thread( consumer_f, &que, up_dst[j].get(), deques + j, j );
	}
	// start producers
	thread   producers[g_prodcr_cnt];
	MsgMap_t src_maps[g_prodcr_cnt];
	for( int j = 0; j < g_prodcr_cnt; ++j )
		producers[j] = thread( producer_f, &que, src_maps + j,
							   j * g_elements, ( j + 1 ) * g_elements, j );

	for( int j = 0; j < g_prodcr_cnt; ++j )
		producers[j].join();
	more_data.load();
	MsgMap_t sent_map;
	for( int j = 0; j < g_prodcr_cnt; ++j )
		sent_map.merge( src_maps[j] );

	more_data.store( false );
	for( int j = 0; j < g_consmr_cnt; ++j )
		consumers[j].join();
	lg_debg << "全部完成(que.size())" << que.size();

	more_data.load();
	size_t total_sent = sent_map.size();
	MsgMap_t recv_map;
	size_t total_recv = 0;
	for( int j = 0; j < g_consmr_cnt; ++j ) {
		for( size_t u = 0; u < deques[j]; ++u ) {
			Msg_t& msg = up_dst[j][u];
			recv_map[msg.msg_id] = msg.messag;
		}
		total_recv += deques[j];
	}

	if( recv_map == sent_map && total_recv == total_sent )
		lg_debg << "核对成功. 发送总数:" << total_sent << ", 接收总数:" << total_recv;
	else
		lg_debg << "核对失败! 发送总数:" << total_sent << ", 接收总数:" << total_recv;
	stopLogging();
};

void ShowUsageAndExit() {
	cerr << R"TEXT(
目的: 测试Craflin_RQ在并发场景下的正确性
用法:
	-H (--help)			: 显示用法后退出
	-L (--log-level)	: 日志级别

	-P (--producers)	: 生产者数量
	-C (--consumers)	: 消费者数量
	-T (--all-tries)	: 入队次数
	-S (--que-size)		: 队列容量
)TEXT" << endl;
	exit( EXIT_FAILURE );
};

void parseCmdLineOpts( int argc, const char* const* const args ) {
	bool opt_err = false;
	for( int i = 1; i < argc; ++i ) {
		string val = trim( args[i] );
//-------- 通用的选项 --------
		if( val == "-H" || val == "--help" ) {
			showUsageAndExit();
		} else if( val == "-L" || val == "--log-level" ) {
			if( !( opt_err = ++i >= argc ) )
				g_logl = static_cast<LogLevel_e>( strtoul( args[i], nullptr, 10 ) );

//-------- 本应用选项 --------
		} else if( val == "-P" || val == "--producers" ) {
			if( !( opt_err = ++i >= argc ) )
				g_prodcr_cnt = strtoul( args[i], nullptr, 10 );
		} else if( val == "-C" || val == "--consumers" ) {
			if( !( opt_err = ++i >= argc ) )
				g_consmr_cnt = strtoul( args[i], nullptr, 10 );
		} else if( val == "-T" || val == "--all-tries" ) {
			if( !( opt_err = ++i >= argc ) )
				g_elements = strtoul( args[i], nullptr, 10 );
		} else if( val == "-S" || val == "--que-size" ) {
			if( !( opt_err = ++i >= argc ) )
				g_que_size = strtoul( args[i], nullptr, 10 );

//-------- 未知的选项 --------
		} else {
			cerr << '"' << val << "\" 是无法识别的选项,无法继续!" << endl;
			showUsageAndExit();
		}

		if( opt_err )
			showUsageAndExit();
	};
};
// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
