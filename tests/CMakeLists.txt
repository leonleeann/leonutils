cmake_minimum_required( VERSION 3.16.0 )

find_package( GTest REQUIRED )
add_library( leonlog_dynmic SHARED IMPORTED )
set_property( TARGET leonlog_dynmic PROPERTY
	IMPORTED_LOCATION "/usr/local/lib/libLeonLog.so.1.8.6" )

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
	TextSheet-test.cpp
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
target_link_libraries( test-craflinrq
	leonutl_static
	leonlog_dynmic
	Threads::Threads
)
install( TARGETS test-craflinrq RUNTIME DESTINATION testing )

#======= TARGET test-unlink-shm ================================================
# 此两个 app 专门测试 shm 是否能够提前删除底层文件, 而共享内存依旧保持可用于 IPC
add_executable( unlink-consumer unlink-consumer.cpp )
add_executable( unlink-producer unlink-producer.cpp )
target_link_libraries( unlink-consumer leonutl_static )
target_link_libraries( unlink-producer leonutl_static )
install( TARGETS unlink-consumer RUNTIME DESTINATION testing )
install( TARGETS unlink-producer RUNTIME DESTINATION testing )

# kate: indent-mode cmake; indent-width 4; replace-tabs off; tab-width 4;
