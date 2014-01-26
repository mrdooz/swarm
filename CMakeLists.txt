cmake_minimum_required (VERSION 2.6)

project (swarm)

set(COMMON_FLAGS "-Wno-switch-enum")

include(FindProtobuf)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})
find_package(SFML 2 REQUIRED system window graphics network audio)
find_package(Boost REQUIRED)
find_package(Protobuf REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${SFML_INCLUDE_DIRS})
include_directories(${PROTOBUF_INCLUDE_DIR})

file(GLOB SRC "*.cpp" "*.hpp" "protocol/*.pb.cc")

# change c++ standard library to libc++ (llvm)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -v -std=c++11 -stdlib=libc++")
add_executable(swarm ${SRC})

if (APPLE)
	find_library(APP_SERVICES ApplicationServices)
    set_target_properties(
        ${PROJECT_NAME} 
        PROPERTIES
        XCODE_ATTRIBUTE_GCC_PREFIX_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/precompiled.hpp"
        XCODE_ATTRIBUTE_GCC_PRECOMPILE_PREFIX_HEADER "YES"
        LINK_FLAGS "-F/Library/Frameworks"
    )
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")

    # specifically link against a protobuf and boost build with libc++
    target_link_libraries(swarm ${SFML_LIBRARIES} "/opt/local/boost/lib/libboost_date_time.a" "/opt/local/protobuf/lib/libprotobuf.a" ${APP_SERVICES} )
else()
	target_link_libraries(swarm ${SFML_LIBRARIES} ${Boost_DATETIME_LIBRARY} ${PROTOBUF_LIBRARY} ${APP_SERVICES} )
endif()

