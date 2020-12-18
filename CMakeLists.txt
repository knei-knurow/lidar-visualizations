cmake_minimum_required(VERSION 3.10)

project(lidar-vis)
set(CMAKE_CXX_STANDARD 14)

set(LIDAR_VIS_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lidar/")
set(3RDPARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/")

set(RPLIDAR_SDK_SRC_DIR "${3RDPARTY_DIR}/rplidar_sdk/sdk")
if(WIN32)
	# TODO
elseif(APPLE)
	# TODO
elseif(UNIX)
	# TODO
else()
	message(FATAL_ERROR "Unsupported platform.")
endif()

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

set(USING_SFML ON CACHE BOOL "Compilation with SFML.")
set(USING_RPLIDAR ON CACHE BOOL "Compilation with RPLIDAR SDK.")

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

# Add Lidar-Vis executable
add_executable(lidar-vis ${LIDAR_VIS_SOURCE_FILES})

# Compile SFML
if(USING_SFML)
	# add_subdirectory("${3RDPARTY_DIR}/SFML")
endif()

# Compile RPLIDAR SDK
if(USING_RPLIDAR)
	add_compile_definitions(USING_RPLIDAR)
	if(UNIX)
		add_library(rplidar_include "${RPLIDAR_SDK_SRC_DIR}/sdk/include/rplidar.h")
		target_include_directories(lidar-vis PRIVATE "${RPLIDAR_SDK_SRC_DIR}/sdk/include")
		
		
		add_custom_target(
			rplidar_lib
			COMMAND make
			WORKING_DIRECTORY ${RPLIDAR_SDK_SRC_DIR}
		)
		target_link_libraries(lidar-vis "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/rplidar_sdk/sdk/output/Linux/Release/librplidar_sdk.a")		
		add_dependencies(lidar-vis rplidar_lib)
	endif()
endif()
