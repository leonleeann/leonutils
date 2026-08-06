#include <gmock/gmock.h>

#include "leonutils/ArrayUtils.hpp"

using namespace leon_utl;

TEST( TestArrayUtils, equivalenceI32 ) {
	array<int32_t, 5> i32_arr0 {}, i32_arr1 {};
	ASSERT_TRUE( eq( i32_arr0, i32_arr1 ) );

	i32_arr0 = i32_arr1;
	++i32_arr0[0];
	ASSERT_FALSE( eq( i32_arr0, i32_arr1 ) );

	i32_arr0 = i32_arr1;
	++i32_arr0[1];
	ASSERT_FALSE( eq( i32_arr0, i32_arr1 ) );

	i32_arr0 = i32_arr1;
	++i32_arr0[2];
	ASSERT_FALSE( eq( i32_arr0, i32_arr1 ) );

	i32_arr0 = i32_arr1;
	++i32_arr0[3];
	ASSERT_FALSE( eq( i32_arr0, i32_arr1 ) );

	i32_arr0 = i32_arr1;
	++i32_arr0[4];
	ASSERT_FALSE( eq( i32_arr0, i32_arr1 ) );
};

TEST( TestArrayUtils, equivalenceF32 ) {
	array<float, 5> f32_arr0 {}, f32_arr1 {};
	ASSERT_TRUE( eq( f32_arr0, f32_arr1 ) );

	f32_arr0 = f32_arr1;
	++f32_arr0[0];
	ASSERT_FALSE( eq( f32_arr0, f32_arr1 ) );

	f32_arr0 = f32_arr1;
	++f32_arr0[1];
	ASSERT_FALSE( eq( f32_arr0, f32_arr1 ) );

	f32_arr0 = f32_arr1;
	++f32_arr0[2];
	ASSERT_FALSE( eq( f32_arr0, f32_arr1 ) );

	f32_arr0 = f32_arr1;
	++f32_arr0[3];
	ASSERT_FALSE( eq( f32_arr0, f32_arr1 ) );

	f32_arr0 = f32_arr1;
	++f32_arr0[4];
	ASSERT_FALSE( eq( f32_arr0, f32_arr1 ) );
};

TEST( TestArrayUtils, equivalenceF64 ) {
	array<double, 5> f64_arr0 {}, f64_arr1 {};
	ASSERT_TRUE( eq( f64_arr0, f64_arr1 ) );

	f64_arr0 = f64_arr1;
	++f64_arr0[0];
	ASSERT_FALSE( eq( f64_arr0, f64_arr1 ) );

	f64_arr0 = f64_arr1;
	++f64_arr0[1];
	ASSERT_FALSE( eq( f64_arr0, f64_arr1 ) );

	f64_arr0 = f64_arr1;
	++f64_arr0[2];
	ASSERT_FALSE( eq( f64_arr0, f64_arr1 ) );

	f64_arr0 = f64_arr1;
	++f64_arr0[3];
	ASSERT_FALSE( eq( f64_arr0, f64_arr1 ) );

	f64_arr0 = f64_arr1;
	++f64_arr0[4];
	ASSERT_FALSE( eq( f64_arr0, f64_arr1 ) );
};

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
