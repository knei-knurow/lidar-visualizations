#include <SFML/Graphics.hpp>
#include <rplidar.h>
#include <iostream>

using namespace rp::standalone::rplidar;

int main(int argc, char* argv)
{
    RPlidarDriver* lidar = RPlidarDriver::CreateDriver();

    std::cout << "Does it really work?\n";

    RPlidarDriver::DisposeDriver(lidar);
}