#include "scenarios.h"

//
//	RecordSeriesScenario
//
RecordSeriesScenario::RecordSeriesScenario(const std::string& output_dir, CoordSystem coord_sys)
    : series_writer_(output_dir, coord_sys) {
  status_ = series_writer_.get_status();
}

bool RecordSeriesScenario::update(Cloud& cloud) {
  status_ = series_writer_.write(cloud);
  return status_;
}

//
//	ScreenshotSeriesScenario
//
ScreenshotSeriesScenario::ScreenshotSeriesScenario(std::function<bool()> screenshot_fn) {
  screenshot_fn_ = screenshot_fn;
}

bool ScreenshotSeriesScenario::update(Cloud& cloud) {
  return screenshot_fn_();
}