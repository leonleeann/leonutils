#include <algorithm> // max, min, sort, swap
#include <cmath>     // abs, floor, log, log10, pow, round, sqrt
#include <numeric>   // accumulate

#include "Algorithms.hpp"
#include "Statistics.hpp"

using std::vector;

namespace leon_utl {

double average( const vector<double>& samps_ ) {
	if( samps_.empty() )
		return 0.0;

	double total = 0.0;
	for( const auto v : samps_ )
		total += v;

	return total / samps_.size();
};

double r_square( const vector<double>& samps_, double avg_ ) {
	auto cnt = samps_.size();
	if( cnt <= 1 )
		return 1.0;

	double idea_v = samps_.front();
	double step_v = idea_v / cnt;
	double dif_sum = 0.0;  // 累计 ∑(y - ŷ)² :  ∑(实测值 - 理想值)²
	double std_sum = 0.0;  // 累计 ∑(y - ȳ)² :  ∑(实测值 - 平均值)²

	for( auto v : samps_ ) {
		double diff = v - idea_v;
		dif_sum += diff * diff;
		idea_v -= step_v;

		diff = v - avg_;
		std_sum += diff * diff;
	}

	// R² = 1 - ∑(y - ŷ)² / ∑(y - ȳ)²
	return gt( std_sum, 0.0 ) ? 1 - dif_sum / std_sum : 1.0;
};

Statistic_t::Statistic_t( vector<double>& samps_ ) {
	if( operator()( samps_ ) )
		return;
	sum = max = min = med = avg = std = cnt = 0;
};

bool Statistic_t::operator()( vector<double>& samps_ ) {
	// 空集的结果是"未定义",不用初始化变量
	if( samps_.empty() )
		return false;

	cnt = static_cast<int64_t>( samps_.size() );
	// 只有一个样本
	if( cnt == 1 ) {
		sum = max = min = med = avg = samps_[0]; std = 0.0;
		return true;
	}

	// 因为样本数超过1,所以 iLast 至少等于 1
	int64_t iLast = cnt - 1;

	sort( samps_.begin(), samps_.end() );
	sum = accumulate( samps_.begin(), samps_.end(), 0.0 );
	max = samps_[iLast];
	min = samps_[0];

	int64_t iMed = iLast / 2;
	med = ( iLast % 2 == 0 ) ? samps_[iMed]
		  : ( samps_[iMed] + samps_[iMed + 1] ) / 2.0;

	avg = sum / cnt;
	std = accumulate( samps_.begin(), samps_.end(), 0.0,
	[this]( const double & a, const double & b ) {
		double diff = b - avg;
		return a + diff * diff;
	} );

	// 通过有限样本估算全体标准差(而非样本内标准差),应让除数少1,所以是iLast而不是size()
	std = sqrt( std / iLast );
	return true;
};

/*
template<typename T>
double _PearsnCorr( vector<T>& nps_ ) {
	// 空集的结果是"NaN"
	if( nps_.empty() )
		return std::numeric_limits<double>::quiet_NaN();
	// 只有一个样本
	if( nps_.size() == 1 )
		return 1.;

	StatisticResult_t x_st {}, y_st {};
	x_st.cnt = y_st.cnt = static_cast<int64_t>( nps_.size() );

	// 总和
	for( const auto& p : nps_ )
		x_st.sum += p.x, y_st.sum += p.y;

	// 平均值
	x_st.avg = x_st.sum / x_st.cnt, y_st.avg = y_st.sum / y_st.cnt;

	// 标准差、协方差
	double cov = 0.;
	for( const auto& p : nps_ ) {
		double x_dif = p.x - x_st.avg, y_dif = p.y - y_st.avg;
		x_st.std += x_dif * x_dif, y_st.std += y_dif * y_dif;
		cov += x_dif * y_dif;
	}
	// 除数过小就不算了
	double std_pro = sqrt( x_st.std ) * sqrt( y_st.std );
	if( abs( std_pro ) < 1E-10 )
		return 1.;

	return cov / std_pro;
};
*/

double pearson( NumPairs_t& nps_ ) {
	// 空集的结果是"NaN"
	if( nps_.empty() )
		return std::numeric_limits<double>::quiet_NaN();
	// 只有一个样本
	if( nps_.size() == 1 )
		return 1.;

	Statistic_t x_st {}, y_st {};
	x_st.cnt = y_st.cnt = static_cast<int64_t>( nps_.size() );

	// 总和
	for( const auto& p : nps_ )
		x_st.sum += p.x, y_st.sum += p.y;

	// 平均值
	x_st.avg = x_st.sum / x_st.cnt, y_st.avg = y_st.sum / y_st.cnt;

	// 标准差、协方差
	double cov = 0.;
	for( const auto& p : nps_ ) {
		double x_dif = p.x - x_st.avg, y_dif = p.y - y_st.avg;
		x_st.std += x_dif * x_dif, y_st.std += y_dif * y_dif;
		cov += x_dif * y_dif;
	}
	// 除数过小就不算了
	double std_pro = sqrt( x_st.std ) * sqrt( y_st.std );
	if( std::abs( std_pro ) < 1E-10 )
		return 1.;

	return cov / std_pro;
};

double spearman( NumPairs_t& nps_ ) {
	/* 思路:
	 * 1.先得出每个X的序号、每个Y的序号
	 * 2.然后计算两个序号序列的 Pearson 相关系数,即为结果
	 */
	/* 反正可以修改原始数据,干脆连以下结构都不要,直接在原始数据上填入序号
	struct NumOrdr_t {
		// 这不是 x 的原值,而是排序后的序号! 之所以要同名,是为了重用 _PearsnCorr 函数
		double x;

		// 这不是 y 的原值,而是排序后的序号! 之所以要同名,是为了重用 _PearsnCorr 函数
		double y;

		const NumPair_t* np;
	}; */

	// 1. 先按 x 值排序, 并以序号代替 x 字段的原始值
	sort( nps_.begin(), nps_.end(), []( const NumPair_t& l, const NumPair_t& r ) {
		return l.x < r.x;
	} );
	double ord = 0.;
	for( auto& p : nps_ )
		p.x = ord++;

	// 2. 按 y 值排序, 并以序号代替 y 字段的原始值
	sort( nps_.begin(), nps_.end(), []( const NumPair_t& l, const NumPair_t& r ) {
		return l.y < r.y;
	} );
	ord = 0.;
	for( auto& p : nps_ )
		p.y = ord++;

	// 3. 两个序号构成的序列的 Pearson 相关系数, 就是 Spearman 相关系数
	return pearson( nps_ );
};

};  // namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
