cmake_minimum_required( VERSION 3.16.0 )

set( PROJECT_VERSION_MAJOR 0 )   # 主版本
set( PROJECT_VERSION_MINOR 0 )   # 次版本
set( PROJECT_VERSION_PATCH 0 )   # 修订号(Bug fix)

# 版本全串
set( PROJECT_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})

# 版本说明
set( PROJECT_VERSION_NOTES "初创" )

# 项目描述
set( PROJECT_DESCRIPTION   "leon's c++ utils" )

project( LeonUtils VERSION ${PROJECT_VERSION} DESCRIPTION ${PROJECT_DESCRIPTION} LANGUAGES CXX )
configure_file( Version.hpp.in Version.hpp )

set( CMAKE_CXX_STANDARD 20 )
set( CMAKE_POSITION_INDEPENDENT_CODE ON )
# 不知道下面这两句有何区别
set( CMAKE_INCLUDE_CURRENT_DIR ON )
include_directories( "." )
# 遵循GNU文件分布约定(如:库文件输出至/usr/lib之下)
include( GNUInstallDirs )

#======== obj libs =====================
add_library( objCommon OBJECT
	algor/Algorithms.cpp
	algor/Statistics.cpp
	chrono/Chrono.cpp
	convert/Converts.cpp
	convert/StrSet.cpp
	convert/Unicodes.cpp
)

######## 主要程序 ###############################################################
#======== 静态版 ========================
add_library( target_static STATIC )
set_target_properties( target_static PROPERTIES
	OUTPUT_NAME_RELEASE  LeonUtils
	OUTPUT_NAME_DEBUG    LeonUtils
#	PUBLIC_HEADER        LeonUtils.hpp
	VERSION              ${PROJECT_VERSION}
	SUFFIX               ".a.${PROJECT_VERSION}"
#	ARCHIVE_OUTPUT_NAME_RELEASE  "ARCHIVE_OUTPUT_NAME_.${PROJECT_VERSION}"
#	LIBRARY_OUTPUT_NAME_RELEASE  "LIBRARY_OUTPUT_NAME_.${PROJECT_VERSION}"
)
target_link_libraries( target_static objCommon )
install( TARGETS target_static
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

#======== 动态版 =====================
add_library( target_dynmic SHARED )
set_target_properties( target_dynmic PROPERTIES
	OUTPUT_NAME_RELEASE    LeonUtils
	OUTPUT_NAME_DEBUG      LeonUtils
#	PUBLIC_HEADER          ThreadName.hpp
	VERSION                ${PROJECT_VERSION}
	SOVERSION              ${PROJECT_VERSION_MAJOR}
)
target_link_libraries( target_dynmic objCommon )
install( TARGETS target_dynmic
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

######## 单元测试 ###############################################################
add_subdirectory( unit-test )

# kate: indent-mode cmake; indent-width 4; replace-tabs off; tab-width 4;
