cmake_minimum_required(VERSION 3.10)

project(lidar-vis)
set(CMAKE_CXX_STANDARD 14)

# RPLIDAR SDK target
# add_custom_target(
#    extern_lib
#    COMMAND make
#    WORKING_DIRECTORY "rplidar_sdk/sdk/"
# )


# add_executable(myexecutable main.cpp)
# target_link_libraries(myexecutable "${CMAKE_CURRENT_SOURCE_DIR}/rplidar_sdk/sdk/output/Linux/Release/librplidar_sdk.a")
#add_dependencies(
#	myexecutable 
#	extern_lib
#)

set(LIDAR_VIS_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lidar/")
set(LIDAR_VIS_SOURCE_FILES
	"${LIDAR_VIS_SRC_DIR}/cloud.h"
	"${LIDAR_VIS_SRC_DIR}/cloud.cpp"
	"${LIDAR_VIS_SRC_DIR}/cloud-grabbers.h"
	"${LIDAR_VIS_SRC_DIR}/cloud-grabbers.cpp"
	"${LIDAR_VIS_SRC_DIR}/cloud-writers.h"
	"${LIDAR_VIS_SRC_DIR}/cloud-writers.cpp"
	"${LIDAR_VIS_SRC_DIR}/guis.h"
	"${LIDAR_VIS_SRC_DIR}/guis.cpp"
	"${LIDAR_VIS_SRC_DIR}/scenarios.h"
	"${LIDAR_VIS_SRC_DIR}/scenarios.cpp"
	"${LIDAR_VIS_SRC_DIR}/app.h"
	"${LIDAR_VIS_SRC_DIR}/app.cpp"
	"${LIDAR_VIS_SRC_DIR}/main.cpp"
)
add_executable(lidar-vis ${LIDAR_VIS_SOURCE_FILES})

