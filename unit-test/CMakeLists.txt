cmake_minimum_required( VERSION 3.16.0 )

find_package( GTest REQUIRED )

#======= TARGET unit-test ==============
add_executable( unit-test UnitTestMain.cpp
	# ../algor/Algorithms-test.cpp
	# ../algor/Statistics-test.cpp
	# ../chrono/Chrono-test.cpp
	# ../convert/Converts-test.cpp
	# ../convert/StrSet-test.cpp
	# ../convert/Unicodes-test.cpp
	# ../convert/UnionTypes-test.cpp
	# ../except/CallStack-test.cpp
	# ../queue/CraflinRQ-test.cpp
	# ../queue/PosixMQ-test.cpp
	# ../queue/SHMRQ-test.cpp
	../sys/Signals-test.cpp
)
target_include_directories( unit-test PRIVATE ${PROJECT_SOURCE_DIR} )
target_link_libraries( unit-test
	target_static

	GTest::gtest GTest::gtest_main
	GTest::gmock GTest::gmock_main
)
install( TARGETS unit-test RUNTIME DESTINATION testing )

# kate: indent-mode cmake; indent-width 4; replace-tabs off; tab-width 4;
