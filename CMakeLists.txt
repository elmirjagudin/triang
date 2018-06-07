cmake_minimum_required(VERSION 2.8)
project(triang)

add_definitions(-std=c++11)
find_package( OpenCV REQUIRED )
find_package( Eigen3 REQUIRED )
#find_package( PCL 1.6 REQUIRED )
#list(REMOVE_ITEM PCL_LIBRARIES "vtkproj4")
include_directories(${EIGEN3_INCLUDE_DIR})
#link_directories(${PCL_LIBRARY_DIRS})
#add_definitions(${PCL_DEFINITIONS})

add_executable(triang main.cpp)
target_link_libraries(triang ${OpenCV_LIBS})

