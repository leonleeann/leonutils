#pragma once
#include <stdexcept>
#include <string>

namespace leon_utl {

using str_t = std::string;

// 错误地使用
class bad_usage: public std::logic_error {
public:
	explicit bad_usage( const str_t& err_ ):
		std::logic_error( err_ ) {};

	explicit bad_usage( const char* err_ ):
		std::logic_error( err_ ) {};
};

// 下标越界
class bad_index: public std::out_of_range {
public:
	explicit bad_index(): std::out_of_range( "下标越界" ) {};

	explicit bad_index( const str_t& err_ ): std::out_of_range( err_ ) {};

	explicit bad_index( const char* err_ ): std::out_of_range( err_ ) {};
};

class parse_err: public std::runtime_error {
public:
	explicit parse_err(): std::runtime_error( "解析失败" ) {};

	explicit parse_err( const str_t& err_ ): std::runtime_error( err_ ) {};

	explicit parse_err( const char* err_ ): std::runtime_error( err_ ) {};
};

}; //namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
