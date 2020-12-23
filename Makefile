CXX := g++
CPPFLAGS := -std=c++17
LIBS :=

SFML := ${CURDIR}/sfml
RPLIDAR := ${CURDIR}/rplidar_sdk

OS=$(shell uname -s)

SFML_INCLUDES :=
RPLIDAR_INCLUDES :=

ifeq ($(USE_SFML),true)
$(info "compiling with sfml")
CPPFLAGS += -DUSING_SFML
LIBS += -lsfml-graphics -lsfml-window -lsfml-system
SFML_INCLUDES += -I$(SFML)/include
SFML_LIBS := -L$(SFML)/lib
endif

ifeq ($(USE_RPLIDAR),true)
$(info "compiling with rplidar_sdk")
CPPFLAGS += -DUSING_RPLIDAR
LIBS += -lrplidar_sdk -pthread
RPLIDAR_INCLUDES += -I$(RPLIDAR)/sdk/sdk/include -I$(RPLIDAR)/sdk/sdk/src
RPLIDAR_LIBS := -L$(RPLIDAR)/sdk/output/$(OS)/Release
endif

lidarvis: main.o app.o cloud.o cloud-grabbers.o cloud-writers.o guis.o scenarios.o
	$(CXX) --output lidarvis \
	main.o \
	app.o \
	cloud.o \
	cloud-grabbers.o \
	cloud-writers.o \
	guis.o \
	scenarios.o \
	$(SFML_LIBS) \
	$(RPLIDAR_LIBS) \
	$(LIBS)

main.o: src/main.cpp
	$(CXX) $(CPPFLAGS) -c src/main.cpp $(SFML_INCLUDES) $(RPLIDAR_INCLUDES)

app.o: src/app.cpp
	$(CXX) $(CPPFLAGS) -c src/app.cpp $(SFML_INCLUDES) $(RPLIDAR_INCLUDES)

cloud.o: src/cloud.cpp
	$(CXX) $(CPPFLAGS) -c src/cloud.cpp

cloud-grabbers.o: src/cloud-grabbers.cpp
	$(CXX) $(CPPFLAGS) -c src/cloud-grabbers.cpp $(RPLIDAR_INCLUDES)

cloud-writers.o: src/cloud-writers.cpp
	$(CXX) $(CPPFLAGS) -c src/cloud-writers.cpp 

guis.o: src/guis.cpp
	$(CXX) $(CPPFLAGS) -c src/guis.cpp $(SFML_INCLUDES)

scenarios.o: src/scenarios.cpp
	$(CXX) $(CPPFLAGS) -c src/scenarios.cpp

clean:
	rm -f *.o src/*.o lidarvis
