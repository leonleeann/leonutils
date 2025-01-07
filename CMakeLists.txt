cmake_minimum_required( VERSION 3.25.0 )

######## 项目及版本定义 ##########################################################
set( PROJECT_VERSION_MAJOR 0 )   # 主版本
set( PROJECT_VERSION_MINOR 2 )   # 次版本
set( PROJECT_VERSION_PATCH 8 )   # 修订号(Bug fix)

# 版本全串
set( PROJECT_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})

# 版本说明
set( PROJECT_VERSION_NOTES "集合操作不用运算符，避免多重overloading，运算符只能在app内" )

# 项目描述
set( PROJECT_DESCRIPTION   "leon's c++ utils" )

project( LeonUtils
	VERSION		${PROJECT_VERSION}
	DESCRIPTION	${PROJECT_DESCRIPTION}
	LANGUAGES CXX
)
configure_file( Version.hpp.in Version.hpp )

######## 全局配置 ###############################################################
# 采用C++标准:20
set( CMAKE_CXX_STANDARD 20 )

# 遵循GNU文件分布约定(如:库文件输出至/usr/lib之下)
include( GNUInstallDirs )

include( CheckCXXCompilerFlag )
check_cxx_compiler_flag( -Wall HAVE_WALL )
if ( HAVE_WALL )
	# gcc 警告全开
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall" )
endif()

set( CMAKE_POSITION_INDEPENDENT_CODE ON )
set( THREADS_PREFER_PTHREAD_FLAG ON )
find_package( Threads REQUIRED )
# find_package( fmt REQUIRED )

# 对任何头文件的搜索都可 以本目录为根开始
set( CMAKE_INCLUDE_CURRENT_DIR ON )
# 不知道下面这句与上面一句有何区别
include_directories( "." )	# 这不是本文件的目录！是包含者文件的目录！！！
include_directories( "include" )

######## 主要模块 ###############################################################
add_library( objCommon OBJECT
	src/Algorithms.cpp
	src/CallStack.cpp
	src/ChildProc.cpp
	src/Chrono.cpp
	src/Converts.cpp
	src/FileSystem.cpp
	src/SetOps.cpp
	src/ShmBuffer.cpp
	src/Signals.cpp
	src/Statistics.cpp
	src/TextSheet.cpp
	src/Unicodes.cpp
)

######## 主要产出 ###############################################################
#======== 静态版 ========================
add_library( leonutl_static STATIC )
set_target_properties( leonutl_static PROPERTIES
	OUTPUT_NAME_RELEASE		LeonUtils
	OUTPUT_NAME_DEBUG		LeonUtils
	VERSION					${PROJECT_VERSION}
	SUFFIX					".a.${PROJECT_VERSION}"
#	ARCHIVE_OUTPUT_NAME_RELEASE  "ARCHIVE_OUTPUT_NAME_.${PROJECT_VERSION}"
#	LIBRARY_OUTPUT_NAME_RELEASE  "LIBRARY_OUTPUT_NAME_.${PROJECT_VERSION}"
)
target_sources( leonutl_static PUBLIC FILE_SET HEADERS BASE_DIRS "include"
FILES
	include/leonutils/Algorithms.hpp
	include/leonutils/CallStack.hpp
	include/leonutils/ChildProc.hpp
	include/leonutils/Chrono.hpp
	include/leonutils/ChronoTypes.hpp
	include/leonutils/Converts.hpp
	include/leonutils/CraflinRQ.hpp
	include/leonutils/CraflinRQ.tpp
	include/leonutils/Exceptions.hpp
	include/leonutils/FileSystem.hpp
	include/leonutils/MemoryOrder.hpp
	include/leonutils/RingQueue.hpp
	include/leonutils/SetOps.hpp
	include/leonutils/ShmAtomicRQ.hpp
	include/leonutils/ShmBuffer.hpp
	include/leonutils/Signals.hpp
	include/leonutils/Statistics.hpp
	include/leonutils/TextSheet.hpp
	include/leonutils/Unicodes.hpp
	include/leonutils/UnionTypes.hpp
)

target_link_libraries( leonutl_static objCommon )
install( TARGETS leonutl_static
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
	FILE_SET HEADERS
)

#======== 动态版 =====================
add_library( leonutl_dynmic SHARED )
set_target_properties( leonutl_dynmic PROPERTIES
	OUTPUT_NAME_RELEASE		LeonUtils
	OUTPUT_NAME_DEBUG		LeonUtils
	VERSION					${PROJECT_VERSION}
	SOVERSION				${PROJECT_VERSION_MAJOR}
)
target_link_libraries( leonutl_dynmic objCommon )
install( TARGETS leonutl_dynmic
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

######## 单元测试 ###############################################################
add_subdirectory( tests )

# kate: indent-mode cmake; indent-width 4; replace-tabs off; tab-width 4;
