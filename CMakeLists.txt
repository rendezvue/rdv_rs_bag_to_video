project(rdv_rs_bag_to_video)

cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

#realsense
find_library(REALSENSE2_FOUND realsense2 HINTS ${LIBRARY_DIR} REQUIRED)
if(NOT REALSENSE2_FOUND)
    SET(REALSENSE2_FOUND "realsense2")
    message(WARN "Failed to find_library(realsense2)")
endif()

#opencv
find_package(OpenCV REQUIRED)
include_directories( ${OPENCV_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} )

#boost
find_package(Boost REQUIRED COMPONENTS system thread filesystem regex date_time program_options)
#find_package(Boost REQUIRED)
include_directories( ${Boost_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR} )
include_directories( ${Boost_LIBRARY_DIRS} ${CMAKE_CURRENT_SOURCE_DIR} ) 

# src
set(SRC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src)
FILE(GLOB SRC_SRC "${SRC_PATH}/*.cpp" ) 
include_directories(${SRC_PATH})

add_executable( rdv_rs_bag_to_video ${SRC_SRC})

target_link_libraries(rdv_rs_bag_to_video ${OpenCV_LIBS} ${Boost_LIBRARIES} ${REALSENSE2_FOUND})
