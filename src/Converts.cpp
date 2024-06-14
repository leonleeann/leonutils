#include <cmath>		// abs, ceil, floor, isnan, log, log10, log2, pow, round, sqrt
#include <cstring>		// strlen, strncmp, strncpy, memset, memcpy, memmove, strerror
#include <iomanip>		// hex, dec, boolalpha, setw, endl
#include <iostream>
#include <numeric>
#include <sstream>
#include <type_traits>	// is_floating_point_v, is_signed_v, is_unsigned_v

#include "leonutils/Algorithms.hpp"
#include "leonutils/Converts.hpp"

using std::ostream;

namespace leon_utl {

str_t rm_special( const char* src_ ) {
	oss_t oss;
	char c;
	while( ( c = *src_ ) ) {
		if( is_alpha( c ) || is_digit( c ) )
			oss << c;
		++src_;
	}
	return oss.str();
};

str_t ptr2hex( const void* ptr_ ) {
	oss_t oss;
	oss << std::hex << ptr_;
	return oss.str();
};

struct CustomGrouper_t : std::numpunct<char> {
	CustomGrouper_t( char grp_w_, char grp_c_  = '\'', size_t persist_ = 1 ):
		// 参数 p_uiRef 传递给基类. 为0表示新建的对象应该被使用者自动释放.
		// 但我们这里需要一个持续存在的对象, 所以要给1(不给也算0)
		std::numpunct<char>( persist_ ),
		group_w( grp_w_ ),
		group_c( grp_c_ ) {};

	/* GrouingDigits_t( char grp_c_, size_t persist_ = 1 )
		: std::numpunct<char>( persist_ ), group_c( grp_c_ ) {};*/

	char do_thousands_sep() const override {
		return group_c;
	};
	str_t do_grouping() const override {
		return str_t( 1, group_w );
	};

	char group_w = 0;  // 隔几位分一下？
	char group_c = '\'';   // 缺省的分隔符
};

thread_local CustomGrouper_t tl_grouper( '\0', '\'', 1 );

template<typename T>
str_t format( const T v_, size_t w_, size_t p_, size_t g_, char f_, char s_ ) {

	// 浮点数的特殊值
	if( std::is_floating_point_v<T> ) {
		if( std::isnan( v_ ) )
			return ( str_t( w_, f_ ) + "nan" ).substr( 3, w_ );
		if( std::isinf( v_ ) )
			return ( str_t( w_, f_ ) + "inf" ).substr( 3, w_ );
		if( strict_max( v_ ) )
			return ( str_t( w_, f_ ) + "maximu" ).substr( 3, w_ );
		if( strict_low( v_ ) )
			return ( str_t( w_, f_ ) + "lowest" ).substr( 3, w_ );
	}

	oss_t oss;
	tl_grouper.group_w = static_cast<char>( g_ );
	tl_grouper.group_c = s_;
	std::locale locGrpDigits( oss.getloc(), &tl_grouper );
	oss.imbue( locGrpDigits );

	oss << std::fixed
		<< std::setw( w_ )
		<< std::setprecision( p_ )
		<< std::setfill( f_ );

	// 单字节的整数总是会被当作 char 输出,只好先转为整数再输出
	if( sizeof( T ) > 1 )
		oss << v_;
	else
		oss << static_cast<int>( v_ );

	str_t result( oss.str() );
	if( w_ > 0 && result.length() > w_ )
		return str_t( w_, '*' );
	else
		return result;
};

// 显式地实例化一下,以免链接时找不到
template str_t format<int8_t>( const int8_t, size_t, size_t, size_t, char, char );
template str_t format<uint8_t>( const uint8_t, size_t, size_t, size_t, char, char );
template str_t format<int16_t>( const int16_t, size_t, size_t, size_t, char, char );
template str_t format<uint16_t>( const uint16_t, size_t, size_t, size_t, char, char );
template str_t format<int32_t>( const int32_t, size_t, size_t, size_t, char, char );
template str_t format<uint32_t>( const uint32_t, size_t, size_t, size_t, char, char );
template str_t format<int64_t>( const int64_t, size_t, size_t, size_t, char, char );
template str_t format<uint64_t>( const uint64_t, size_t, size_t, size_t, char, char );
template str_t format<float>( const float, size_t, size_t, size_t, char, char );
template str_t format<double>( const double, size_t, size_t, size_t, char, char );

}; //namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
