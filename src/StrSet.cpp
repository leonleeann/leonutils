#include <algorithm>	// set_difference, set_intersection, set_union
#include <iomanip>		// hex, dec, boolalpha, setw, endl
#include <iostream>
#include <sstream>

#include "leonutils/StrSet.hpp"

namespace leon_utl {

using oss_t = std::ostringstream;

StrSet_t split2set( const str_t& src_, char d_ ) {
	StrSet_t tokens;
	if( src_.empty() )
		return tokens;

	size_t start = 0, end = 0;
	while( ( end = src_.find( d_, start ) ) != src_.npos ) {
		tokens.insert( src_.substr( start, end - start ) );
		start = end + 1;
	}
	tokens.insert( src_.substr( start ) );
	tokens.erase( "" );
	return tokens;
};

str_t set2str( const StrSet_t& set_ ) {
	oss_t oss;
	oss << '{';
	for( const auto& s0 : set_ )
		oss << s0 << ',';
	oss << '}';
	return oss.str();
};

StrSet_t union_set( const StrSet_t& s1_, const StrSet_t& s2_ ) {
	StrSet_t result;
	std::set_union( s1_.cbegin(), s1_.cend(), s2_.cbegin(), s2_.cend(),
					std::inserter( result, result.begin() ) );
	return result;
};

StrSet_t diffr_set( const StrSet_t& s1_, const StrSet_t& s2_ ) {
	StrSet_t result;
	std::set_difference( s1_.cbegin(), s1_.cend(), s2_.cbegin(), s2_.cend(),
						 std::inserter( result, result.begin() ) );
	return result;
};

StrSet_t intersect( const StrSet_t& s1_, const StrSet_t& s2_ ) {
	StrSet_t result;
	std::set_intersection( s1_.cbegin(), s1_.cend(), s2_.cbegin(), s2_.cend(),
						   std::inserter( result, result.begin() ) );
	return result;
};

}; //namespace leon_utl

std::ostream& operator<<( std::ostream& os_, const leon_utl::StrSet_t& s_ ) {
	os_ << '{';
	for( const auto& str : s_ )
		os_ << str << ',';
	return os_ << '}';
};

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
