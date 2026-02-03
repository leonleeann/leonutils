#pragma once
#include <cmath>		// abs, ceil, floor, isnan, log, log10, log2, pow, round, sqrt
#include <concepts>
#include <limits>
#include <vector>

template<typename T, typename A = std::allocator<T> >
using vct_t = std::vector<T, A>;
using NumbVect_t = vct_t<double>;

namespace leon_utl {

// 指数式移动平均数计算器
template<typename T> requires std::floating_point<T>
struct EmaTracer_t {
	T old_w { 0.999 };	// 旧值权重
	T new_w { 0.001 };	// 新值权重
	T ema_v { 0.000 };	// 当前ema

	T update( T new_ ) {
		return ema_v = ema_v * old_w + new_ * new_w;
	};

	T get() const { return ema_v; };
};

// "增量式"统计, 不能保留全部样本时使用, 每次更新时积累中间变量, 最后算出标准差
struct IncStat_t {
	int64_t		_max { std::numeric_limits<int64_t>::lowest() };
	int64_t		_min { std::numeric_limits<int64_t>::max() };
	int64_t		_cnt {};
	double		_sum {};
	double		_sqr {};	// 所有样本的平方之和, 用于增量式(不保留样本)计算标准差

	// 为让开始的n次统计比较平稳(不会得到太小的标准差), 可以初始化一下, 假装已经统计了很多次
	void initialize( int64_t cnt_, double avg_, double std_ ) {
		_max = std::numeric_limits<int64_t>::lowest();
		_min = std::numeric_limits<int64_t>::max();

		// 给 _sqr 的值相当于做 std() 函数的逆运算
		_cnt = cnt_;
		_sum = avg_ * cnt_;
		auto avg_p2 = avg_ * avg_;
		auto std_p2 = std_ * std_;
		_sqr = std_p2 * ( cnt_ - 1 ) - avg_p2 * cnt_ + 2 * _sum * avg_;
	};

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
