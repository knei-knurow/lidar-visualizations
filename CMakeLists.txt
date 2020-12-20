cmake_minimum_required(VERSION 3.10)
project(lidar-vis)
set(CMAKE_CXX_STANDARD 14)

# Variables which can be modified by the user
set(USING_SFML ON CACHE BOOL "Compilation with SFML.")
set(USING_RPLIDAR OFF CACHE BOOL "Compilation with RPLIDAR SDK.")

# Project paths
set(LIDAR_VIS_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lidar/")
set(3RDPARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/")

# RPLIDAR SDK paths
set(RPLIDAR_SDK_DIR "${3RDPARTY_DIR}/rplidar_sdk/sdk")
set(RPLIDAR_SDK_SRC_DIR "${RPLIDAR_SDK_DIR}/sdk/src")
set(RPLIDAR_SDK_INCLUDE_DIR "${RPLIDAR_SDK_DIR}/sdk/include")


# Build RPLIDAR SDK from source using its own delivered Makefile
if(BUILD_RPLIDAR_SDK)
	add_custom_target(
		extern_lib
		COMMAND make #DEBUG=1
		WORKING_DIRECTORY ${RPLIDAR_SDK_DIR}
	)
endif()

# Build SFML from source
if(BUILD_SFML)
	add_subdirectory("${3RDPARTY_DIR}/SFML")
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


set(LIDAR_VIS_SRC_FILES
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
add_executable(lidar-vis ${LIDAR_VIS_SRC_FILES})

if(USING_RPLIDAR)
	set(THREADS_PREFER_FLAG ON)
	find_package(Threads REQUIRED)
	
	target_include_directories(lidar-vis PRIVATE 
		${RPLIDAR_SDK_INCLUDE_DIR}
		${RPLIDAR_SDK_SRC_DIR}
	)

	target_link_libraries(lidar-vis PRIVATE
		"${RPLIDAR_SDK_DIR}/output/Linux/Release/librplidar_sdk.a"
		Threads::Threads
		rt
	)

	add_compile_definitions(USING_RPLIDAR)
endif()

if(USING_SFML)
	target_include_directories(lidar-vis PRIVATE 
		${CMAKE_CURRENT_SOURCE_DIR}/sfml/include
	)

	target_link_libraries(lidar-vis PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/sfml/lib/libsfml-system.so
		${CMAKE_CURRENT_SOURCE_DIR}/sfml/lib/libsfml-window.so
		${CMAKE_CURRENT_SOURCE_DIR}/sfml/lib/libsfml-graphics.so
	)


	# include(FindPkgConfig)
	#pkg_search_module(SFML REQUIRED sfml-all)
	#target_include_directories(lidar-vis PRIVATE 
	#	${SFML_INCLUDEDIR}/SFML
	#)
	#target_link_libraries(lidar-vis PRIVATE 
	#	${SFML_LIBDIR}/libsfml-system
	#	${SFML_LIBDIR}/libsfml-window
	#	${SFML_LIBDIR}/libsfml-graphics
	#)
	#message(">>>${SFML_INCLUDEDIR}")
	
	#if(SFML_ALL
	#target_link_libraries(lidar-vis PRIVATE sfml-all)


	#find_package(sfml-window REQUIRED)

	#add_library(libsfml-graphics.so SHARED IMPORTED)
	# target_include_directories(lidar-vis PRIVATE 
	#	/usr/include
	#)
	#target_link_libraries(lidar-vis
	#	-L/usr/lib/x86_64-linux-gnu/libsfml-system.so
	#	-L/usr/lib/x86_64-linux-gnu/libsfml-window.so
	#	-L/usr/lib/x86_64-linux-gnu/libsfml-graphics.so
	#)
	add_compile_definitions(USING_SFML)
endif()
