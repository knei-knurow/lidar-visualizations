#include "cloud-grabbers.h"

//
//	CloudRPLIDARPortGrabber
//
#ifdef USING_RPLIDAR
CloudRPLIDARPortGrabber::CloudRPLIDARPortGrabber(std::string portname,
                                                 int baudrate,
                                                 RPLIDARScanModes scan_mode,
                                                 int rpm)
    : portname_(portname), baudrate_(baudrate), scan_mode_(scan_mode) {
  driver_ = rplidar::RPlidarDriver::CreateDriver();
  buffer_size_ = rplidar::RPlidarDriver::MAX_SCAN_NODES;
  buffer_ = new rplidar_response_measurement_node_hq_t[buffer_size_];
  if (rpm < 170 || rpm > 1023) {
    std::cerr << "ERROR: RPM value should be between 170 and 1023." << std::endl;
    rpm = 660;
  }
  rpm_ = rpm;
  if (!launch()) {
    stop();
  }
}

CloudRPLIDARPortGrabber::~CloudRPLIDARPortGrabber() {
  stop();
  delete[] buffer_;
}

bool CloudRPLIDARPortGrabber::read(Cloud& cloud) {
  scan();
  if (!status_)
    return false;

  cloud = Cloud();
  for (int i = 0; i < buffer_size_; i++) {
    PointCyl pt_cyl;
    pt_cyl.angle = buffer_[i].angle_z_q14 / 65536.0f * 360;
    pt_cyl.dist = buffer_[i].dist_mm_q2 / 4.0f;

    if (pt_cyl.dist > cloud.max) {
      cloud.max = pt_cyl.dist;
      cloud.max_idx = cloud.size;
    }
    if (pt_cyl.dist < cloud.min && pt_cyl.dist > 0) {
      cloud.min = pt_cyl.dist;
      cloud.min_idx = cloud.size;
    }

    cloud.size++;
    cloud.pts_cyl.push_back(pt_cyl);
    cloud.pts_cart.push_back(pt_cyl.to_cart());
  }
  return status_;
}

bool CloudRPLIDARPortGrabber::print_info() {
  if (!status_)
    return false;

  rplidar_response_device_info_t lidar_info;
  auto res = driver_->getDeviceInfo(lidar_info);
  if (IS_FAIL(res)) {
    std::cerr << "ERROR: Unable to get device info response." << std::endl;
    status_ = false;
    return false;
  }
  std::cout << "LIDAR info:" << std::endl;
  std::cout << "  Model:         " << lidar_info.model << std::endl;
  std::cout << "  Firmware ver.: " << lidar_info.firmware_version << std::endl;
  std::cout << "  Hardware ver.: " << unsigned(lidar_info.hardware_version) << std::endl;
  std::cout << "  Serial number: ";
  for (int i = 0; i < 16; i++) {
    std::cout << std::setfill('0') << std::setw(2) << std::hex << int(lidar_info.serialnum[i])
              << std::dec;
  }
  std::cout << std::endl;
  return true;
}

bool CloudRPLIDARPortGrabber::print_health() {
  if (!status_)
    return false;

  rplidar_response_device_health_t lidar_health;
  auto res = driver_->getHealth(lidar_health);
  if (IS_FAIL(res)) {
    std::cerr << "ERROR: Unable to get device health response." << std::endl;
    status_ = false;
    return false;
  }
  std::cout << "LIDAR health: ";
  if (lidar_health.status == RPLIDAR_STATUS_OK)
    std::cout << "Good";
  else if (lidar_health.status == RPLIDAR_STATUS_WARNING)
    std::cout << "Warning";
  else if (lidar_health.status == RPLIDAR_STATUS_ERROR)
    std::cout << "Error";
  std::cout << std::endl;
  return true;
}

bool CloudRPLIDARPortGrabber::print_scan_modes(std::vector<rplidar::RplidarScanMode>& scan_modes,
                                               _u16& default_mode) {
  if (!status_)
    return false;

  scan_modes.clear();
  auto res = driver_->getTypicalScanMode(default_mode);
  res = driver_->getAllSupportedScanModes(scan_modes);
  if (IS_FAIL(res)) {
    std::cerr << "ERROR: Unable to get device scan modes response." << std::endl;
    return false;
  }
  std::cout << "Supported scan modes:" << std::endl;
  for (const auto& scan_mode : scan_modes) {
    std::cout << "  ";
    std::cout << scan_mode.scan_mode << " (" << scan_mode.id;
    if (scan_mode.id == default_mode)
      std::cout << ", DEFAULT";
    std::cout << "), "
              << "sample time: " << scan_mode.us_per_sample << "us, "
              << "max distance: " << scan_mode.max_distance << "m" << std::endl;
  }
  return true;
}

bool CloudRPLIDARPortGrabber::launch() {
  if (!status_)
    return false;

  std::cout << "LIDAR connection:" << std::endl;
  std::cout << "  Port: " << portname_ << "" << std::endl;
  std::cout << "  Baudrate: " << baudrate_ << "" << std::endl;
  auto res = driver_->connect(portname_.c_str(), baudrate_);
  if (IS_FAIL(res)) {
    std::cerr << "ERROR: Unable to establish connection." << std::endl;
    status_ = false;
    return false;
  }
  std::cout << "Connection established." << std::endl;

  print_info();
  std::vector<rplidar::RplidarScanMode> scan_modes;
  _u16 mode;
  print_scan_modes(scan_modes, mode);
  std::cout << "Selected scan mode: " << _u16(scan_mode_) << "." << std::endl;
  std::cout << "Statring motor." << std::endl;
  res = driver_->startMotor();
  if (IS_FAIL(res)) {
    std::cout << "ERROR: Unable to start motor." << std::endl;
    status_ = false;
    return false;
  }
  std::cout << "Setting motor speed to [rpm]: " << rpm_ << std::endl;
  res = driver_->setMotorPWM(rpm_);
  if (IS_FAIL(res)) {
    std::cout << "ERROR: Unable to set motor RPM." << std::endl;
    status_ = false;
    return false;
  }
  print_health();
  res = driver_->startScanExpress(false, _u16(scan_mode_));
  if (IS_FAIL(res)) {
    std::cout << "ERROR: Unable to start scanning." << std::endl;
    status_ = false;
    return false;
  }
  status_ = true;
  return true;
}

bool CloudRPLIDARPortGrabber::scan() {
  size_t buffer_size = rplidar::RPlidarDriver::MAX_SCAN_NODES;
  auto res = driver_->grabScanDataHq(buffer_, buffer_size);
  buffer_size_ = buffer_size;
  if (IS_FAIL(res)) {
    std::cerr << "ERROR: Unable to read scanning data." << std::endl;
    status_ = false;
    return false;
  }
  driver_->ascendScanData(buffer_, buffer_size);
  return true;
}

void CloudRPLIDARPortGrabber::stop() {
  std::cout << "Stopping motor and deallocating LIDAR driver." << std::endl;
  auto res = driver_->stopMotor();
  if (IS_FAIL(res)) {
    std::cerr << "ERROR: Unable to start motor." << std::endl;
    status_ = false;
  }
  driver_->disconnect();
  rplidar::RPlidarDriver::DisposeDriver(driver_);
}
#endif

//
//	CloudFileGrabber
//
CloudFileGrabber::CloudFileGrabber(const std::string& filename, float rot_angle) {
  filename_ = filename;
  rot_angle_ = rot_angle;
  status_ = std::ifstream(filename_).good();
  if (!status_) {
    std::cerr << "Error: File does not contain a valid cloud." << std::endl;
    status_ = false;
  }
}

bool CloudFileGrabber::read(Cloud& cloud) {
  if (!status_)
    return false;

  if (cloud_.size == 0) {
    std::ifstream file(filename_);
    while (file) {
      std::string line;
      std::getline(file, line);
      if (line.empty() || line[0] == '#')
        continue;
      PointCyl pt_cyl;
      std::stringstream sline(line);
      sline >> pt_cyl.angle >> pt_cyl.dist;

      if (pt_cyl.dist > cloud_.max) {
        cloud_.max = pt_cyl.dist;
        cloud_.max_idx = cloud_.size;
      }
      if (pt_cyl.dist < cloud_.min && pt_cyl.dist > 0) {
        cloud_.min = pt_cyl.dist;
        cloud_.min_idx = cloud_.size;
      }

      cloud_.size++;
      cloud_.pts_cyl.push_back(pt_cyl);
      cloud_.pts_cart.push_back(pt_cyl.to_cart());
    }

    if (cloud_.size == 0) {
      std::cerr << "Error: File does not contain a valid cloud." << std::endl;
      status_ = false;
    }
    cloud = cloud_;
  } else if (rot_angle_ != 0.0f) {
    for (int i = 0; i < cloud.size; i++) {
      cloud.pts_cyl[i].angle += rot_angle_;
      if (cloud.pts_cyl[i].angle >= 360.0f) {
        cloud.pts_cyl[i].angle -= 360.0f;
      } else if (cloud.pts_cyl[i].angle < 0.0f) {
        cloud.pts_cyl[i].angle += 360.0f;
      }
      cloud.pts_cart[i] = cloud.pts_cyl[i].to_cart();
    }
  }

  return status_;
}

//
//	CloudFileSeriesGrabber
//
CloudFileSeriesGrabber::CloudFileSeriesGrabber(const std::string& filename) {
  filename_ = filename;
  clouds_cnt_ = 0;
  file_.open(filename_);
  open();
  next_cloud_time_ = std::chrono::steady_clock::now();
}

bool CloudFileSeriesGrabber::read(Cloud& cloud) {
  if (!status_)
    return false;

  if (std::chrono::steady_clock::now() < next_cloud_time_)
    return true;

  if (cloud.size == 0)
    next_cloud_time_ = std::chrono::steady_clock::now();

  cloud = Cloud();
  while (file_.good()) {
    std::string line;
    std::getline(file_, line);
    PointCyl pt_cyl;
    std::stringstream sline(line);

    if (line.empty() || line[0] == '#') {
      continue;
    } else if (line[0] == '!') {
      long long _, delay_ms = 0;
      char __;
      sline >> __ >> _ >> delay_ms;
      next_cloud_time_ = std::chrono::steady_clock::now() + std::chrono::milliseconds(delay_ms);
      break;
    }
    sline >> pt_cyl.angle >> pt_cyl.dist;

    if (pt_cyl.dist > cloud.max) {
      cloud.max = pt_cyl.dist;
      cloud.max_idx = cloud.size;
    }
    if (pt_cyl.dist < cloud.min && pt_cyl.dist > 0) {
      cloud.min = pt_cyl.dist;
      cloud.min_idx = cloud.size;
    }

    cloud.size++;
    cloud.pts_cyl.push_back(pt_cyl);
    cloud.pts_cart.push_back(pt_cyl.to_cart());
  }

  if (cloud.size == 0 && clouds_cnt_ == 0) {
    std::cerr << "Error: File does not contain a valid cloud." << std::endl;
    status_ = false;
  }

  clouds_cnt_++;

  if (cloud.size == 0) {
    std::cerr << "Cloud series end." << std::endl;
    next_cloud_time_ += std::chrono::milliseconds(1000);
    file_.clear();
    file_.seekg(0);
    status_ = open();
  }

  return status_;
}

bool CloudFileSeriesGrabber::open() {
  while (file_.good()) {
    std::string line;
    std::getline(file_, line);
    if (line.empty() || line[0] == '!') {
      break;
    }
  }

  if (!file_.good()) {
    std::cerr << "Error: File does not contain a valid cloud series." << std::endl;
    status_ = false;
  }
  return status_;
}
