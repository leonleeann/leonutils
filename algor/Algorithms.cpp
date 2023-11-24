#include "Algorithms.hpp"

namespace leon_utl {

int decimals( const double src_ ) {
	double tgt;
	int cnt = 0;
	for( ; cnt < 24; ++cnt ) {
		tgt = src_ * std::pow( 10, cnt );
		if( eq( std::round( tgt + 0.1 ), tgt ) )
			break;
	}

	return cnt;
};

};  // namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;

