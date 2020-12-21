CXX=g++
CPPFLAGS=-std=c++17 -DUSING_SFML -DUSING_RPLIDAR
LIBS=-lsfml-graphics -lsfml-window -lsfml-system -lrplidar_sdk

SFML=${CURDIR}/sfml
RPLIDAR=${CURDIR}/rplidar_sdk

lidarvis: main app cloud cloud-grabbers cloud-writers guis scenarios
	$(CXX) --output lidarvis \
	main.o \
	app.o \
	cloud.o \
	cloud-grabbers.o \
	cloud-writers.o \
	guis.o \
	scenarios.o \
	-L$(SFML)/lib \
	-L$(RPLIDAR)/sdk/output/Darwin/Release \
	$(LIBS)

main: src/main.cpp
	$(CXX) $(CPPFLAGS) -c src/main.cpp -I$(SFML)/include -I$(RPLIDAR)/sdk/sdk/include -I$(RPLIDAR)/sdk/sdk/src/hal

app: src/app.cpp
	$(CXX) $(CPPFLAGS) -c src/app.cpp -I$(SFML)/include -I$(RPLIDAR)/sdk/sdk/include -I$(RPLIDAR)/sdk/sdk/src/hal

cloud: src/cloud.cpp
	$(CXX) $(CPPFLAGS) -c src/cloud.cpp

cloud-grabbers: src/cloud-grabbers.cpp
	$(CXX) $(CPPFLAGS) -c src/cloud-grabbers.cpp -I$(RPLIDAR)/sdk/sdk/include -I$(RPLIDAR)/sdk/sdk/src/hal

cloud-writers: src/cloud-writers.cpp
	$(CXX) $(CPPFLAGS) -c src/cloud-writers.cpp 

guis: src/guis.cpp
	$(CXX) $(CPPFLAGS) -c src/guis.cpp -I$(SFML)/include

scenarios: src/scenarios.cpp
	$(CXX) $(CPPFLAGS) -c src/scenarios.cpp

clean:
	rm -f *.o src/*.o lidarvis
