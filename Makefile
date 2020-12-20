CXX=g++
CPPFLAGS=-std=c++17 -DUSING_SFML
LIBS=-lsfml-graphics -lsfml-window -lsfml-system

SFML=${CURDIR}/sfml

lidar: main app cloud cloud-grabbers cloud-writers guis scenarios
	$(CXX) --output vil \
	main.o \
	app.o \
	cloud.o \
	cloud-grabbers.o \
	cloud-writers.o \
	guis.o \
	scenarios.o \
	-L$(SFML)/lib \
	$(LIBS)

app: src/app.cpp
	$(CXX) $(CPPFLAGS) -c src/app.cpp -I$(SFML)/include

main: src/main.cpp
	$(CXX) $(CPPFLAGS) -c src/main.cpp -I$(SFML)/include

cloud: src/cloud.cpp
	$(CXX) $(CPPFLAGS) -c src/cloud.cpp 

cloud-grabbers: src/cloud-grabbers.cpp
	$(CXX) $(CPPFLAGS) -c src/cloud-grabbers.cpp 

cloud-writers: src/cloud-writers.cpp
	$(CXX) $(CPPFLAGS) -c src/cloud-writers.cpp 

guis: src/guis.cpp
	$(CXX) $(CPPFLAGS) -c src/guis.cpp -I$(SFML)/include

scenarios: src/scenarios.cpp
	$(CXX) $(CPPFLAGS) -c src/scenarios.cpp

clean:
	rm -f *.o src/*.o vil
