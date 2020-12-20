CXX=g++
CPPFLAGS=-std=c++17 -DUSING_SFML
LIBS=-lsfml-graphics -lsfml-window -lsfml-system

SFML=${CURDIR}/sfml

lol:
	@echo "lol"
	ls $(SFML)/include/SFML

lidar: lidar/app.o lidar/cloud-grabbers.o lidar/cloud-writers.o lidar/cloud.o lidar/guis.o lidar/main.o lidar/scenarios.o
	$(CXX) --output vil \
	lidar/app.o \
	lidar/cloud-grabbers.o \
	lidar/cloud-writers.o \
	lidar/cloud.o \
	lidar/guis.o \
	lidar/main.o \
	lidar/scenarios.o \
	-L$(SFML)/lib \
	$(LIBS)

app.o: lidar/app.cpp
	$(CXX) $(CPPFLAGS) -c lidar/app.cpp -I$(SFML)/include

cloud-grabbers.o: lidar/cloud-grabbers.cpp
	$(CXX) $(CPPFLAGS) -c lidar/cloud-grabbers.cpp -I$(SFML)/include

cloud-writers.o: lidar/cloud-writers.cpp
	$(CXX) $(CPPFLAGS) -c lidar/cloud-writers.cpp -I$(SFML)/include

cloud.o: lidar/cloud.cpp
	$(CXX) $(CPPFLAGS) -c lidar/cloud.cpp -I$(SFML)/include

guis.o: lidar/guis.cpp
	$(CXX) $(CPPFLAGS) -c lidar/guis.cpp -I$(SFML)/include

main.o: lidar/main.cpp
	$(CXX) $(CPPFLAGS) -c lidar/main.cpp -I$(SFML)/include

scenarios.o: lidar/scenarios.cpp
	$(CXX) $(CPPFLAGS) -c lidar/scenarios.cpp -I$(SFML)/include

clean:
	rm -f *.o lidar/*.o vil
