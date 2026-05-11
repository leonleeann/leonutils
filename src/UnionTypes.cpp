#include <iomanip>
#include <iostream>

#include "leonutils/UnionTypes.hpp"

namespace std {

ost_t& operator<<( ost_t& os_, leon_utl::U16_u u_ ) { return os_ << u_.view(); };

ost_t& operator<<( ost_t& os_, leon_utl::U32_u u_ ) { return os_ << u_.view(); };

ost_t& operator<<( ost_t& os_, leon_utl::U64_u u_ ) { return os_ << u_.view(); };

};	// namespace std

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
