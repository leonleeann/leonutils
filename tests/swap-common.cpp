#include "swap-common.hpp"

size_t g_buf_bytes = 1024 * 1024 * 4096UL;

void ParseCli( int argc_, const char* const* const args_ ) {
	int  cli_mbs = -1;
	bool opt_err = false;

	for( int i = 1; i < argc_; ++i ) {
		str_t argv = args_[i];

		if( argv == "-h" || argv == "--help" ) {
			ShowHelp();

		} else if( argv == "-s" || argv == "--size-in-MB" ) {
			if( !( opt_err = ++i >= argc_ ) )
				cli_mbs = atoi( args_[i] );

		} else
			ShowHelp( '"' + argv + "\"是无法识别的选项,无法继续!" );

		if( opt_err )
			ShowHelp( '"' + argv + "\"选项后面需要选项值,无法继续!" );
	};

	if( cli_mbs > 0 ) {
//		lg_fatl << "cli_mbs:" << cli_mbs;
		g_buf_bytes = 1024 * 1024 * static_cast<size_t>( cli_mbs );
//		lg_fatl << "g_buf_bytes:" << g_buf_bytes;
	}
};

void ShowHelp( str_cr hint_ ) {
	if( !hint_.empty() )
		std::cerr << hint_ << '\n' << std::endl;

	std::cerr << R"TEXT(用法:
	-h (--help)         : 显示用法后退出
	-s (--size-in-MB)   <分配尺寸>(以MiB为单位,默认4096,也即4G)
)TEXT" << std::endl;

	exit( EXIT_FAILURE );
};

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
