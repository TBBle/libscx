cmake_minimum_required(VERSION 3.0.2)

project(scx)
enable_testing()

# QT 5
#set(CMAKE_INCLUDE_CURRENT_DIR ON)
#set(CMAKE_AUTOMOC ON)
#find_package(Qt5Core)

# Boost
find_package(Boost REQUIRED COMPONENTS locale)
include_directories(${Boost_INCLUDE_DIRS})

# Our own stuff
include_directories("${PROJECT_SOURCE_DIR}/src")

add_library(scx
	src/scx.hpp
	src/SCXFile.cpp
	src/SCXFile.hpp
	src/AssetName.hpp
	src/AssetName.cpp
	src/Scene.hpp
	src/Table1Data.hpp
	src/Table1Data.cpp
	src/Variable.hpp
	src/Variable.cpp
)

target_link_libraries(scx ${Boost_LIBRARIES})

# CMake 3.1 onwards
#set_property(TARGET scx PROPERTY CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON)
# CMake 3.0 or earlier
add_definitions(-std=c++11)

add_executable(test_scx
	unit_test/catch.hpp

	unit_test/test_SCXFile.cpp
)

target_link_libraries(test_scx scx)

add_test(test_scx test_scx)
