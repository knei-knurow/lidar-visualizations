CXX=g++
CPPFLAGS=-std=c++17 -DUSING_SFML
LIBS=-lsfml-graphics -lsfml-window -lsfml-system

SFML=/usr/local/Cellar/sfml/2.5.1

lidar: lidar/app.o lidar/cloud-grabbers.o lidar/cloud-writers.o lidar/cloud.o lidar/guis.o lidar/main.o lidar/scenarios.o sfml
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

sfml:
	$(CXX) -I$(SFML)/include

app.o: lidar/app.cpp
	$(CXX) -c lidar/app.cpp

cloud-grabbers.o: lidar/cloud-grabbers.cpp
	$(CXX) -c lidar/cloud-grabbers.cpp

cloud-writers.o: lidar/cloud-writers.cpp
	$(CXX) -c lidar/cloud-writers.cpp

cloud.o: lidar/cloud.cpp
	$(CXX) -c lidar/cloud.cpp

guis.o: lidar/guis.cpp
	$(CXX) -c lidar/guis.cpp

main.o: lidar/main.cpp
	$(CXX) -c lidar/main.cpp

scenarios.o: lidar/scenarios.cpp
	$(CXX) -c lidar/scenarios.cpp

clean:
	rm -f lidar/*.o vil
