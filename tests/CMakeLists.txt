cmake_minimum_required( VERSION 3.16.0 )

find_package( GTest REQUIRED )

#======= TARGET unit-test ==============
add_executable( unit-test UnitTestMain.cpp
	Algorithms-test.cpp
	CallStack-test.cpp
	Chrono-test.cpp
	Converts-test.cpp
	CraflinRQ-test.cpp
	FileSystem-test.cpp
	RingQue-test.cpp
	ShmAtomicRQ-test.cpp
	ShmBuffer-test.cpp
	Signals-test.cpp
	Statistics-test.cpp
	StrSet-test.cpp
	Unicodes-test.cpp
	UnionTypes-test.cpp
)
target_include_directories( unit-test PRIVATE ${PROJECT_SOURCE_DIR} )
target_link_libraries( unit-test
	leonutl_static

	GTest::gtest GTest::gtest_main
	GTest::gmock GTest::gmock_main
)
install( TARGETS unit-test RUNTIME DESTINATION testing )

#======= TARGET test-craflinrq =================================================
add_executable( test-craflinrq Craflin_RQ-testMain.cpp )
target_link_libraries( test-craflinrq leonutl_static Threads::Threads LeonLog )
install( TARGETS test-craflinrq RUNTIME DESTINATION testing )

#======= TARGET sharq-consumer =================================================
add_executable( sharq-consumer sharq-consumer.cpp )
target_link_libraries( sharq-consumer leonutl_static LeonLog )
install( TARGETS sharq-consumer RUNTIME DESTINATION testing )

#======= TARGET sharq-producer =================================================
add_executable( sharq-producer sharq-producer.cpp )
target_link_libraries( sharq-producer leonutl_static LeonLog )
install( TARGETS sharq-producer RUNTIME DESTINATION testing )

# kate: indent-mode cmake; indent-width 4; replace-tabs off; tab-width 4;
