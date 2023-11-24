cmake_minimum_required( VERSION 3.16.0 )

#[[
get_property( my_includes DIRECTORY PROPERTY INCLUDE_DIRECTORIES )
message( STATUS "头文件搜索目录:'${my_includes}'========" )
foreach( dir ${my_includes} )
	message( STATUS "dir='${dir}'" )
endforeach()
message( STATUS "头文件搜索目录:'结束'========" )
]]

#[[======= obj libs ======================
add_library( objUniteTest		OBJECT
	${OCTOPUS_CODE_BASE}/common/log/Log2Console.cpp
	# ${OCTOPUS_CODE_BASE}/common/use-case/UseCase-Jobs.cpp
	# ${OCTOPUS_CODE_BASE}/common/use-case/UseCase-Quotes.cpp
)]]

find_package( GTest REQUIRED )

#======= TARGET unit-test ==============
add_executable( unit-test UnitTestMain.cpp
	../algor/Algorithms-test.cpp
	../algor/Statistics-test.cpp
	../chrono/Chrono-test.cpp
	../convert/Converts-test.cpp
	../convert/StrSet-test.cpp
	../convert/Unicodes-test.cpp
	../convert/UnionTypes-test.cpp
	../queue/SHMRQ-test.cpp
)
target_include_directories( unit-test PRIVATE ${PROJECT_SOURCE_DIR} )
target_link_libraries( unit-test
	objCommon
	# objUniteTest

	GTest::gtest GTest::gtest_main
	GTest::gmock GTest::gmock_main
)
install( TARGETS unit-test RUNTIME DESTINATION testing )

# kate: indent-mode cmake; indent-width 4; replace-tabs off; tab-width 4;
