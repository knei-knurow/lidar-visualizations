CXX=g++
CXXFLAGS=-Wall -std=c++17
LIBS=-lsfml-graphics -lsfml-window -lsfml-system

lidar: lidar/app.o lidar/cloud-grabbers.o lidar/cloud-writers.o lidar/cloud.o lidar/guis.o lidar/main.o lidar/scenarios.o
		$(CXX) -o vil \
		lidar/app.o \
		lidar/cloud-grabbers.o \
		lidar/cloud-writers.o \
		lidar/cloud.o \
		lidar/guis.o \
		lidar/main.o \
		lidar/scenarios.o \
		-L${CURDIR}/SFML/lib $(LIBS)
		# -L${CURDIR}/rplidar_sdk/sdk/output/Darwin/Release -lrplidar_sdk	\

app.o: lidar/app.cpp
	$(CXX) $(CXXFLAGS) -c lidar/app.cpp -I${CURDIR}/SFML/include

cloud-grabbers.o: lidar/cloud-grabbers.cpp
	$(CXX) $(CXXFLAGS) -c lidar/cloud-grabbers.cpp

cloud-writers.o: lidar/cloud-writers.cpp
	$(CXX) $(CXXFLAGS) -c lidar/cloud-writers.cpp

cloud.o: lidar/cloud.cpp
	$(CXX) $(CXXFLAGS) -c lidar/cloud.cpp

guis.o: lidar/guis.cpp
	$(CXX) $(CXXFLAGS) -c lidar/guis.cpp

main.o: lidar/main.cpp
	$(CXX) $(CXXFLAGS) -c lidar/main.cpp

scenarios.o: lidar/scenarios.cpp
	$(CXX) $(CXXFLAGS) -c lidar/scenarios.cpp

clean:
	rm lidar/*.o
