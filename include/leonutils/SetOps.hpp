#pragma once
#include <algorithm>	// set_union, set_difference, set_intersection
#include <concepts>
#include <cstdlib>		// atoi, atol, atof, strtol, strtoul, strtof, strtod
#include <leonutils/Converts.hpp>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <type_traits>

template<typename K, typename V, typename C = std::less<K>,
		 typename A = std::allocator<std::pair<const K, V> > >
using map_t = std::map<K, V, C, A>;

template<typename K, typename C = std::less<K>, typename A = std::allocator<K> >
using set_t = std::set<K, C, A>;

using oss_t = std::ostringstream;
using ost_t = std::ostream;
using str_t = std::string;
using str_cr = const str_t&;
using strset_t = set_t<str_t>;
using intset_t = set_t<int>;
using i32set_t = set_t<int32_t>;
using u32set_t = set_t<uint32_t>;
using i64set_t = set_t<int64_t>;
using u64set_t = set_t<uint64_t>;

namespace leon_utl {

//==== 交叉转换 =================================================================

// 切分字符串到集合
template<typename T = str_t>
set_t<T> split2set( str_cr src, char delimiter ) {

	set_t<T> result;
	if( src.empty() )
		return result;

	size_t pos = 0, end;
	while( ( end = src.find( delimiter, pos ) ) != src.npos ) {
		auto substr = trim( src.substr( pos, end - pos ) );
		if( ! substr.empty() )
			result.insert( T{ substr } );
		pos = end + 1;
	}

	auto substr = src.substr( pos );
	if( ! substr.empty() )
		result.insert( T{ substr } );
	return result;
};

template<std::integral T>
set_t<T> split2set( str_cr src, char delimiter ) {

	auto str_set = split2set<str_t>( src, delimiter );
	set_t<T> result;
	if( str_set.empty() )
		return result;

	for( const auto& s : str_set ) {
		if( s.empty() )
			continue;
		if( std::is_signed_v<T> )
			result.insert( std::stoll( s ) );
		else
			result.insert( std::stoull( s ) );
	}

	return result;
};

// 集合转为字符串
template<typename K, typename C = std::less<K>, typename A = std::allocator<K> >
str_t to_str( const set_t<K,C,A>& v_set, char delimiter ) {

	oss_t oss;
	bool first = true;

	for( const auto& v : v_set ) {
		if( !first )
			oss << delimiter;
		first = false;

		oss << v;
	}

	return oss.str();
};

template <typename K, typename V, typename C, typename SA, typename MA>
void assign( set_t<K, C, SA>& keys_, const map_t<K, V, C, MA>& map_ ) {
	keys_.clear();
	for( auto&[k, v] : map_ )
		keys_.insert( k );
};

template <typename K, typename V, typename C, typename A>
set_t<K, C> keys( const map_t<K, V, C, A>& map_ ) {
	set_t<K, C> keys;
	for( auto&[k, v] : map_ )
		keys.insert( k );
	return keys;
};

template <typename K, typename C, typename A>
strset_t keys( const set_t<K, C, A>& src_ ) {
	strset_t tgt;
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
/*
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
*/

//==== 求差集 ===================================================================
template <typename K, typename C, typename A>
set_t<K, C, A> diffr_set( const set_t<K, C, A>& s0_, const set_t<K, C, A>& s1_ ) {
	set_t<K, C, A> result;
	std::set_difference( s0_.cbegin(), s0_.cend(), s1_.cbegin(), s1_.cend(),
						 std::inserter( result, result.begin() ) );
	return result;
};
/*
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
*/

//==== 求交集 ===================================================================
template <typename K, typename C, typename A>
set_t<K, C, A> intersect( const set_t<K, C, A>& s0_, const set_t<K, C, A>& s1_ ) {
	set_t<K, C, A> result;
	std::set_intersection( s0_.cbegin(), s0_.cend(), s1_.cbegin(), s1_.cend(),
						   std::inserter( result, result.begin() ) );
	return result;
};
/*
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
*/

}; //namespace leon_utl

// 好像运算符重载要放入 std::ostream 所在的命名空间才能匹配到
namespace std {

template<typename K, typename C = std::less<K>, typename A = std::allocator<K> >
ost_t& operator<<( ost_t& os_, const set_t<K, C, A>& set_ ) {
	os_ << '{';

	for( auto& v : set_ )
		os_ << v << ',';

	return os_ << '}';
};

};	// namespace std

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
