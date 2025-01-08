#pragma once
#include <cmath>		// abs, ceil, floor, isnan, log, log10, log2, pow, round, sqrt
#include <limits>
#include <vector>

template<typename T, typename A = std::allocator<T> >
using vct_t = std::vector<T, A>;
using NumbVect_t = vct_t<double>;

namespace leon_utl {

// "增量式"统计, 不能保留全部样本时使用, 每次更新时积累中间变量, 最后算出标准差
struct IncStat_t {
	int64_t		_max { std::numeric_limits<int64_t>::lowest() };
	int64_t		_min { std::numeric_limits<int64_t>::max() };
	int64_t		_cnt {};
	double		_sum {};
	double		_sqr {};	// 所有样本的平方之和, 用于增量式(不保留样本)计算标准差

	void update( int64_t d_ ) {
		_max = std::max( _max, d_ );
		_min = std::min( _min, d_ );
		++_cnt;
		double f_d = static_cast<double>( d_ );
		_sum += f_d;
		_sqr += f_d * f_d;
	};

	double avg() const {
		if( _cnt == 0 )
			return 0.0;
		else
			return _sum / _cnt;
	};

	double std() const {
		if( _cnt <= 1 )
			return 0.0;

		double a = avg();
		return std::sqrt( ( _sqr - 2. * _sum * a + a * a * _cnt )
						  / ( _cnt - 1 ) );
	};
};

double average( const NumbVect_t& );

// 计算一个数列的R²(不是可决系数!), 常用于评估一个净值序列与时间的相关性(盈利稳定性)
// 输入: 按时间顺序排列的净值, 输出: 净值与排列顺序的相关性
// double CoeOfDeterm( const NumsVect_t& samples, int sample_count, double avg );
double r_square( const NumbVect_t& samples, double avg );

struct Statistic_t {
	Statistic_t() = default;

	/* 一次扫描计算出 总和、最大值、最小值、中位数、平均值、标准差
	样本容器用 vector 而非 forward_list/list, 因为 vector 的方括号操作可快速找到中位数.
	而且样本数 < 4096 时, 其实 vector 更快, 因为有 cpu cache
	*/
	Statistic_t( NumbVect_t& samples );
	bool operator()( NumbVect_t& samples );

	int64_t	cnt;	// 样本数
	double	sum;	// 总和
	double	max;	// 最大值
	double	min;	// 最小值
	double	med;	// 中位数
	double	avg;	// 平均值
	double	std;	// 标准差
};

struct NumPair_t {
	double x;
	double y;
};
using NumPairs_t = vct_t<NumPair_t>;

// 计算 Pearson 相关系数(会改变输入序列!!!)
double pearson( NumPairs_t& );

// 计算 Spearman 相关系数(会改变输入序列!!!)
double spearman( NumPairs_t& );

};  // namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
