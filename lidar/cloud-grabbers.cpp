#include "cloud-grabbers.h"

PointCart PointCyl::to_cart() const {
	int x = int(std::round(dist * std::sin(angle * (acos(-1.0f) / 180.0f))));
	int y = int(std::round(dist * std::cos(angle * (acos(-1.0f) / 180.0f))));
	return { x, y };
}

PointCart PointCyl::to_cart(float scale, float origin_x, float origin_y) const {
	int x = int(std::round(dist * std::sin(angle * (acos(-1.0f) / 180.0f)) * scale)) + origin_x;
	int y = int(std::round(dist * std::cos(angle * (acos(-1.0f) / 180.0f)) * scale)) + origin_y;
	return { x, y };
}


#ifdef USING_RPLIDAR
CloudLidarPortGrabber::CloudLidarPortGrabber(std::string portname, int baudrate)
	: portname_(portname), baudrate_(baudrate) {
	driver_ = rplidar::RPlidarDriver::CreateDriver();
	buffer_size_ = rplidar::RPlidarDriver::MAX_SCAN_NODES;
	buffer_ = new rplidar_response_measurement_node_hq_t[buffer_size_];
	launch();
}

CloudLidarPortGrabber::~CloudLidarPortGrabber() {
	stop();
	delete[] buffer_;
}

bool CloudLidarPortGrabber::read(Cloud& cloud) {
	scan(false);
	if (!status_) return false;

	cloud = Cloud();
	for (int i = 0; i < buffer_size_; i++) {
		PointCyl pt_cyl;
		pt_cyl.angle = buffer_[i].angle_z_q14 / 65536.0f * 360;
		pt_cyl.dist = buffer_[i].dist_mm_q2 / 4.0f;

		if (pt_cyl.dist == 0) continue;
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

bool CloudLidarPortGrabber::print_info() {
	if (!status_) return false;

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
		std::cout << std::setfill('0') << std::setw(2) << std::hex << int(lidar_info.serialnum[i]) << std::dec;
	}
	std::cout << std::endl;
	return true;
}

bool CloudLidarPortGrabber::print_health() {
	if (!status_) return false;

	rplidar_response_device_health_t lidar_health;
	auto res = driver_->getHealth(lidar_health);
	if (IS_FAIL(res)) {
		std::cerr << "ERROR: Unable to get device health response." << std::endl;
		status_ = false;
		return false;
	}
	std::cout << "LIDAR health: ";
	if (lidar_health.status == RPLIDAR_STATUS_OK) std::cout << "Good";
	else if (lidar_health.status == RPLIDAR_STATUS_WARNING) std::cout << "Warning";
	else if (lidar_health.status == RPLIDAR_STATUS_ERROR) std::cout << "Error";
	std::cout << std::endl;
	return true;
}

bool CloudLidarPortGrabber::print_scan_modes(_u16 preffered_mode_id) {
	if (!status_) return false;

	std::vector<rplidar::RplidarScanMode> scan_modes;
	_u16 current_scan_mode;
	auto res = driver_->getTypicalScanMode(current_scan_mode);
	res = driver_->getAllSupportedScanModes(scan_modes);
	if (IS_FAIL(res)) {
		std::cerr << "ERROR: Unable to get device scan modes response." << std::endl;
		return false;
	}
	std::cout << "Supported scan modes:" << std::endl;
	for (const auto& scan_mode : scan_modes) {
		std::cout << "  ";
		std::cout << scan_mode.scan_mode << " (" << scan_mode.id;
		if (scan_mode.id == current_scan_mode) std::cout << ", DEFAULT";
		if (preffered_mode_id == scan_mode.id) {
			std::cout << ", SELECTED";
			current_scan_mode = preffered_mode_id;
		}
		std::cout << "), "
			<< "sample time: " << scan_mode.us_per_sample << "us, "
			<< "max distance: " << scan_mode.max_distance << "m" << std::endl;
	}
	return true;
}

void CloudLidarPortGrabber::print_scan_info() const {
	int zero_quality = 0, avg_quality = 0, min_dist = 10000000, max_dist = 0;
	for (size_t i = 0; i < buffer_size_; i++) {
		if (buffer_[i].quality == 0) zero_quality++;
		else {
			if (buffer_[i].dist_mm_q2 < min_dist) min_dist = buffer_[i].dist_mm_q2;
			if (buffer_[i].dist_mm_q2 > max_dist) max_dist = buffer_[i].dist_mm_q2;
		}
		avg_quality += buffer_[i].quality;
	}
	avg_quality /= buffer_size_;
	std::cout << "- "
		<< "count: " << buffer_size_
		<< ", zeros: " << zero_quality << " (" << zero_quality * 100 / buffer_size_ << "%)"
		<< ", avg. quality: " << avg_quality << std::endl;
}

bool CloudLidarPortGrabber::launch(_u16 scan_mode) {
	if (!status_) return false;

	std::cout << "LIDAR connection:" << std::endl;
	std::cout << "  Port: " << portname_ << "" << std::endl;
	std::cout << "  Baudrate: " << baudrate_ << "" << std::endl;
	auto res = driver_->connect(portname_.c_str(), baudrate_);
	if (IS_FAIL(res)) {
		std::cerr << "ERROR: Unable to establish connection with LIDAR." << std::endl;
		status_ = false;
		return false;
	}
	std::cout << "Connection established." << std::endl;
	print_info();
	print_health();
	print_scan_modes();
	std::cout << "Statring motor." << std::endl;
	res = driver_->startMotor();
	if (IS_FAIL(res)) {
		std::cout << "ERROR: Unable to start motor." << std::endl;
		status_ = false;
		return false;
	}
	res = driver_->startScanExpress(false, scan_mode);
	if (IS_FAIL(res)) {
		std::cout << "ERROR: Unable to start scanning." << std::endl;
		status_ = false;
		return false;
	}
	return true;
}

bool CloudLidarPortGrabber::scan(bool verbose) {
	size_t buffer_size;
	auto res = driver_->grabScanDataHq(buffer_, buffer_size);
	buffer_size_ = buffer_size;
	if (IS_FAIL(res)) {
		std::cerr << "ERROR: Unable to read scanning data." << std::endl;
		status_ = false;
		return false;
	}
	driver_->ascendScanData(buffer_, buffer_size);
	if (verbose) print_scan_info();
	return true;
}

void CloudLidarPortGrabber::stop() {
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

CloudFileGrabber::CloudFileGrabber(const std::string& filename) {
	filename_ = filename;
}

bool CloudFileGrabber::read(Cloud& cloud) {
	if (!status_) return false;

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

			if (pt_cyl.dist == 0) continue;
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
	}

	cloud = cloud_;

	return status_;
}


CloudFileSeriesGrabber::CloudFileSeriesGrabber(const std::string& filename) {
	// TODO
}

bool CloudFileSeriesGrabber::read(Cloud& cloud) {
	return false;
}

