#pragma once
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include "cloud.h"

class CloudWriter {
 public:
  CloudWriter() : status_(true){};
  virtual ~CloudWriter(){};
  virtual bool write(const Cloud& cloud) = 0;
  virtual bool get_status() const { return status_; }

  static std::string generate_filename(const std::string& ext,
                                       const std::string& dir,
                                       const std::string& name = "",
                                       size_t cnt = 0);

 protected:
  bool status_;
};

class CloudFileWriter : public CloudWriter {
 public:
  CloudFileWriter(const std::string& output_dir,
                  CoordSystem coord_sys = CoordSystem::CYL);
  virtual bool write(const Cloud& cloud);

 private:
  std::string output_dir_;
  size_t files_cnt_;
  CoordSystem coord_sys_;
};

class CloudFileSeriesWriter : public CloudWriter {
 public:
  CloudFileSeriesWriter(const std::string& output_dir,
                        CoordSystem coord_sys = CoordSystem::CYL);
  virtual ~CloudFileSeriesWriter();
  virtual bool write(const Cloud& cloud);

 private:
  std::string filename_;
  std::ofstream file_;
  size_t clouds_cnt_;
  CoordSystem coord_sys_;
  std::chrono::steady_clock::time_point time_begin_;
};
