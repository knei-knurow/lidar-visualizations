#include "scenarios.h"

//
//	RecordSeriesScenario
//
RecordSeriesScenario::RecordSeriesScenario(const std::string& output_dir,
	CoordSystem coord_sys)
	: series_writer_(output_dir, coord_sys) {
	status_ = series_writer_.get_status();
}

RecordSeriesScenario::~RecordSeriesScenario() {
}

bool RecordSeriesScenario::update(Cloud& cloud) {
	status_ = series_writer_.write(cloud);
	return status_;
}