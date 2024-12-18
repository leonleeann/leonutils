#pragma once
#include <string>

using str_t = std::string;

namespace leon_utl {

extern str_t	new_tmp_dir();
extern void		del_tmp_dir( const str_t& );

extern str_t	read_file( const char* f_path );
extern bool		writ_file( const char* f_path, const str_t& body );

}; //namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
