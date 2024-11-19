#include <chrono>
#include <cstring>		// strlen, strncmp, strncpy, memset, memcpy, memmove, strerror
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <leonutils/ShmAtomicRQ.hpp>
#include <leonutils/ShmBuffer.hpp>
#include <sys/mman.h>	// shm_open, PROT_READ, PROT_WRITE, MAP_PRIVATE, MAP_ANON
#include <thread>

using namespace leon_utl;
using namespace std::chrono_literals;

namespace fs = std::filesystem;
using path_t = fs::path;

using MsgQ_t = ShmAtmRQ_t<int, int32_t>;

constexpr int32_t BUFFER_CAPA = 64;

void WaitMsg( MsgQ_t* mq_, int want_ ) {

	int msg {};
	while( !mq_->deque( msg ) )
		std::this_thread::sleep_for( 1ms );

	if( msg != want_ ) {
		std::cerr << "失去同步!" << std::endl;
		exit( EXIT_FAILURE );
	}
};

void CheckBuffer( void* buf_, int32_t cnt_, int val_ ) {
	int* buffer = static_cast<int*>( buf_ );
	for( int32_t j = 0; j < cnt_; ++j )
		if( buffer[j] != val_ ) {
			std::cerr << "内容不符!" << std::endl;
			exit( EXIT_FAILURE );
		};
};

int main( int argc, char** args ) {
	MsgQ_t consumer_mq;
	MsgQ_t producer_mq;
	consumer_mq.make( 16, "CONSUMER_MQ" );

	ShmBuffer_t	buf;
	buf.make( "UNLINK_SHM", BUFFER_CAPA, false );

	std::cout << "已建Buffer,等待它对接0..." << std::endl;
	WaitMsg( &consumer_mq, 0 );	//----------------------------------------------

	buf.delOsFile();
	producer_mq.plug( "PRODUCER_MQ" );
	producer_mq.enque( 0 );
	std::cout << "已删文件,等待它写全1..." << std::endl;
	WaitMsg( &consumer_mq, 1 );	//----------------------------------------------

	CheckBuffer( buf.get(), BUFFER_CAPA, 1 );
	producer_mq.enque( 1 );
	std::cout << "已读全1,等它改2..." << std::endl;
	WaitMsg( &consumer_mq, 2 );	//----------------------------------------------

	CheckBuffer( buf.get(), BUFFER_CAPA, 2 );
	producer_mq.enque( 2 );

	std::cout << "成功完成." << std::endl;
	return EXIT_SUCCESS;
};

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
