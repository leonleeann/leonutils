cmake_minimum_required( VERSION 3.16.0 )

find_package( GTest REQUIRED )

#======= TARGET unit-test ==============
add_executable( unit-test UnitTestMain.cpp
	../algor/Algorithms-test.cpp
	../algor/Statistics-test.cpp
	../buffer/ShmBuffer-test.cpp
	../chrono/Chrono-test.cpp
	../convert/Converts-test.cpp
	../convert/StrSet-test.cpp
	../convert/Unicodes-test.cpp
	../convert/UnionTypes-test.cpp
	../except/CallStack-test.cpp
	../queue/CraflinRQ-test.cpp
	../queue/RingQue-test.cpp
	../sys/FileSystem-test.cpp
	../sys/Signals-test.cpp
)
target_include_directories( unit-test PRIVATE ${PROJECT_SOURCE_DIR} )
target_link_libraries( unit-test
	leonutl_static

	GTest::gtest GTest::gtest_main
	GTest::gmock GTest::gmock_main
)
install( TARGETS unit-test RUNTIME DESTINATION testing )

#======= TARGET test-craflinrq =================================================
add_executable( test-craflinrq ../queue/Craflin_RQ-testMain.cpp )
target_link_libraries( test-craflinrq leonutl_static Threads::Threads LeonLog )
install( TARGETS test-craflinrq RUNTIME DESTINATION testing )

#======= TARGET sharq-consumer =================================================
add_executable( sharq-consumer ../queue/sharq-consumer.cpp )
target_link_libraries( sharq-consumer leonutl_static LeonLog )
install( TARGETS sharq-consumer RUNTIME DESTINATION testing )

#======= TARGET sharq-producer =================================================
add_executable( sharq-producer ../queue/sharq-producer.cpp )
target_link_libraries( sharq-producer leonutl_static LeonLog )
install( TARGETS sharq-producer RUNTIME DESTINATION testing )

# kate: indent-mode cmake; indent-width 4; replace-tabs off; tab-width 4;
