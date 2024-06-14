#pragma once
#include <filesystem>

namespace leon_utl {

namespace fs = std::filesystem;
using path_t = fs::path;
using str_t = std::string;

// 创建并锁定临时目录
extern path_t	lock_dir();

extern str_t	read_file( const char* f_path );
extern bool		writ_file( const char* f_path, const str_t& body );

}; //namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
