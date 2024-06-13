#pragma once
#include <vector>

namespace leon_utl {

double average( const std::vector<double>& );

// 计算一个数列的R²(不是可决系数!), 常用于评估一个净值序列与时间的相关性(盈利稳定性)
// 输入: 按时间顺序排列的净值, 输出: 净值与排列顺序的相关性
// double CoeOfDeterm( const std::vector<double>& samples, int sample_count, double avg );
double r_square( const std::vector<double>& samples, double avg );

struct Statistic_t {
	Statistic_t() = default;

	/* 一次扫描计算出 总和、最大值、最小值、中位数、平均值、标准差
	样本容器用 vector 而非 forward_list/list, 因为 vector 的方括号操作可快速找到中位数.
	而且样本数 < 4096 时, 其实 vector 更快, 因为有 cpu cache
	*/
	Statistic_t( std::vector<double>& samples );
	bool operator()( std::vector<double>& samples );

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
using NumPairs_t = std::vector<NumPair_t>;

// 计算 Pearson 相关系数(会改变输入序列!!!)
double pearson( NumPairs_t& );

// 计算 Spearman 相关系数(会改变输入序列!!!)
double spearman( NumPairs_t& );

};  // namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
