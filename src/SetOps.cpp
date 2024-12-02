#include <iostream>
#include <sstream>

#include "leonutils/SetOps.hpp"

using oss_t = std::ostringstream;

namespace leon_utl {

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

}; //namespace leon_utl

ost_t& operator<<( ost_t& os_, const StrSet_t& s_ ) {
	os_ << '{';
	for( const auto& str : s_ )
		os_ << str << ',';
	return os_ << '}';
};

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
