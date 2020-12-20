g++ -Wall \
	-DUSING_SFML \
	-DUSING_RPLIDAR \
	-std=c++17 \
	-I3rdparty/rplidar_sdk/sdk/sdk/include \
	-I3rdparty/rplidar_sdk/sdk/sdk/src \
	-Isfml/include \
	lidar/cloud.cpp lidar/cloud-grabbers.cpp lidar/cloud-writers.cpp lidar/scenarios.cpp lidar/guis.cpp lidar/app.cpp lidar/main.cpp \
	-L3rdparty/rplidar_sdk/sdk/output/Linux/Release \
	-Lsfml/lib \
	-pthread \
	-lrplidar_sdk \
	-lsfml-system -lsfml-window -lsfml-graphics \
	-o asdfasdf