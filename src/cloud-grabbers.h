#pragma once
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>
#include "cloud.h"
#ifdef USING_RPLIDAR
#include <rplidar.h>
using namespace rp::standalone;
#endif

enum class CloudGrabberType {
  RPLIDAR_PORT,
  FILE,
  FILE_SERIES,
};

class CloudGrabber {
 public:
  CloudGrabber() : status_(true){};
  virtual ~CloudGrabber(){};
  virtual bool read(Cloud& cloud) = 0;
  virtual bool get_status() const { return status_; }

 protected:
  bool status_;
};

#ifdef USING_RPLIDAR
enum class RPLIDARScanModes {
  STANDARD,
  EXPRESS,
  BOOST,
  SENSITIVITY,
  STABILITY,
  RPLIDAR_SCAN_MODES_COUNT,
};

class CloudRPLIDARPortGrabber : public CloudGrabber {
 public:
  CloudRPLIDARPortGrabber(std::string portname,
                          int baudrate,
                          RPLIDARScanModes scan_mode = RPLIDARScanModes::SENSITIVITY,
                          int rpm = 660);
  virtual ~CloudRPLIDARPortGrabber();
  virtual bool read(Cloud& cloud);

  bool print_info();
  bool print_health();
  bool print_scan_modes(std::vector<rplidar::RplidarScanMode>& scan_modes, _u16& default_mode);
  bool launch();
  bool scan();
  void stop();

 private:
  const std::string portname_;
  const int baudrate_;
  const RPLIDARScanModes scan_mode_;
  int rpm_;
  rplidar::RPlidarDriver* driver_;
  rplidar_response_measurement_node_hq_t* buffer_;
  size_t buffer_size_;
};
#endif

class CloudFileGrabber : public CloudGrabber {
 public:
  CloudFileGrabber(const std::string& filename, float rot_angle_ = 0.0f);
  virtual bool read(Cloud& cloud);

 private:
  std::string filename_;
  float rot_angle_;
  Cloud cloud_;
};

class CloudFileSeriesGrabber : public CloudGrabber {
 public:
  CloudFileSeriesGrabber(const std::string& filename);
  virtual bool read(Cloud& cloud);

 private:
  bool open();

  std::string filename_;
  std::ifstream file_;
  size_t clouds_cnt_;
  std::chrono::steady_clock::time_point next_cloud_time_;
};
