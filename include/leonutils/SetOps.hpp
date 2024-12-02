#pragma once
#include <algorithm>	// set_union, set_difference, set_intersection
#include <map>
#include <set>
#include <string>

template<typename K, typename V, typename C = std::less<K>,
		 typename A = std::allocator<std::pair<const K, V> > >
using map_t = std::map<K, V, C, A>;

template<typename K, typename C = std::less<K>, typename A = std::allocator<K> >
using set_t = std::set<K, C, A>;

using str_t = std::string;
using StrSet_t = set_t<str_t>;

using ost_t = std::ostream;

namespace leon_utl {

//==== 交叉转换 =================================================================
// 切分字符串到集合
StrSet_t split2set( const str_t& src, char delimiter );

// 字符串集合输出
str_t set2str( const StrSet_t& );

template <typename K, typename V, typename C, typename SA, typename MA>
void assign( set_t<K, C, SA>& keys_, const map_t<K, V, C, MA>& map_ ) {
	keys_.clear();
	for( auto&[k, v] : map_ )
		keys_.insert( k );
};

template <typename K, typename V, typename C, typename A>
set_t<K, C> keys( const map_t<K, V, C, A>& map_ ) {
	set_t<K, C> keys;
	assign( keys, map_ );
	return keys;
};

template <typename K, typename C, typename A>
StrSet_t keys( const set_t<K, C, A>& src_ ) {
	StrSet_t tgt;
	for( auto& s : src_ )
		tgt.insert( s.str() );
	return tgt;
};

//==== 求并集 ===================================================================
template <typename K, typename C, typename A>
set_t<K, C, A> union_set( const set_t<K, C, A>& s0_, const set_t<K, C, A>& s1_ ) {
	set_t<K, C, A> result;
	std::set_union( s0_.cbegin(), s0_.cend(), s1_.cbegin(), s1_.cend(),
					std::inserter( result, result.begin() ) );
	return result;
};

template <typename K, typename C, typename A>
set_t<K, C, A> operator +( const set_t<K, C, A>& s0_, const set_t<K, C, A>& s1_ ) {
	set_t<K, C, A> result;
	std::set_union( s0_.cbegin(), s0_.cend(), s1_.cbegin(), s1_.cend(),
					std::inserter( result, result.begin() ) );
	return result;
};

template <typename K, typename V, typename C, typename SA, typename MA>
set_t<K, C, SA> operator +( const set_t<K, C, SA>& s0_, const map_t<K, V, C, MA>& s1_ ) {
	set_t<K, C, SA> result { s0_ };
	for( const auto&[k, v] : s1_ )
		result.insert( k );
	return result;
};

template <typename K, typename C, typename A>
set_t<K, C, A>& operator+=( set_t<K, C, A>& s0_, const set_t<K, C, A>& s1_ ) {
	for( const auto& k : s1_ )
		s0_.insert( k );
	return s0_;
};

template <typename K, typename V, typename C, typename SA, typename MA>
set_t<K, C, SA>& operator+=( set_t<K, C, SA>& s0_, const map_t<K, V, C, MA>& s1_ ) {
	for( const auto&[k, v] : s1_ )
		s0_.insert( k );
	return s0_;
};

//==== 求差集 ===================================================================
template <typename K, typename C, typename A>
set_t<K, C, A> diffr_set( const set_t<K, C, A>& s0_, const set_t<K, C, A>& s1_ ) {
	set_t<K, C, A> result;
	std::set_difference( s0_.cbegin(), s0_.cend(), s1_.cbegin(), s1_.cend(),
						 std::inserter( result, result.begin() ) );
	return result;
};

template <typename K, typename C, typename A>
set_t<K, C, A> operator -( const set_t<K, C, A>& s0_, const set_t<K, C, A>& s1_ ) {
	set_t<K, C, A> result;
	std::set_difference( s0_.cbegin(), s0_.cend(), s1_.cbegin(), s1_.cend(),
						 std::inserter( result, result.begin() ) );
	return result;
};

template <typename K, typename V, typename C, typename A>
map_t<K, V, C, A> operator -( const map_t<K, V, C, A>& s0_, const map_t<K, V, C, A>& s1_ ) {
	map_t<K, V, C, A> result;
	for( const auto&[k, v] : s0_ )
		if( ! s1_.contains( k ) )
			result[k] = v;
	return result;
};

template <typename K, typename V, typename C, typename SA, typename MA>
set_t<K, C, SA> operator -( const set_t<K, C, SA>& s0_, const map_t<K, V, C, MA>& s1_ ) {
	set_t<K, C, SA> result;
	for( const auto& k : s0_ )
		if( ! s1_.contains( k ) )
			result.insert( k );
	return result;
};

template <typename K, typename V, typename C, typename SA, typename MA>
map_t<K, V, C, MA> operator -( const map_t<K, V, C, MA>& s0_, const set_t<K, C, SA>& s1_ ) {
	map_t<K, V, C, MA> result;
	for( const auto&[k, v] : s0_ )
		if( ! s1_.contains( k ) )
			result[k] = v;
	return result;
};

template <typename K, typename C, typename A>
set_t<K, C, A>& operator-=( set_t<K, C, A>& s0_, const set_t<K, C, A>& s1_ ) {
	for( const auto& k : s1_ )
		s0_.erase( k );
	return s0_;
};

template <typename K, typename V, typename C, typename A>
map_t<K, V, C, A>& operator-=( map_t<K, V, C, A>& s0_, const map_t<K, V, C, A>& s1_ ) {
	for( const auto&[k, v] : s1_ )
		s0_.erase( k );
	return s0_;
};

template <typename K, typename V, typename C, typename SA, typename MA>
set_t<K, C, SA>& operator-=( set_t<K, C, SA>& s0_, const map_t<K, V, C, MA>& s1_ ) {
	for( const auto&[k, v] : s1_ )
		s0_.erase( k );
	return s0_;
};

template <typename K, typename V, typename C, typename SA, typename MA>
map_t<K, V, C, MA>& operator-=( map_t<K, V, C, MA>& s0_, const set_t<K, C, SA>& s1_ ) {
	for( const auto& k : s1_ )
		s0_.erase( k );
	return s0_;
};

//==== 求交集 ===================================================================
template <typename K, typename C, typename A>
set_t<K, C, A> intersect( const set_t<K, C, A>& s0_, const set_t<K, C, A>& s1_ ) {
	set_t<K, C, A> result;
	std::set_intersection( s0_.cbegin(), s0_.cend(), s1_.cbegin(), s1_.cend(),
						   std::inserter( result, result.begin() ) );
	return result;
};

template <typename K, typename C, typename A>
set_t<K, C, A> operator *( const set_t<K, C, A>& s0_, const set_t<K, C, A>& s1_ ) {
	set_t<K, C, A> result;
	std::set_intersection( s0_.cbegin(), s0_.cend(), s1_.cbegin(), s1_.cend(),
						   std::inserter( result, result.begin() ) );
	return result;
};

template <typename K, typename V, typename C, typename A>
map_t<K, V, C, A> operator *( const map_t<K, V, C, A>& s0_, const map_t<K, V, C, A>& s1_ ) {
	map_t<K, V, C, A> result;
	for( const auto&[k, v] : s0_ )
		if( s1_.contains( k ) )
			result[k] = v;
	return result;
};

template <typename K, typename V, typename C, typename SA, typename MA>
set_t<K, C, SA> operator *( const set_t<K, C, SA>& s0_, const map_t<K, V, C, MA>& s1_ ) {
	set_t<K, C, SA> result;
	for( const auto& k : s0_ )
		if( s1_.contains( k ) )
			result.insert( k );
	return result;
};

template <typename K, typename V, typename C, typename SA, typename MA>
map_t<K, V, C, MA> operator *( const map_t<K, V, C, MA>& s0_, const set_t<K, C, SA>& s1_ ) {
	map_t<K, V, C, MA> result;
	for( const auto&[k, v] : s0_ )
		if( s1_.contains( k ) )
			result[k] = v;
	return result;
};

}; //namespace leon_utl

// 有关 ostream 的运算符重载, 尽量不放任何命名空间内
ost_t& operator<<( ost_t&, const StrSet_t& );

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
