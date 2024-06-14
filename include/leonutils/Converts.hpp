#pragma once
#include <cstring>		// strlen, strncmp, strncpy, memset, memcpy, memmove, strerror
#include <iomanip>
#include <iosfwd>
#include <string>

namespace leon_utl {

using oss_t = std::ostringstream;
using str_t = std::string;

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

inline bool is_number( const str_t& s ) {
	return is_number( s.c_str(), s.size() );
};

inline str_t trim_l( const str_t& src ) {
	auto pos = src.find_first_not_of( ' ' );
	return ( str_t::npos == pos ? "" : src.substr( pos ) );
};

inline str_t trim_r( const str_t& src ) {
	auto pos = src.find_last_not_of( ' ' );
	return ( str_t::npos == pos ? "" : src.substr( 0, pos + 1 ) );
};

inline str_t trim( const str_t& src ) {
	return trim_l( trim_r( src ) );
};

inline void copy_str( const char* src_, char* dest_, ssize_t size_ = -1 ) {
	ssize_t src_len = std::strlen( src_ );
	if( size_ < 0 )
		size_ = src_len + 1;

	ssize_t to_cp = std::min( src_len, size_ - 1 );
	std::memcpy( dest_, src_, to_cp );
	dest_[to_cp] = '\0';
};

/* 因为 std::strncpy 不太满足需求,只好自己做一个. 比如:
 * CTP 的一个 TradingDay 字段是 char[9], 我们用一个 string("20240217") 本来刚好填充.
 * 但调用 strncpy, 最后一个参数如果给 9, 要得到 gcc 警告"Wstringop-truncation";
 * 如果给8, 又不能保证末尾有个 null 字符. */
inline void copy_str( const str_t& src_, char* dest_, ssize_t size_ = -1 ) {
	ssize_t src_len = src_.size();
	if( size_ < 0 )
		size_ = src_len + 1;

	ssize_t to_cp = std::min( src_len, size_ - 1 );
	std::memcpy( dest_, src_.c_str(), to_cp );
	dest_[to_cp] = '\0';
};

inline str_t pad_l( const str_t& s, size_t w,
					const str_t& tail = " ", char ch = ' ' ) {
	oss_t o;
	o << std::setw( w - tail.size() ) << std::right << std::setfill( ch )
	  << s << tail;
	return o.str();
};

inline str_t pad_r( const str_t& s, size_t w,
					const str_t& head = " ", char ch = ' ' ) {
	oss_t o;
	o << head << std::setw( w - head.size() ) << std::left << std::setfill( ch )
	  << s;
	return o.str();
};

inline str_t to_lower( const str_t& src ) {
	str_t s = src;
	for( auto& c : s )
		c = std::tolower( c );
	return s;
};

inline str_t to_upper( const str_t& src ) {
	str_t s = src;
	for( auto& c : s )
		c = std::toupper( c );
	return s;
};

// 移除字符串内的特殊字符,仅保留字母、数字,和下划线
str_t rm_special( const char* source );

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
str_t ptr2hex( const void* ptr );

template<typename T>
str_t format( const T	value,		//源数值
			  size_t	width = 0,	//结果宽度(含填充及分隔符)
			  size_t	preci = 3,	//精度(小数位数)
			  size_t	group = 0,	//分组位数(隔几位分一下)
			  char	pad_c = ' ',	//左边填充字符
			  char	sep_c = '\'' );	//分组分隔符

}; //namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
