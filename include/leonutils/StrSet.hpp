#pragma once
#include <set>
#include <string>

namespace leon_utl {

using ost_t = std::ostream;
using str_t = std::string;
using StrSet_t = std::set<str_t>;

// 切分字符串到集合
StrSet_t split2set( const str_t& src, char delimiter );

// 字符串集合输出
str_t set2str( const StrSet_t& );

// 字符串集合求并集
StrSet_t union_set( const StrSet_t&, const StrSet_t& );

// 字符串集合求差集
StrSet_t diffr_set( const StrSet_t&, const StrSet_t& );

// 字符串集合求交集
StrSet_t intersect( const StrSet_t&, const StrSet_t& );

}; //namespace leon_utl

// 必须放在 namespace 之外
std::ostream& operator<<( std::ostream&, const leon_utl::StrSet_t& );

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
