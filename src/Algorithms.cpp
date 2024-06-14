#include "leonutils/Algorithms.hpp"

namespace leon_utl {

double safe_div( double num_, double deno_ ) {
	if( std::isnan( num_ ) || std::isnan( deno_ ) )
		return std::numeric_limits<double>::quiet_NaN();

	int  n_coe = num_ < 0. ? -1 : 1,	d_coe = deno_ < 0. ? -1 : 1;
	bool n_inf = std::isinf( num_ ),	d_inf = std::isinf( deno_ );

	if( n_inf && d_inf )
		return n_coe * d_coe;
	if( d_inf )
		return 0.0;
	if( n_inf )
		return n_coe * d_coe * std::numeric_limits<double>::infinity();

	bool n_zer = eq( num_, 0.0 ), d_zer = eq( deno_, 0.0 );
	if( n_zer && d_zer )
		return std::numeric_limits<double>::quiet_NaN();
	if( n_zer )
		return 0.0;
	if( d_zer )
		return n_coe * d_coe * std::numeric_limits<double>::infinity();

	return num_ / deno_;
};

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
