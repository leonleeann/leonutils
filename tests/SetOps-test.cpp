#include <gtest/gtest.h>

#include "leonutils/SetOps.hpp"
#include "leonutils/StrSet.hpp"

using namespace leon_utl;
using IntMap_t = map<int, int>;
using IntSet_t = set<int>;

TEST( TestSetOps, setOperators ) {
	IntSet_t	set1 { 1, 2, 3, 4, };
	IntSet_t	set2 { 4, 5, 6, 7, };

	ASSERT_EQ( set1 + set2, ( IntSet_t{ 1, 2, 3, 4, 5, 6, 7 } ) );
	ASSERT_EQ( set1 - set2, ( IntSet_t{ 1, 2, 3 } ) );
	ASSERT_EQ( set2 - set1, ( IntSet_t{ 5, 6, 7 } ) );
	ASSERT_EQ( set1 * set2, IntSet_t{ 4 } );
	ASSERT_EQ( set2 * set1, IntSet_t{ 4 } );

	ASSERT_EQ( set1 += set2, ( IntSet_t{ 1, 2, 3, 4, 5, 6, 7 } ) );
	ASSERT_EQ( set1, ( IntSet_t{ 1, 2, 3, 4, 5, 6, 7 } ) );

	set1 = IntSet_t{ 1, 2, 3, 4, };
//	set2 = IntSet_t{ 4, 5, 6, 7, };
	ASSERT_EQ( set1 -= set2, ( IntSet_t{ 1, 2, 3 } ) );
	ASSERT_EQ( set1, ( IntSet_t{ 1, 2, 3 } ) );

	set1 = IntSet_t{ 1, 2, 3, 4, };
	set2 = IntSet_t{};
	ASSERT_EQ( set1 + set2, set1 );
	ASSERT_EQ( set1 - set2, set1 );
	ASSERT_EQ( set2 - set1, IntSet_t{} );
	ASSERT_EQ( set1 * set2, IntSet_t{} );
	ASSERT_EQ( set2 * set1, IntSet_t{} );

	set1 = IntSet_t{};
	set2 = IntSet_t{ 4, 5, 6, 7, };
	ASSERT_EQ( set1 + set2, set2 );
	ASSERT_EQ( set1 - set2, IntSet_t{} );
	ASSERT_EQ( set2 - set1, set2 );
	ASSERT_EQ( set1 * set2, IntSet_t{} );
	ASSERT_EQ( set2 * set1, IntSet_t{} );
};

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

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
