#pragma once
#include <map>
#include <set>

namespace leon_utl {

using std::map;
using std::set;

//==== 求并集 ===================================================================
template<typename K>
set<K> operator+( const set<K>& s0_, const set<K>& s1_ ) {
	set<K> result;
	std::set_union( s0_.cbegin(), s0_.cend(), s1_.cbegin(), s1_.cend(),
					std::inserter( result, result.begin() ) );
	return result;
};

template<typename K, typename V>
set<K> operator+( const set<K>& s0_, const map<K, V>& s1_ ) {
	set<K> result { s0_ };
	for( const auto&[k, v] : s1_ )
		result.insert( k );
	return result;
};

template<typename K>
set<K>& operator+=( set<K>& s0_, const set<K>& s1_ ) {
	for( const auto& k : s1_ )
		s0_.insert( k );
	return s0_;
};

template<typename K, typename V>
set<K>& operator+=( set<K>& s0_, const map<K, V>& s1_ ) {
	for( const auto&[k, v] : s1_ )
		s0_.insert( k );
	return s0_;
};

//==== 求差集 ===================================================================
template<typename K>
set<K> operator -( const set<K>& s0_, const set<K>& s1_ ) {
	set<K> result;
	std::set_difference( s0_.cbegin(), s0_.cend(), s1_.cbegin(), s1_.cend(),
						 std::inserter( result, result.begin() ) );
	return result;
};

template<typename K, typename V>
map<K, V> operator -( const map<K, V>& s0_, const map<K, V>& s1_ ) {
	map<K, V> result;
	for( const auto&[k, v] : s0_ )
		if( ! s1_.contains( k ) )
			result[k] = v;
	return result;
};

template<typename K, typename V>
set<K> operator -( const set<K>& s0_, const map<K, V>& s1_ ) {
	set<K> result;
	for( const auto& k : s0_ )
		if( ! s1_.contains( k ) )
			result.insert( k );
	return result;
};

template<typename K, typename V>
map<K, V> operator -( const map<K, V>& s0_, const set<K>& s1_ ) {
	map<K, V> result;
	for( const auto&[k, v] : s0_ )
		if( ! s1_.contains( k ) )
			result[k] = v;
	return result;
};

template<typename K>
set<K>& operator-=( set<K>& s0_, const set<K>& s1_ ) {
	for( const auto& k : s1_ )
		s0_.erase( k );
	return s0_;
};

template<typename K, typename V>
map<K, V>& operator-=( map<K, V>& s0_, const map<K, V>& s1_ ) {
	for( const auto&[k, v] : s1_ )
		s0_.erase( k );
	return s0_;
};

template<typename K, typename V>
set<K>& operator-=( set<K>& s0_, const map<K, V>& s1_ ) {
	for( const auto&[k, v] : s1_ )
		s0_.erase( k );
	return s0_;
};

template<typename K, typename V>
map<K, V>& operator-=( map<K, V>& s0_, const set<K>& s1_ ) {
	for( const auto& k : s1_ )
		s0_.erase( k );
	return s0_;
};

//==== 求交集 ===================================================================
template<typename K>
set<K> operator *( const set<K>& s0_, const set<K>& s1_ ) {
	set<K> result;
	std::set_intersection( s0_.cbegin(), s0_.cend(), s1_.cbegin(), s1_.cend(),
						   std::inserter( result, result.begin() ) );
	return result;
};

template<typename K, typename V>
map<K, V> operator *( const map<K, V>& s0_, const map<K, V>& s1_ ) {
	map<K, V> result;
	for( const auto&[k, v] : s0_ )
		if( s1_.contains( k ) )
			result[k] = v;
	return result;
};

template<typename K, typename V>
set<K> operator *( const set<K>& s0_, const map<K, V>& s1_ ) {
	set<K> result;
	for( const auto& k : s0_ )
		if( s1_.contains( k ) )
			result.insert( k );
	return result;
};

template<typename K, typename V>
map<K, V> operator *( const map<K, V>& s0_, const set<K>& s1_ ) {
	map<K, V> result;
	for( const auto&[k, v] : s0_ )
		if( s1_.contains( k ) )
			result[k] = v;
	return result;
};

}; //namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
