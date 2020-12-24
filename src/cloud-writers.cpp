#include "cloud-writers.h"

//
//	CloudWriter
//
std::string CloudWriter::generate_filename(const std::string& ext,
                                           const std::string& dir,
                                           const std::string& name,
                                           size_t cnt) {
  auto t = std::time(nullptr);
  auto tm = *std::localtime(&t);
  std::ostringstream oss;
  oss << std::put_time(&tm, "%d.%m.%Y-%H.%M.%S");

  std::string filename = dir + "/" + oss.str();
  if (name != "") {
    filename += "-" + name;
  }
  if (cnt != 0) {
    filename += "-" + std::to_string(cnt);
  }
  filename += "." + ext;
  return filename;
}

//
//	CloudFileWriter
//
CloudFileWriter::CloudFileWriter(const std::string& output_dir, CoordSystem coord_sys) {
  output_dir_ = output_dir;
  files_cnt_ = 0;
  coord_sys_ = coord_sys;
}

bool CloudFileWriter::write(const Cloud& cloud) {
  std::string filename;
  if (coord_sys_ == CoordSystem::CART) {
    filename = generate_filename("txt", output_dir_, "cloud-cart", ++files_cnt_);
  } else if (coord_sys_ == CoordSystem::CYL) {
    filename = generate_filename("txt", output_dir_, "cloud-cyl", ++files_cnt_);
  }

  std::ofstream file(filename);
  if (!file) {
    status_ = false;
    std::cerr << "ERROR: Unable to save cloud file." << std::endl;
    return false;
  }

  file << "# RPLIDAR SCAN DATA" << std::endl;
  file << "# Software: https://github.com/knei-knurow/lidar-visualizations" << std::endl;
  file << "# Authors: Szymon Bednorz, Bartek Dudek" << std::endl;
  if (coord_sys_ == CoordSystem::CART) {
    file << "# X Y" << std::endl;
    for (const auto& pt : cloud.pts_cart) {
      file << pt.x << " " << pt.y << std::endl;
    }
  } else if (coord_sys_ == CoordSystem::CYL) {
    file << "# Angle Dist" << std::endl;
    for (const auto& pt : cloud.pts_cyl) {
      file << pt.angle << " " << pt.dist << std::endl;
    }
  }

  std::cout << "Cloud file saved: " << filename << std::endl;
  return true;
}

//
//	CloudFileSeriesWriter
//
CloudFileSeriesWriter::CloudFileSeriesWriter(const std::string& output_dir, CoordSystem coord_sys) {
  clouds_cnt_ = 0;
  coord_sys_ = coord_sys;

  if (coord_sys_ == CoordSystem::CART) {
    filename_ = generate_filename("txt", output_dir, "cloud-ser-cart");
  } else if (coord_sys_ == CoordSystem::CYL) {
    filename_ = generate_filename("txt", output_dir, "cloud-ser-cyl");
  }

  std::cout << "Starting cloud series: " << filename_ << "." << std::endl;
  file_ = std::ofstream(filename_);
  if (!file_) {
    status_ = false;
    std::cerr << "ERROR: Unable to create cloud series file." << std::endl;
  }
  file_ << "# RPLIDAR SCAN DATA SERIES" << std::endl;
  file_ << "# Software: https://github.com/knei-knurow/lidar-visualizations" << std::endl;
  file_ << "# Authors: Szymon Bednorz, Bartek Dudek" << std::endl;
  file_ << "#" << std::endl;
  file_ << "# Each frame (360 deg full scan) is followed by its number and a "
           "time elapsed from the previous frame (in milliseconds)."
        << std::endl;
  file_ << "#" << std::endl;
  if (coord_sys_ == CoordSystem::CART) {
    file_ << "# X, Y" << std::endl;
  } else if (coord_sys_ == CoordSystem::CYL) {
    file_ << "# Angle, Distance" << std::endl;
  }
}

CloudFileSeriesWriter::~CloudFileSeriesWriter() {
  file_.flush();
  file_.close();
  std::cout << "Closing cloud series: " << filename_ << "." << std::endl;
}

bool CloudFileSeriesWriter::write(const Cloud& cloud) {
  if (clouds_cnt_ == 0) {
    time_begin_ = std::chrono::steady_clock::now();
  }
  auto time_now = std::chrono::steady_clock::now();
  auto time_diff =
      std::chrono::duration_cast<std::chrono::milliseconds>(time_now - time_begin_).count();
  time_begin_ = time_now;

  file_ << "! " << ++clouds_cnt_ << " " << time_diff << "\n";
  if (coord_sys_ == CoordSystem::CART) {
    for (const auto& pt : cloud.pts_cart) {
      file_ << pt.x << " " << pt.y << "\n";
    }
  } else if (coord_sys_ == CoordSystem::CYL) {
    for (const auto& pt : cloud.pts_cyl) {
      file_ << pt.angle << " " << pt.dist << "\n";
    }
  }

  file_.flush();
  status_ = file_.good();
  return status_;
}
