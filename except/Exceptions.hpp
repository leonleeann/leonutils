#pragma once
#include <stdexcept>
#include <string>

namespace leon_utl {

using str_t = std::string;

// 错误地使用
class bad_usage : public std::logic_error {
public:
	// using std::logic_error::logic_error;
	explicit bad_usage( const str_t& err_msg ):
		std::logic_error( err_msg ) {};

	explicit bad_usage( const char* err_msg ):
		std::logic_error( err_msg ) {};
};

// 下标越界
class bad_index : public std::runtime_error {
public:
	// using std::runtime_error::runtime_error;

	explicit bad_index(): std::runtime_error( "下标越界" ) {};

	explicit bad_index( const str_t& err_msg ):
		std::runtime_error( err_msg ) {};

	explicit bad_index( const char* err_msg ):
		std::runtime_error( err_msg ) {};
};

}; //namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
