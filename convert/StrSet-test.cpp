#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "StrSet.hpp"

using namespace leon_utl;
using std::set;
using std::string;

TEST( TestStrSets, splitStr2set ) {
	string s = "abc,def,ghi";
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

TEST( TestStrSets, set2str ) {
	StrSet_t tokens {};
	ASSERT_EQ( set2str( tokens ), "{}" );

	tokens.insert( "abc" );
	tokens.insert( "def" );
	tokens.insert( "ghi" );
	ASSERT_EQ( set2str( tokens ), "{abc,def,ghi,}" );

	tokens.erase( "ghi" );
	ASSERT_EQ( set2str( tokens ), "{abc,def,}" );

	tokens.clear();
	ASSERT_EQ( set2str( tokens ), "{}" );
};

TEST( TestStrSets, outputStrSet ) {
	StrSet_t tokens {};
	ASSERT_EQ( set2str( tokens ), "{}" );

	tokens.insert( "abc" );
	tokens.insert( "def" );
	tokens.insert( "ghi" );
	ASSERT_EQ( set2str( tokens ), "{abc,def,ghi,}" );

	tokens.erase( "ghi" );
	ASSERT_EQ( set2str( tokens ), "{abc,def,}" );

	tokens.clear();
	ASSERT_EQ( set2str( tokens ), "{}" );
};

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
