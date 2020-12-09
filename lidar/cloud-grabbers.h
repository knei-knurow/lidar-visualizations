#pragma once
#include <vector>
#include <numeric>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>

#ifdef USING_RPLIDAR
#include <rplidar.h>
using namespace rp::standalone;
#endif

struct PointCart {
	int x = 0;
	int y = 0;
};

struct PointCyl {
	float angle = 0;
	float dist = 0;

	PointCart to_cart() const;
	PointCart to_cart(float scale, float origin_x, float origin_y) const;
};

struct Cloud {
	std::vector<PointCyl> pts_cyl;
	std::vector<PointCart> pts_cart;
	float min = std::numeric_limits<float>::max();
	float max = 0;
	size_t max_idx = -1;
	size_t min_idx = -1;
	size_t size = 0;
};

enum class CloudGrabberType {
#ifdef USING_RPLIDAR
	RPLIDAR_PORT,
	FILE,
	FILE_SERIES,
#endif
};

class CloudGrabber {
public:
	CloudGrabber() : status_(true) {};
	virtual ~CloudGrabber() {};
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

class CloudRPLIDARPortGrabber
	: public CloudGrabber {
public:
	CloudRPLIDARPortGrabber(std::string portname, int baudrate, RPLIDARScanModes scan_mode = RPLIDARScanModes::SENSITIVITY);
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
	rplidar::RPlidarDriver* driver_;
	rplidar_response_measurement_node_hq_t* buffer_;
	size_t buffer_size_;
};
#endif


class CloudFileGrabber 
	: public CloudGrabber {
public:
	CloudFileGrabber(const std::string& filename, float rot_angle_ = 0.0f);
	virtual bool read(Cloud& cloud);

private:
	std::string filename_;
	float rot_angle_;
	Cloud cloud_;
};

// TODO
class CloudFileSeriesGrabber
	: public CloudGrabber {
public:
	CloudFileSeriesGrabber(const std::string& filename);
	virtual bool read(Cloud& cloud);

private:
};