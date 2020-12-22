CXX=g++
CPPFLAGS=-std=c++17 -DUSING_SFML -DUSING_RPLIDAR
LIBS=-lsfml-graphics -lsfml-window -lsfml-system -lrplidar_sdk -pthread

SFML=${CURDIR}/sfml
RPLIDAR=${CURDIR}/rplidar_sdk

OS=$(shell uname -s)

lidarvis: main.o app.o cloud.o cloud-grabbers.o cloud-writers.o guis.o scenarios.o
	$(CXX) --output lidarvis \
	main.o \
	app.o \
	cloud.o \
	cloud-grabbers.o \
	cloud-writers.o \
	guis.o \
	scenarios.o \
	-L$(SFML)/lib \
	-L$(RPLIDAR)/sdk/output/$(OS)/Release \
	$(LIBS)

main.o: src/main.cpp
	$(CXX) $(CPPFLAGS) -c src/main.cpp -I$(SFML)/include -I$(RPLIDAR)/sdk/sdk/include -I$(RPLIDAR)/sdk/sdk/src

app.o: src/app.cpp
	$(CXX) $(CPPFLAGS) -c src/app.cpp -I$(SFML)/include -I$(RPLIDAR)/sdk/sdk/include -I$(RPLIDAR)/sdk/sdk/src

cloud.o: src/cloud.cpp
	$(CXX) $(CPPFLAGS) -c src/cloud.cpp

cloud-grabbers.o: src/cloud-grabbers.cpp
	$(CXX) $(CPPFLAGS) -c src/cloud-grabbers.cpp -I$(RPLIDAR)/sdk/sdk/include -I$(RPLIDAR)/sdk/sdk/src

cloud-writers.o: src/cloud-writers.cpp
	$(CXX) $(CPPFLAGS) -c src/cloud-writers.cpp 

guis.o: src/guis.cpp
	$(CXX) $(CPPFLAGS) -c src/guis.cpp -I$(SFML)/include

scenarios.o: src/scenarios.cpp
	$(CXX) $(CPPFLAGS) -c src/scenarios.cpp

clean:
	rm -f *.o src/*.o lidarvis
