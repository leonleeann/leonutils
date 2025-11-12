#include <gtest/gtest.h>
#include <limits>

#include "leonutils/SetOps.hpp"

using namespace leon_utl;
using IntMap_t = map_t<int, int>;
using IntSet_t = set_t<int>;

TEST( TestSetOps, setOperators ) {
	IntSet_t	set1 { 1, 2, 3, 4, };
	IntSet_t	set2 { 4, 5, 6, 7, };

	ASSERT_EQ( union_set( set1, set2 ), ( IntSet_t{ 1, 2, 3, 4, 5, 6, 7 } ) );
	ASSERT_EQ( diffr_set( set1, set2 ), ( IntSet_t{ 1, 2, 3 } ) );
	ASSERT_EQ( diffr_set( set2, set1 ), ( IntSet_t{ 5, 6, 7 } ) );
	ASSERT_EQ( intersect( set1, set2 ), IntSet_t{ 4 } );
	ASSERT_EQ( intersect( set2, set1 ), IntSet_t{ 4 } );

	/*	ASSERT_EQ( set1 += set2, ( IntSet_t{ 1, 2, 3, 4, 5, 6, 7 } ) );
		ASSERT_EQ( set1, ( IntSet_t{ 1, 2, 3, 4, 5, 6, 7 } ) );
		set1 = IntSet_t{ 1, 2, 3, 4, };
	//	set2 = IntSet_t{ 4, 5, 6, 7, };
		ASSERT_EQ( set1 -= set2, ( IntSet_t{ 1, 2, 3 } ) );
		ASSERT_EQ( set1, ( IntSet_t{ 1, 2, 3 } ) ); */

	set1 = IntSet_t{ 1, 2, 3, 4, };
	set2 = IntSet_t{};
	ASSERT_EQ( union_set( set1, set2 ), set1 );
	ASSERT_EQ( diffr_set( set1, set2 ), set1 );
	ASSERT_EQ( diffr_set( set2, set1 ), IntSet_t{} );
	ASSERT_EQ( intersect( set1, set2 ), IntSet_t{} );
	ASSERT_EQ( intersect( set2, set1 ), IntSet_t{} );

	set1 = IntSet_t{};
	set2 = IntSet_t{ 4, 5, 6, 7, };
	ASSERT_EQ( union_set( set1, set2 ), set2 );
	ASSERT_EQ( diffr_set( set1, set2 ), IntSet_t{} );
	ASSERT_EQ( diffr_set( set2, set1 ), set2 );
	ASSERT_EQ( intersect( set1, set2 ), IntSet_t{} );
	ASSERT_EQ( intersect( set2, set1 ), IntSet_t{} );
};

/*
TEST( TestSetOps, mapOperators ) {
	IntMap_t	map1 { {1, 1}, {2, 2}, {3, 3}, {4, 4} };
	IntMap_t	map2 { {4, 4}, {5, 5}, {6, 6}, {7, 7} };

	ASSERT_EQ( map1 - map2, ( IntMap_t{ {1, 1}, {2, 2}, {3, 3} } ) );
	ASSERT_EQ( map2 - map1, ( IntMap_t{ {5, 5}, {6, 6}, {7, 7} } ) );
	ASSERT_EQ( map1 * map2, ( IntMap_t{ {4, 4} } ) );
	ASSERT_EQ( map2 * map1, ( IntMap_t{ {4, 4} } ) );

	ASSERT_EQ( map1 -= map2, ( IntMap_t{ {1, 1}, {2, 2}, {3, 3} } ) );
	ASSERT_EQ( map1, ( IntMap_t{ {1, 1}, {2, 2}, {3, 3} } ) );
	ASSERT_EQ( map2 -= map1, ( IntMap_t{ {4, 4}, {5, 5}, {6, 6}, {7, 7} } ) );
	ASSERT_EQ( map2, ( IntMap_t{ {4, 4}, {5, 5}, {6, 6}, {7, 7} } ) );
};

TEST( TestSetOps, crossOperators ) {
	IntSet_t	set1 { 1, 2, 3, 4, };
//	IntMap_t	map1 { {1,1}, {2,2}, {3,3}, {4,4} };
	IntMap_t	map2 { {4, 4}, {5, 5}, {6, 6}, {7, 7} };

	ASSERT_EQ( set1 + map2, ( IntSet_t{ 1, 2, 3, 4, 5, 6, 7 } ) );
	ASSERT_EQ( set1 - map2, ( IntSet_t{ 1, 2, 3 } ) );
	ASSERT_EQ( map2 - set1, ( IntMap_t{ {5, 5}, {6, 6}, {7, 7} } ) );
	ASSERT_EQ( set1 * map2, ( IntSet_t{ 4 } ) );
	ASSERT_EQ( map2 * set1, ( IntMap_t{ {4, 4} } ) );

	ASSERT_EQ( set1 -= map2, ( IntSet_t{ 1, 2, 3 } ) );
	ASSERT_EQ( set1, ( IntSet_t{ 1, 2, 3 } ) );
	set1 = IntSet_t{ 1, 2, 3, 4, };

	ASSERT_EQ( map2, ( IntMap_t{ {4, 4}, {5, 5}, {6, 6}, {7, 7} } ) );
	ASSERT_EQ( map2 -= set1, ( IntMap_t{ {5, 5}, {6, 6}, {7, 7} } ) );
	ASSERT_EQ( map2, ( IntMap_t{ {5, 5}, {6, 6}, {7, 7} } ) );
};

TEST( TestStrSet, crossValidate ) {
	StrSet_t	set1 { "AAA", "BBB", "CCC", };
	StrSet_t	set2 { "CCC", "DDD", "EEE", };
	ASSERT_EQ( set1 + set2, union_set( set1, set2 ) );
	ASSERT_EQ( set1 - set2, diffr_set( set1, set2 ) );
	ASSERT_EQ( set2 - set1, diffr_set( set2, set1 ) );
	ASSERT_EQ( set1 * set2, intersect( set1, set2 ) );
	ASSERT_EQ( set2 * set1, intersect( set2, set1 ) );

//	set1 = StrSet_t{ "AAA", "BBB", "CCC", };
	set2 = StrSet_t{};
	ASSERT_EQ( set1 + set2, union_set( set1, set2 ) );
	ASSERT_EQ( set1 - set2, diffr_set( set1, set2 ) );
	ASSERT_EQ( set2 - set1, diffr_set( set2, set1 ) );
	ASSERT_EQ( set1 * set2, intersect( set1, set2 ) );
	ASSERT_EQ( set2 * set1, intersect( set2, set1 ) );
};
*/

TEST( TestStrSets, splitStr2set ) {
	str_t s = "abc,def,ghi";
	StrSet_t tokens = split2set( s, ',' );
	ASSERT_TRUE( tokens.contains( "abc" ) );
	ASSERT_TRUE( tokens.contains( "def" ) );
	ASSERT_TRUE( tokens.contains( "ghi" ) );
	ASSERT_EQ( tokens.size(), 3 );

	s = "abc,def,ghi,";
	tokens = split2set( s, ',' );
	ASSERT_TRUE( tokens.contains( "abc" ) );
	ASSERT_TRUE( tokens.contains( "def" ) );
	ASSERT_TRUE( tokens.contains( "ghi" ) );
// 	lg_debg << set2str( tokens );
	ASSERT_EQ( tokens.size(), 3 );

	s = "abc%def%ghi%";
	tokens = split2set( s, ' ' );
// 	lg_debg << set2str( tokens );
	ASSERT_TRUE( tokens.contains( "abc%def%ghi%" ) );
	ASSERT_EQ( tokens.size(), 1 );

	s = "abc def ghi ";
	tokens = split2set( s, ' ' );
	ASSERT_TRUE( tokens.contains( "abc" ) );
	ASSERT_TRUE( tokens.contains( "def" ) );
	ASSERT_TRUE( tokens.contains( "ghi" ) );
	ASSERT_EQ( tokens.size(), 3 );

	s = "";
	tokens = split2set( s, ',' );
	ASSERT_TRUE( tokens.empty() );

	s = ",,";
	tokens = split2set( s, ',' );
	ASSERT_TRUE( tokens.empty() );

	s = " , , ";
	std::remove( s.begin(), s.end(), ' ' );
};

TEST( TestIntSets, splitStr2IntSet ) {
	str_t s = "789,123,456,";
	IntSet_t ints = split2set<int>( s, ',' );
	ASSERT_TRUE( ints.contains( 123 ) );
	ASSERT_TRUE( ints.contains( 456 ) );
	ASSERT_TRUE( ints.contains( 789 ) );
	ASSERT_FALSE( ints.contains( 0 ) );
	ASSERT_FALSE( ints.contains( -1 ) );

	s = to_str( ints, ':' );
	ASSERT_EQ( s, "123:456:789" );

	// 测试大整数
	s = std::to_string( std::numeric_limits<int32_t>::max() ) + ',' +
		std::to_string( std::numeric_limits<int32_t>::min() ) + ",-1";
	auto i32_set = split2set<int32_t>( s, ',' );
	ASSERT_EQ( i32_set.size(), 3 );
	ASSERT_TRUE( i32_set.contains( std::numeric_limits<int32_t>::max() ) );
	ASSERT_TRUE( i32_set.contains( std::numeric_limits<int32_t>::min() ) );
	ASSERT_TRUE( i32_set.contains( -1 ) );
	std::cout << i32_set << std::endl;

	s = std::to_string( std::numeric_limits<uint32_t>::max() ) + ',' +
		std::to_string( std::numeric_limits<uint32_t>::min() ) + ",-1";
	auto u32_set = split2set<uint32_t>( s, ',' );
	ASSERT_EQ( u32_set.size(), 2 );
	ASSERT_TRUE( u32_set.contains( std::numeric_limits<uint32_t>::max() ) );
	ASSERT_TRUE( u32_set.contains( std::numeric_limits<uint32_t>::min() ) );
	std::cout << u32_set << std::endl;

	s = std::to_string( std::numeric_limits<int64_t>::max() ) + ',' +
		std::to_string( std::numeric_limits<int64_t>::min() ) + ",-1";
	auto i64_set = split2set<int64_t>( s, ',' );
	ASSERT_EQ( i64_set.size(), 3 );
	ASSERT_TRUE( i64_set.contains( std::numeric_limits<int64_t>::max() ) );
	ASSERT_TRUE( i64_set.contains( std::numeric_limits<int64_t>::min() ) );
	ASSERT_TRUE( i64_set.contains( -1 ) );
	std::cout << i64_set << std::endl;

	s = std::to_string( std::numeric_limits<uint64_t>::max() ) + ',' +
		std::to_string( std::numeric_limits<uint64_t>::min() ) + ",-1";
	auto u64_set = split2set<uint64_t>( s, ',' );
	ASSERT_EQ( u64_set.size(), 2 );
	ASSERT_TRUE( u64_set.contains( std::numeric_limits<uint64_t>::max() ) );
	ASSERT_TRUE( u64_set.contains( std::numeric_limits<uint64_t>::min() ) );
	std::cout << u64_set << std::endl;

	// 测试超界整数(明明是32位的数,故意让它按16位解析)
	s = std::to_string( std::numeric_limits<int32_t>::max() ) + ",,,";
	set_t<int16_t> i16_set = split2set<int16_t>( s, ',' );
	std::cout << "原始串:" << s << ",解析集:" << i16_set << std::endl;
	ASSERT_EQ( i16_set, set_t<int16_t> {-1} );

	s = ",,," + std::to_string( std::numeric_limits<int32_t>::min() );
	i16_set = split2set<int16_t>( s, ',' );
	std::cout << "原始串:" << s << ",解析集:" << i16_set << std::endl;
	ASSERT_EQ( i16_set, set_t<int16_t> {0} );
};

TEST( TestStrSets, set2str ) {
	StrSet_t tokens {};
	ASSERT_EQ( to_str( tokens, ',' ), "" );

	tokens.insert( "abc" );
	tokens.insert( "def" );
	tokens.insert( "ghi" );
	ASSERT_EQ( to_str( tokens, ',' ), "abc,def,ghi" );

	tokens.erase( "ghi" );
	ASSERT_EQ( to_str( tokens, ',' ), "abc,def" );

	tokens.clear();
	ASSERT_EQ( to_str( tokens, ',' ), "" );
};

TEST( TestStrSets, outputStrSet ) {
	StrSet_t tokens {};
	ASSERT_EQ( to_str( tokens, ':' ), "" );

	tokens.insert( "abc" );
	tokens.insert( "def" );
	tokens.insert( "ghi" );
	ASSERT_EQ( to_str( tokens, ':' ), "abc:def:ghi" );

	tokens.erase( "ghi" );
	ASSERT_EQ( to_str( tokens, ':' ), "abc:def" );

	tokens.clear();
	ASSERT_EQ( to_str( tokens, ':' ), "" );
};

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
