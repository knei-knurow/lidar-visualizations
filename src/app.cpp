#include "app.h"

//
// App
//
App::App(std::vector<std::string>& args) {
  running_ = true;
  if (!parse_args(args)) {
    running_ = false;
  }
}

App::~App() {}

int App::run() {
  Cloud cloud;

  while (running_) {
    // Grab cloud
    if (running_ && cloud_grabber_) {
      if (!cloud_grabber_->read(cloud)) {
        running_ = false;
      }
    }

    // Do some additional things, depending on the selected scenario
    if (running_ && scenario_) {
      if (!scenario_->update(cloud)) {
        running_ = false;
      }
    }

    // Update the GUI with new data
    // Save output files on user's request
    if (running_ && gui_) {
      if (!gui_->update(cloud)) {
        running_ = false;
      }
    }
  }

  return 0;
}

void App::print_help() {
  std::cout << "-----------------------------------------------------------\n"
            << "Lidar Visualizations\n"
            << "-----------------------------------------------------------\n"
            << "Source: https://github.com/knei-knurow/lidar-visualizations\n"
            << "\n"
            << "Usage:\n"
            << "\tlidar [options]\n"
            << "\n"
            << "Options:\n"
            << "\tInput (required):\n"
            << "\t-f  --file [filename]        Input cloud filename\n"
            << "\t-fs --file-series [filename] Input cloud series filename\n"
            << "\t-p  --port [portname]        Input RPLIDAR port*\n"
            << "\n"
            << "\tGeneral:\n"
            << "\t-h  --help                   Display help\n"
            << "\t-o  --output-dir [dirname]   Output dir:\n"
            << "\t-s  --scenario [id]          Specify scenario (default: 0)\n"
            << "\t-g  --gui [id]               Specity GUI (default: 1)\n"
#ifdef USING_RPLIDAR
            << "\n"
            << "\tRPLIDAR options:\n"
            << "\t-m  --rplidar-mode [id]        RPLIDAR scanning mode (default: 4)\n"
            << "\t-r  --rpm                      RPLIDAR revolutions per minute (default: 660, "
               "min: 170, max: 1023)\n"
#endif
#ifdef USING_SFML
            << "\n"
            << "\tSFML GUI options:\n"
            << "\t-H  --height [val]             Window height (defualt: 1280)\n"
            << "\t-W  --width [val]              Window width (defualt: 720)\n"
            << "\t-C  --colormap [id]            Colormap (0, 1)\n"
            << "\t-M  --ptr-mode [id]            Points display mode (0, 1, 2)\n"
            << "\t-B  --bold                     Larger points\n"
            << "\t-S  --scale [scale]            Scale (1mm -> 1px for scale = 1.0)\n"
#endif
            << "\n"
            << "Scenarios:\n"
            << "\t0    Do nothing, just grab a cloudand visualize (default)\n"
            << "\t1    Save each cloud as a part of cloud series\n"
            << "\t2    Save each cloud as a new screenshot (extremely unoptimized)\n"
            << "\n"
            << "GUIs:\n"
            << "\t0    Terminal GUI - prints data as a list of points on stdout\n"
#ifdef USING_SFML
            << "\t1    SFML GUI - default, the most beautiful one from the gallery\n"
#endif
#ifdef USING_RPLIDAR
            << "\n"
            << "RPLIDAR modes:\n"
            << "\t0    Standard\n"
            << "\t1    Express\n"
            << "\t2    Boost\n"
            << "\t3    Sensitivity (default)\n"
            << "\t4    Stability\n"
#endif
#ifdef USING_SFML
            << "\n"
            << "SFML GUI Keyboard Shortcuts:\n"
            << "\tT                 Save cloud to a .txt file\n"
            << "\tS                 Save screenshot\n"
            << "\tArrows            Move cloud\n"
            << "\tMoude scroll      Scale cloud\n"
            << "\tMouse middle      Reset position, autoscale cloud\n"
            << "\tC                 Switch colormap\n"
            << "\tM                 Switch points display mode\n"
#endif
            << "";
}

bool App::check_arg(std::vector<std::string>& all_args,
                    const std::string& short_arg,
                    const std::string& long_arg) {
  auto it = std::find_if(
      all_args.begin(), all_args.end(),
      [short_arg, long_arg](const std::string& s) { return s == short_arg || s == long_arg; });

  if (it == all_args.end()) {
    return false;
  }

  all_args.erase(it);
  return true;
}

std::string App::get_arg_value(std::vector<std::string>& all_args,
                               const std::string& short_arg,
                               const std::string& long_arg,
                               const std::string& default_value) {
  auto it = std::find_if(
      all_args.begin(), all_args.end(),
      [short_arg, long_arg](const std::string& s) { return s == short_arg || s == long_arg; });

  std::string value = default_value;
  if (it == all_args.end()) {
  } else if (it + 1 == all_args.end()) {
    all_args.erase(it);
  } else {
    value = *(it + 1);
    all_args.erase(it, it + 2);
  }
  return value;
}

bool App::parse_args(std::vector<std::string>& args) {
  // Print help
  if (check_arg(args, "-h", "--help")) {
    print_help();
    return false;
  }

  // Input RPLIDAR port name
  std::string rplidar_port = get_arg_value(args, "-p", "--port");

  // Input cloud filename
  std::string cloud_filename = get_arg_value(args, "-f", "--file");

  // Input cloud series filename
  std::string cloud_series_filename = get_arg_value(args, "-fs", "--file-series");

  // Output directory
  std::string output_dir = get_arg_value(args, "-o", "--output-dir", ".");

#ifdef USING_RPLIDAR
  // RPLIDAR mode
  std::string rplidar_mode_val = get_arg_value(args, "-m", "--rplidar-mode",
                                               std::to_string(int(RPLIDARScanModes::SENSITIVITY)));
  RPLIDARScanModes rplidar_mode;
  if (rplidar_mode_val == std::to_string(int(RPLIDARScanModes::STANDARD))) {
    rplidar_mode = RPLIDARScanModes::STANDARD;
  } else if (rplidar_mode_val == std::to_string(int(RPLIDARScanModes::EXPRESS))) {
    rplidar_mode = RPLIDARScanModes::EXPRESS;
  } else if (rplidar_mode_val == std::to_string(int(RPLIDARScanModes::BOOST))) {
    rplidar_mode = RPLIDARScanModes::BOOST;
  } else if (rplidar_mode_val == std::to_string(int(RPLIDARScanModes::SENSITIVITY))) {
    rplidar_mode = RPLIDARScanModes::SENSITIVITY;
  } else if (rplidar_mode_val == std::to_string(int(RPLIDARScanModes::STABILITY))) {
    rplidar_mode = RPLIDARScanModes::STABILITY;
  } else {
    std::cerr << "ERROR: Invalid RPLIDAR mode id." << std::endl;
    return false;
  }

  // RPLIDAR RPM
  int rplidar_rpm = 660;
  std::stringstream(get_arg_value(args, "-r", "--rpm")) >> rplidar_rpm;
#endif

  // GUI
  std::string gui_type_val = get_arg_value(args, "-g", "--gui", std::to_string(int(GUIType::SFML)));
  GUIType gui_type;
  if (gui_type_val == std::to_string(int(GUIType::TERMINAL))) {
    gui_type = GUIType::TERMINAL;
  }
#ifdef USING_SFML
  else if (gui_type_val == std::to_string(int(GUIType::SFML))) {
    gui_type = GUIType::SFML;
  }
#endif
  else {
    std::cerr << "ERROR: Invalid GUI id." << std::endl;
    return false;
  }

  // Scenario
  std::string scenario_val =
      get_arg_value(args, "-s", "--scenario", std::to_string(int(ScenarioType::IDLE)));
  ScenarioType scenario_type;
  if (scenario_val == std::to_string(int(ScenarioType::IDLE))) {
    scenario_type = ScenarioType::IDLE;
  } else if (scenario_val == std::to_string(int(ScenarioType::RECORD_SERIES))) {
    scenario_type = ScenarioType::RECORD_SERIES;
  } else if (scenario_val == std::to_string(int(ScenarioType::SCREENSHOT_SERIES))) {
    scenario_type = ScenarioType::SCREENSHOT_SERIES;
  } else {
    std::cerr << "ERROR: Invalid scenario id." << std::endl;
    return false;
  }

  // Initialize the cloud grabber
  if (!cloud_series_filename.empty()) {
    cloud_grabber_ = std::make_unique<CloudFileSeriesGrabber>(cloud_series_filename);
    if (!cloud_grabber_->get_status())
      cloud_grabber_.reset(nullptr);
  } else if (!cloud_filename.empty()) {
    cloud_grabber_ = std::make_unique<CloudFileGrabber>(cloud_filename, 0.2);
    if (!cloud_grabber_->get_status())
      cloud_grabber_.reset(nullptr);
  }
#ifdef USING_RPLIDAR
  else if (!rplidar_port.empty()) {
    cloud_grabber_ =
        std::make_unique<CloudRPLIDARPortGrabber>(rplidar_port, 256000, rplidar_mode, rplidar_rpm);
    if (!cloud_grabber_->get_status())
      cloud_grabber_.reset(nullptr);
  }
#endif
  if (!cloud_grabber_ && !gui_ && !scenario_) {
    print_help();
    return false;
  }

  // Initialize the GUI
  if (gui_type == GUIType::TERMINAL) {
    gui_ = std::make_unique<TerminalGUI>();
  }
#ifdef USING_SFML
  else if (gui_type == GUIType::SFML) {
    SFMLGUISettings sfml_settings;

    unsigned colormap_temp = -1, display_mode_temp = -1;

    std::stringstream(get_arg_value(args, "-W", "--width")) >> sfml_settings.width;
    std::stringstream(get_arg_value(args, "-H", "--height")) >> sfml_settings.height;

    if (bool(std::stringstream(get_arg_value(args, "-C", "--colormap")) >> colormap_temp))
      sfml_settings.colormap = static_cast<SFMLGUISettings::Colormap>(
          colormap_temp % SFMLGUISettings::Colormap::COLORMAP_COUNT);

    if (bool(std::stringstream(get_arg_value(args, "-M", "--ptr-mode")) >> display_mode_temp))
      sfml_settings.pts_display_mode = static_cast<SFMLGUISettings::PtsDispayMode>(
          display_mode_temp % SFMLGUISettings::PtsDispayMode::PTS_DISPLAY_MODE_COUNT);

    if (bool(std::stringstream(get_arg_value(args, "-S", "--scale")) >> sfml_settings.scale))
      sfml_settings.autoscale = false;

    if (check_arg(args, "-B", "--bold"))
      sfml_settings.bold_mode = true;

    sfml_settings.output_dir = output_dir;

    gui_ = std::make_unique<SFMLGUI>(sfml_settings);
  }
#endif

  // Initialize the scenario
  if (scenario_type == ScenarioType::RECORD_SERIES) {
    scenario_ = std::make_unique<RecordSeriesScenario>(output_dir);
  }
#ifdef USING_SFML
  else if (scenario_type == ScenarioType::SCREENSHOT_SERIES) {
    if (gui_type == GUIType::SFML) {
      auto gui_ptr = static_cast<SFMLGUI*>(gui_.get());
      std::function<bool(void)> fn_ptr = std::bind(&SFMLGUI::save_screenshot, gui_ptr);
      scenario_ = std::make_unique<ScreenshotSeriesScenario>(fn_ptr);
    } else {
      std::cerr << "ERROR: Selected GUI and scenario are not compatible." << std::endl;
      return false;
    }
  }
#endif

  if (!args.empty()) {
    std::cerr << "WARNING: Unused command line arguments: ";
    for (const auto& unused_arg : args) {
      std::cerr << "\"" << unused_arg << "\" ";
    }
    std::cerr << std::endl;
  }

  return true;
}
