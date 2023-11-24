#pragma once
#include <forward_list>
#include <vector>

namespace leon_utl {

double Average( const std::forward_list<double>& );

// 计算一个数列的可决系数(R²), 常用于评估一个净值序列与时间的相关性(盈利稳定性)
// 输入: 按时间顺序排列的净值, 输出: 净值与排列顺序的相关性
double CoeOfDeterm( const std::forward_list<double>& samples, int sample_count, double avg );

struct StatisticResult_t {
	double	sum;	// 总和
	double	avg;	// 平均值
	double	med;	// 中位数
	double	max;	// 最大值
	double	min;	// 最小值
	double	std;	// 标准差
	int64_t	cnt;	// 样本数
};

// 一次扫描计算出 平均值、中位数、最大值、最小值、标准差
// 之所以样本参数用vector而不是forward_list/list,是因为vector的方括号操作可快速找到中位数
bool StatisticAll( std::vector<double>& samples, StatisticResult_t& result );

struct NumPair_t {
	double x;
	double y;
};
using NumPairs_t = std::vector<NumPair_t>;

// 计算 Pearson 相关系数(会改变输入序列!!!)
double PearsnCorr( NumPairs_t& );

// 计算 Spearman 相关系数(会改变输入序列!!!)
double SpearmCorr( NumPairs_t& );

};  // namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
