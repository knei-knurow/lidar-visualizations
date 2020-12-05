#pragma once
#include <rplidar.h>

using namespace rp::standalone;

const int DEF_BAUDRATE = 256000;
static const char* DEF_PORT = "COM3";

bool rplidar_print_info(rplidar::RPlidarDriver* lidar);

bool rplidar_print_health(rplidar::RPlidarDriver* lidar, _u8* status = nullptr);

bool rplidar_print_scan_modes(rplidar::RPlidarDriver* lidar, _u16* mode = nullptr, _u16 preffered_mode_id = -1);

void rplidar_print_scan_info(rplidar_response_measurement_node_hq_t* buffer, size_t count);

bool rplidar_launch(rplidar::RPlidarDriver* lidar, std::string port = DEF_PORT, _u32 baudrate = DEF_BAUDRATE, _u16 preffered_mode_id = -1);

bool rplidar_scan(rplidar::RPlidarDriver* lidar, rplidar_response_measurement_node_hq_t* buffer, size_t & count, bool verbose = false);

void rplidar_stop(rplidar::RPlidarDriver* lidar);
