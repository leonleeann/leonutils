#include <iostream>
#include <sstream>

#include "leonutils/SetOps.hpp"

using oss_t = std::ostringstream;

namespace leon_utl {

/*
template <typename K, typename C, typename SA>
str_t to_str( const set_t<K, C, SA>& set_, char d_ ) {

	oss_t oss;
	bool first = true;

	for( const auto& k : set_ ) {
		if( !first )
			oss << d_;
		first = false;

		oss << k;
	}

	return oss.str();
};
template str_t to_str( const IntSet_t&, char );
template str_t to_str( const StrSet_t&, char );
*/

}; //namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
