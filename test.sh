g++ -Wall \
	-I3rdparty/rplidar_sdk/sdk/sdk/include \
	-I3rdparty/rplidar_sdk/sdk/sdk/src \
	test.cpp \
	-L3rdparty/rplidar_sdk/sdk/output/Linux/Release \
	-pthread \
	-lrplidar_sdk \
	-lsfml-system -lsfml-window -lsfml-graphics \
	-o test