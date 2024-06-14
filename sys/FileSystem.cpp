#include <cstring>
#include <fstream>

#include "FileSystem.hpp"
#include "convert/Converts.hpp"

namespace fs = std::filesystem;
using str_t = std::string;
using std::ifstream;
using std::ofstream;

namespace leon_utl {

// 请求OS创建一个临时目录, 进入并锁定它
path_t lock_dir() {

	path_t tmp_path = fs::temp_directory_path() / "CTP.XXXXXX";
	char buffer[ 256 ];
	copy_str( tmp_path, buffer, sizeof( buffer ) );

	if( !mkdtemp( buffer ) ) {
		str_t tmpDir( "临时目录:\"" );
		tmpDir.append( buffer );
		tmpDir.append( "\"创建失败!" );
		throw std::runtime_error( tmpDir );
	}

	tmp_path = path_t( buffer );
	current_path( tmp_path );
	return tmp_path;
};

str_t read_file( const char* f_path ) {
	ifstream ifs { f_path, std::ios_base::in | std::ios_base::ate };
	if( ! ifs.is_open() )
		return "";

	auto size = ifs.tellg();
	if( size == 0 )
		return "";

	str_t buff( size, '\0' );
	ifs.seekg( 0 );
	if( ! ifs.good() )
		return "";
	if( ! ifs.read( buff.data(), size ) )
		return "";

	buff.resize( size );
	return buff;
};

bool writ_file( const char* f_path, const str_t& body ) {
	ofstream ofs { f_path, std::ios_base::out | std::ios_base::trunc };
	ofs.imbue( std::locale( "zh_CN.UTF-8" ) );
	ofs.clear();

	if( ! ofs.is_open() || ! ofs.good() )
		return false;

	ofs << body;
	ofs.close();
	return true;
};

}; //namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
