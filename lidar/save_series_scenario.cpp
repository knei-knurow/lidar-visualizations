#include <chrono>
#include <fstream>
#include "app.h"
#include "scenarios.h"

SaveSeriesScenario::SaveSeriesScenario(const std::string& dir) {
	filename_ = create_filename(dir, ".series.txt", 0);
	status_ = true;
	frame_counter_ = 0;
	file_.open(filename_);
	if (!file_) {
		status_ = false;
	}
	file_ << "# RPLIDAR SCAN DATA SERIES" << std::endl;
	file_ << "# Software: https://github.com/knei-knurow/lidar-visualizations" << std::endl;
	file_ << "# Authors: Szymon Bednorz, Bartek Dudek" << std::endl;
	file_ << "#" << std::endl;
	file_ << "# Each frame (360 deg full scan) is followed by its number and a time elapsed from the previous frame (in milliseconds)." << std::endl;
	file_ << "#" << std::endl;
	file_ << "# Angle, Distance" << std::endl;
}

SaveSeriesScenario::~SaveSeriesScenario() {
	file_.flush();
	file_.close();
}

bool SaveSeriesScenario::update(uint8_t* render_buffer, Cloud& cloud) {
	if (frame_counter_ == 0) {
		time_begin_ = std::chrono::steady_clock::now();
	}
	auto time_now = std::chrono::steady_clock::now();
	auto time_diff = std::chrono::duration_cast<std::chrono::milliseconds>(time_now - time_begin_).count();
	time_begin_ = time_now;

	file_ << "! " << ++frame_counter_ << " " << time_diff << "\n";
	for (const auto& pt : cloud.pts) {
		file_ << pt.first << " " << pt.second << "\n";
	}
	file_.flush();

	return file_.good();
}