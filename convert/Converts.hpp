#pragma once
#include <iomanip>
#include <sstream>
#include <string>

namespace leon_utl {

inline bool is_alpha( char c ) {
	return ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' );
};

inline bool is_digit( char c ) {
	return c >= '0' && c <= '9';
};

inline bool is_number( const char* pc, size_t s ) {
	char c;
	// 跳过前导空格,遇到数字就停下,遇到空格继续判断,遇到其它字符直接返回false
	for( ; true; ++pc, --s ) {
		if( s <= 0 )
			return false;
		c = *pc;
		if( c == 0 )
			return false;
		if( is_digit( c ) )
			break;
		if( c != 32 )
			return false;
	}

	// 中间判断,遇到空格就停下,遇到数字继续判断,遇到其它字符直接返回false
	for( ++pc, --s; true; ++pc, --s ) {
		if( s <= 0 )
			return true;
		c = *pc;
		if( c == 0 )
			return true;
		if( c == 32 )
			break;
		if( ! is_digit( c ) )
			return false;
	}

	// 尾随只能是空格
	for( ++pc, --s; true; ++pc, --s ) {
		if( s <= 0 )
			return true;
		c = *pc;
		if( c == 0 )
			return true;
		if( c != 32 )
			return false;
	}
};

inline bool is_number( const std::string& s ) {
	return is_number( s.c_str(), s.size() );
};

inline std::string trim_l( const std::string& src ) {
	auto pos = src.find_first_not_of( ' ' );
	return ( std::string::npos == pos ? "" : src.substr( pos ) );
};

inline std::string trim_r( const std::string& src ) {
	auto pos = src.find_last_not_of( ' ' );
	return ( std::string::npos == pos ? "" : src.substr( 0, pos + 1 ) );
};

inline std::string trim( const std::string& src ) {
	return trim_l( trim_r( src ) );
};

inline std::string pad_l( const std::string& str, size_t wide,
						  const std::string& tail = " ", char pad_ch = ' ' ) {
	std::ostringstream oss;
	oss << std::setw( wide - tail.size() ) << std::right
		<< std::setfill( pad_ch ) << str << tail;
	return oss.str();
};

inline std::string pad_r( const std::string& str, size_t wide,
						  const std::string& head = " ", char pad_ch = ' ' ) {
	std::ostringstream oss;
	oss << head << std::setw( wide - head.size() ) << std::left
		<< std::setfill( pad_ch ) << str;
	return oss.str();
};

inline std::string to_lower( const std::string& src ) {
	std::string s = src;
	for( auto& c : s )
		c = std::tolower( c );
	return s;
};

inline std::string to_upper( const std::string& src ) {
	std::string s = src;
	for( auto& c : s )
		c = std::toupper( c );
	return s;
};

template <typename T>
inline constexpr const char* parse_int( T& i, const char* s, size_t n ) {
	i = 0;
	while( s != nullptr && is_digit( *s ) && n-- > 0 )
		i = i * 10 + ( *s++ - '0' );
	return s;
};

template <typename T>
inline const char* parse_all( T& i, const char* s ) {
	i = 0;
	while( s != nullptr && is_digit( *s ) )
		i = i * 10 + ( *s++ - '0' );
	return s;
};

// 指针按十六进制输出地址值, 便于调试时对比内存位置是否相同
std::string ptr2hex( const void* );

template<typename T>
std::string format( const T	value,			//源数值
					size_t	width = 0,		//结果宽度(含填充及分隔符)
					size_t	preci = 3,		//精度(小数位数)
					size_t	group = 0,		//分组位数(隔几位分一下)
					char	pad_c = ' ',	//左边填充字符
					char	sep_c = '\'' );	//分组分隔符

}; //namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
