cmake_minimum_required(VERSION 3.4.1 FATAL_ERROR)

project(scx CXX)
enable_testing()

# Globally C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Unicode for MSVC
if(MSVC)
	add_definitions(-DUNICODE -D_UNICODE)
endif()

# QT 5
#set(CMAKE_INCLUDE_CURRENT_DIR ON)
#set(CMAKE_AUTOMOC ON)
#find_package(Qt5Core)

# Boost
if(MSVC)
	# Boost headers use #pragma link for MSVC, giving the
	# static library names. So just go with it...
	set(Boost_USE_STATIC_LIBS ON)
endif()
find_package(Boost REQUIRED COMPONENTS date_time locale)
#add_definitions("${Boost_LIB_DIAGNOSTIC_DEFINITIONS}")

# Our own stuff
include_directories("${PROJECT_SOURCE_DIR}/src")

add_library(scx
	src/scx.hpp
	src/SCXFile.cpp
	src/SCXFile.hpp
	src/AssetName.hpp
	src/AssetName.cpp
	src/Scene.hpp
	src/Scene.cpp
	src/Table1Data.hpp
	src/Table1Data.cpp
	src/Variable.hpp
	src/Variable.cpp

	# Utility templates
	src/buffutils.hpp
)

target_include_directories(scx PUBLIC ${Boost_INCLUDE_DIRS})
target_link_libraries(scx PUBLIC ${Boost_LIBRARIES})

target_include_directories(scx PUBLIC gsl)

# This didn't work...
# http://stackoverflow.com/a/20165220 for reference
#set_property(TARGET scx
#	PROPERTY CXX_STANDARD 14
#	PROPERTY CXX_STANDARD_REQUIRED ON)

add_executable(test_scx
	unit_test/test_SCXFile.cpp
)

target_include_directories(test_scx PRIVATE catch)

target_link_libraries(test_scx scx)

add_test(test_scx test_scx)
