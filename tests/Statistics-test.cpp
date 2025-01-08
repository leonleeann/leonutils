#include <cmath>        // abs, ceil, floor, isnan, log, log10, pow, round, sqrt
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <limits>

#include "leonutils/Algorithms.hpp"
#include "leonutils/Statistics.hpp"

using namespace leon_utl;
using std::numeric_limits;

// 试验另一种方法计算标准差(一轮迭代)
double StdDev( const NumbVect_t& nums_ ) {
	if( nums_.size() <= 1 )
		return 0;

	double all_sum {}, all_sqr {}, all_2ab {};
	for( auto& n : nums_ ) {
		all_sum += n;
		all_sqr += n * n;
		all_2ab -= n * 2;
	};
	double cnt = nums_.size();
	double avg = all_sum / cnt;
	return sqrt( ( all_sqr + all_2ab * avg + avg * avg * cnt ) / ( cnt - 1 ) );
};

NumbVect_t RANDS_1D {
	0.581640, 0.673824, 0.871751, 0.231745, 0.237298, 0.514532, 0.145223, 0.704888,
	0.984218, 0.685163, 0.739075, 0.210370, 0.558321, 0.867142, 0.184602, 0.363319,
	0.798100, 0.893294, 0.840979, 0.575848, 0.606993, 0.918926, 0.937460, 0.626330,
	0.066237, 0.749055, 0.982635, 0.550714, 0.003984, 0.817334, 0.699981, 0.891269,
	0.090208, 0.368710, 0.254091, 0.870689, 0.183160, 0.812544, 0.685063, 0.033694,
	0.095671, 0.836642, 0.600489, 0.468980, 0.971775, 0.988077, 0.399133, 0.969386,
	0.965530, 0.782546, 0.597121, 0.454789, 0.564527, 0.576980, 0.121738, 0.266877,
	0.886378, 0.997907, 0.859766, 0.295953, 0.912448, 0.438122, 0.139964, 0.673467,
	0.635924, 0.153134, 0.476764, 0.860804, 0.435626, 0.184662, 0.032406, 0.301921,
	0.888390, 0.412042, 0.531472, 0.316178, 0.524041, 0.347831, 0.232141, 0.330426,
	0.218964, 0.102006, 0.216041, 0.866977, 0.720757, 0.496018, 0.660020, 0.633765,
	0.863836, 0.488210, 0.407598, 0.538189, 0.094233, 0.129599, 0.840921, 0.156126,
	0.984833, 0.432180, 0.298585, 0.355312,
};

NumPairs_t RANDS_2D {
	{ 0.581640, 0.002258 }, { 0.673824, 0.758902 }, { 0.871751, 0.386773 },
	{ 0.231745, 0.920563 }, { 0.237298, 0.161142 }, { 0.514532, 0.453844 },
	{ 0.145223, 0.386036 }, { 0.704888, 0.589367 }, { 0.984218, 0.254208 },
	{ 0.685163, 0.143582 }, { 0.739075, 0.839918 }, { 0.210370, 0.855650 },
	{ 0.558321, 0.053232 }, { 0.867142, 0.536521 }, { 0.184602, 0.263053 },
	{ 0.363319, 0.735341 }, { 0.798100, 0.060626 }, { 0.893294, 0.880077 },
	{ 0.840979, 0.136112 }, { 0.575848, 0.414212 }, { 0.606993, 0.563758 },
	{ 0.918926, 0.127695 }, { 0.937460, 0.720673 }, { 0.626330, 0.818682 },
	{ 0.066237, 0.351066 }, { 0.749055, 0.101301 }, { 0.982635, 0.433876 },
	{ 0.550714, 0.429262 }, { 0.003984, 0.582179 }, { 0.817334, 0.085511 },
	{ 0.699981, 0.061031 }, { 0.891269, 0.963535 }, { 0.090208, 0.920273 },
	{ 0.368710, 0.053364 }, { 0.254091, 0.386450 }, { 0.870689, 0.859494 },
	{ 0.183160, 0.986447 }, { 0.812544, 0.817912 }, { 0.685063, 0.262175 },
	{ 0.033694, 0.566869 }, { 0.095671, 0.306504 }, { 0.836642, 0.594606 },
	{ 0.600489, 0.008312 }, { 0.468980, 0.129646 }, { 0.971775, 0.367640 },
	{ 0.988077, 0.194105 }, { 0.399133, 0.351789 }, { 0.969386, 0.110687 },
	{ 0.965530, 0.406794 }, { 0.782546, 0.651657 }, { 0.597121, 0.069886 },
	{ 0.454789, 0.157036 }, { 0.564527, 0.051311 }, { 0.576980, 0.563717 },
	{ 0.121738, 0.543420 }, { 0.266877, 0.460084 }, { 0.886378, 0.262632 },
	{ 0.997907, 0.145072 }, { 0.859766, 0.783697 }, { 0.295953, 0.708654 },
	{ 0.912448, 0.131126 }, { 0.438122, 0.507499 }, { 0.139964, 0.952736 },
	{ 0.673467, 0.923857 }, { 0.635924, 0.843032 }, { 0.153134, 0.271495 },
	{ 0.476764, 0.104157 }, { 0.860804, 0.006627 }, { 0.435626, 0.050007 },
	{ 0.184662, 0.189742 }, { 0.032406, 0.726946 }, { 0.301921, 0.707460 },
	{ 0.888390, 0.460888 }, { 0.412042, 0.357269 }, { 0.531472, 0.728083 },
	{ 0.316178, 0.167759 }, { 0.524041, 0.286486 }, { 0.347831, 0.973464 },
	{ 0.232141, 0.547337 }, { 0.330426, 0.604894 }, { 0.218964, 0.417754 },
	{ 0.102006, 0.874336 }, { 0.216041, 0.321410 }, { 0.866977, 0.706745 },
	{ 0.720757, 0.238949 }, { 0.496018, 0.377176 }, { 0.660020, 0.400809 },
	{ 0.633765, 0.903156 }, { 0.863836, 0.559362 }, { 0.488210, 0.956231 },
	{ 0.407598, 0.494197 }, { 0.538189, 0.318431 }, { 0.094233, 0.212009 },
	{ 0.129599, 0.916754 }, { 0.840921, 0.065930 }, { 0.156126, 0.977482 },
	{ 0.984833, 0.021741 }, { 0.432180, 0.355541 }, { 0.298585, 0.296033 },
	{ 0.355312, 0.706861 },
};

TEST( TestStatistics, statNormally ) {
	NumbVect_t	samples = { 3, 5.5, 9, 1.25, 9999.0, 3.14159, -1.0, 0 };
	Statistic_t	result { samples };

	ASSERT_TRUE( eq( result.sum, 10019.89159 ) );
	ASSERT_TRUE( eq( result.avg, 1252.48644875 ) );
	ASSERT_TRUE( eq( result.med, 3.070795 ) );
	ASSERT_TRUE( eq( result.max, 9999.0 ) );
	ASSERT_TRUE( eq( result.min, -1.0 ) );
	ASSERT_TRUE( eq( result.std, 3534.12658936645 ) );
	ASSERT_DOUBLE_EQ( result.std, StdDev( samples ) );
};

TEST( TestStatistics, statOddsNumber ) {
	NumbVect_t	samples = { 3, 5.5, 9, 1.25, 9999.0, 3.14159, -1.0 };
	Statistic_t	result {};

	ASSERT_TRUE( result( samples ) );
	ASSERT_TRUE( eq( result.sum, 10019.89159 ) );
	ASSERT_TRUE( eq( result.avg, 1431.41308428571 ) );
	ASSERT_TRUE( eq( result.med, 3.14159 ) );
	ASSERT_TRUE( eq( result.max, 9999 ) );
	ASSERT_TRUE( eq( result.min, -1 ) );
	ASSERT_TRUE( eq( result.std, 3777.95204127486 ) );
	ASSERT_DOUBLE_EQ( result.std, StdDev( samples ) );

	samples.clear();
	ASSERT_FALSE( result( samples ) );

	samples.push_back( 0.0 );
	ASSERT_TRUE( result( samples ) );
	ASSERT_TRUE( eq( result.sum, 0.0 ) );
	ASSERT_TRUE( eq( result.avg, 0.0 ) );
	ASSERT_TRUE( eq( result.med, 0.0 ) );
	ASSERT_TRUE( eq( result.max, 0.0 ) );
	ASSERT_TRUE( eq( result.min, 0.0 ) );
	ASSERT_TRUE( eq( result.std, 0.0 ) );
	ASSERT_DOUBLE_EQ( result.std, StdDev( samples ) );

	samples[0] = 123.456;
	ASSERT_TRUE( result( samples ) );

	ASSERT_TRUE( eq( result.sum, 123.456 ) );
	ASSERT_TRUE( eq( result.avg, 123.456 ) );
	ASSERT_TRUE( eq( result.med, 123.456 ) );
	ASSERT_TRUE( eq( result.max, 123.456 ) );
	ASSERT_TRUE( eq( result.min, 123.456 ) );
	ASSERT_TRUE( eq( result.std, 0.0 ) );
	ASSERT_DOUBLE_EQ( result.std, StdDev( samples ) );
};

TEST( TestStatistics, CoeOfDeterm ) {
	// 序列必须倒序
	NumbVect_t samples = { 4, 3, 2, 1 };
	ASSERT_TRUE( eq( r_square( samples, 2.5 ), 1.0 ) );

	samples = { 25, 16, 9, 4, 1, };
	double coe_deter = r_square( samples, 11 );
	ASSERT_TRUE( eq( coe_deter, 0.72192513368984 ) );
};

TEST( TestStatistics, PearsonCorrelation ) {
	constexpr double result = -0.21328709971621806;
	ASSERT_DOUBLE_EQ( pearson( RANDS_2D ), result );

	NumPairs_t empty_nps {};
	ASSERT_TRUE( eq( pearson( empty_nps ), std::numeric_limits<double>::quiet_NaN() ) );

	NumPairs_t only_1_np { { 123, 456 } };
	ASSERT_DOUBLE_EQ( pearson( only_1_np ), 1. );
};

TEST( TestStatistics, SpearmanCorrelation ) {
	constexpr double result = -0.21936993699369936;
	ASSERT_DOUBLE_EQ( spearman( RANDS_2D ), result );

	NumPairs_t empty_nps {};
	ASSERT_TRUE( eq( spearman( empty_nps ), std::numeric_limits<double>::quiet_NaN() ) );

	NumPairs_t only_1_np { { 123, 456 } };
	ASSERT_DOUBLE_EQ( spearman( only_1_np ), 1. );
};

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
