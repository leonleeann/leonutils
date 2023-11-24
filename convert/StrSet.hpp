#pragma once
#include <set>
#include <string>

namespace leon_utl {

using StrSet_t = std::set<std::string>;

// 切分字符串到集合
StrSet_t split2set( const std::string& src, char delimiter );

// 字符串集合输出
std::string set2str( const StrSet_t& );

/* 输出字符串集合(好像gcc-12的重载决议会用到系统库,还没找到原因,只好暂时不用流式输出)
std::ostream& operator<<( std::ostream&, const StrSet_t& ); */
std::ostream& output( std::ostream&, const StrSet_t& );

// 字符串集合求并集
StrSet_t union_of( const StrSet_t&, const StrSet_t& );

// 字符串集合求差集
StrSet_t differ_of( const StrSet_t&, const StrSet_t& );

// 字符串集合求交集
StrSet_t intersect( const StrSet_t&, const StrSet_t& );

}; //namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
