#include <algorithm>	// set_difference, set_intersection, set_union
#include <iomanip>		// hex, dec, boolalpha, setw, endl
#include <iostream>
#include <sstream>

#include "StrSet.hpp"

using std::ostream;
using std::ostringstream;
using std::set;
using std::set_union;
using std::string;

namespace leon_utl {

set<string> split2set( const string& src_, char d_ ) {
	set<string> tokens;
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

string set2str( const set<string>& set_ ) {
	ostringstream oss;
	oss << '{';
	for( const auto& s0 : set_ )
		oss << s0 << ',';
	oss << '}';
	return oss.str();
};

ostream& output( ostream& os_, const set<string>& s_ ) {
	os_ << '{';
	for( const auto& str : s_ )
		os_ << str << ',';
	return os_ << '}';
};

set<string> union_of( const set<string>& s1_, const set<string>& s2_ ) {
	set<string> result;
	std::set_union( s1_.cbegin(), s1_.cend(), s2_.cbegin(), s2_.cend(),
					std::inserter( result, result.begin() ) );
	return result;
};

set<string> differ_of( const set<string>& s1_, const set<string>& s2_ ) {
	set<string> result;
	std::set_difference( s1_.cbegin(), s1_.cend(), s2_.cbegin(), s2_.cend(),
						 std::inserter( result, result.begin() ) );
	return result;
};

set<string> intersect( const set<string>& s1_, const set<string>& s2_ ) {
	set<string> result;
	std::set_intersection( s1_.cbegin(), s1_.cend(), s2_.cbegin(), s2_.cend(),
						   std::inserter( result, result.begin() ) );
	return result;
};

}; //namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
