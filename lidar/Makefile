CXX=g++-10
CXXFLAGS=-Wall -std=c++17
LIBS=-lsfml-graphics -lsfml-window -lsfml-system

lidar: app.o cloud-grabbers.o cloud-writers.o cloud.o guis.o main.o scenarios.o
	$(CXX) -o lidar app.o cloud-grabbers.o cloud-writers.o cloud.o guis.o main.o scenarios.o	\
	-L/Users/bartek/dev/cpp/lidar-visualizations/rplidar-sdk/sdk/output/Darwin/Release -lrplidar_sdk	\
	-L/usr/local/Cellar/sfml/2.5.1/lib $(LIBS)

app.o: app.cpp
	$(CXX) $(CXXFLAGS) -c app.cpp -I/usr/local/Cellar/sfml/2.5.1/include

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