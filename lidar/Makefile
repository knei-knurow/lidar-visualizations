CXX=g++
CXXFLAGS=-Wall -std=c++17
LIBS=-lsfml-graphics -lsfml-window -lsfml-system

lidar: app.o cloud-grabbers.o cloud-writers.o cloud.o guis.o main.o scenarios.o
		$(CXX) -o vil \
		app.o \
		cloud-grabbers.o \
		cloud-writers.o \
		cloud.o \
		guis.o \
		main.o \
		scenarios.o \
		# -L${CURDIR}/rplidar_sdk/sdk/output/Darwin/Release -lrplidar_sdk	\
		# -L${CURDIR}/SFML/lib $(LIBS)

app.o: app.cpp
	$(CXX) $(CXXFLAGS) -c app.cpp -I${CURDIR}/SFML/include

cloud-grabbers.o: cloud-grabbers.cpp
	$(CXX) $(CXXFLAGS) -c cloud-grabbers.cpp

cloud-writers.o: cloud-writers.cpp
	$(CXX) $(CXXFLAGS) -c cloud-writers.cpp

cloud.o: cloud.cpp
	$(CXX) $(CXXFLAGS) -c cloud.cpp

guis.o: guis.cpp
	$(CXX) $(CXXFLAGS) -c guis.cpp

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp

scenarios.o: scenarios.cpp
	$(CXX) $(CXXFLAGS) -c scenarios.cpp

clean:
	rm *.o lidar
