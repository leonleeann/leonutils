#include <chrono>
#include <iomanip>
#include <iostream>
#include <leonutils/Chrono.hpp>
#include <leonutils/Converts.hpp>

//========== 类型申明 ===========================================================
using namespace leon_utl;
using namespace std::chrono;
using namespace std::chrono_literals;

void ParseArgs( int cnt, const char* const* const args );

int64_t	s_ns_since_epoch;
bool	s_has_input {};

int main( int argc, const char* const* const args ) {
	ParseArgs( argc, args );

	SysTime_t stamp { ns( s_ns_since_epoch ) };
	std::cout << fmt( stamp, 9, FULL_FORMAT ) << std::endl;

	return EXIT_SUCCESS;
};

void Help( str_cr hint_ ) {
	if( ! hint_.empty() )
		std::cerr << hint_ << '\n' << std::endl;

	std::cerr << R"TXT(用法:
	show-stamp -h (--help)  : 显示用法后退出

	show-stamp <自epoch以来的ns数>
)TXT" << std::endl;

	exit( EXIT_FAILURE );
};

void ParseArgs( int cnt_, const char* const* const args_ ) {

	for( int i = 1; i < cnt_; ++i ) {
		str_t argv = args_[i];
		if( argv == "-h" || argv == "--help" )
			Help( "" );

		else if( is_number( argv ) ) {
			parse_int( s_ns_since_epoch, argv.c_str(), argv.size() );
			s_has_input = true;

		} else
			Help( '"' + argv + "\"不是一个正整数!" );
	}

	if( !s_has_input )
		Help( "必须要给一个数代表从UTC[1970-01-01 00:00:00]以来的纳秒数!" );
};

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; ;
