#pragma once

#ifdef WINDOWED_APP
#include <rplidar.h>

using namespace rp::standalone;

const int DEF_BAUDRATE = 256000;
static const char* DEF_PORTTT = "COM3";


bool rplidar_print_info(rplidar::RPlidarDriver* lidar);

bool rplidar_print_health(rplidar::RPlidarDriver* lidar, _u8* status = nullptr);

bool rplidar_print_scan_modes(rplidar::RPlidarDriver* lidar, _u16* scan_mode = nullptr);

bool rplidar_launch(rplidar::RPlidarDriver* lidar, const char* port = "COM3", _u32 baudrate = DEF_BAUDRATE);

void rplidar_stop(rplidar::RPlidarDriver* lidar);

#endif