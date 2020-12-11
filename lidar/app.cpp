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

App::~App() {
}

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
	std::cout << "-----------------------------------------------------------" << std::endl;
	std::cout << "LIDAR Visualizations" << std::endl;
	std::cout << "-----------------------------------------------------------" << std::endl;
	std::cout << "Authors: Bartek Dudek, Szymon Bednorz" << std::endl;
	std::cout << "Source: https://github.com/knei-knurow/lidar-visualizations" << std::endl;
	std::cout << std::endl;
	std::cout << "Usage:" << std::endl;
	std::cout << "\tlidar [options]" << std::endl;
	std::cout << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "\t-f <arg>  file with lines containing angle [deg] and distance [mm] separated by whitespaces" << std::endl;
	std::cout << "\t-h        Show this message" << std::endl;
	std::cout << "\t-o <arg>  Output directory" << std::endl;
	std::cout << "\t-p <arg>  RPLidar port" << std::endl;
	std::cout << "\t-r        Disable mouse ray" << std::endl;
	std::cout << "\t-s <arg>  Select display scale (1mm -> 1px for scale = 1.0; set 0 to autoscale)" << std::endl;
	std::cout << "\t-S <arg>  Select scenario" << std::endl;
	std::cout << std::endl;
	std::cout << "Scenarios:" << std::endl;
	std::cout << "\t0\tsave point clouds from each frame as batched TXT file" << std::endl;
	std::cout << std::endl;
	std::cout << "GUI Mode Keyboard Shortcuts:" << std::endl;
	std::cout << "\tT           save point cloud as TXT" << std::endl;
	std::cout << "\tS           save screenshot" << std::endl;
	std::cout << "\tUp/Down     scale displayed cloud (faster with shift, slower with ctrl)" << std::endl;
	std::cout << "\tLeft/Right  rotate cloud (faster with shift, slower with ctrl; only with files)" << std::endl;
	std::cout << "\tP           rotation on/off (only with files)" << std::endl;
	std::cout << "\tC           switch color maps" << std::endl;
	std::cout << "\tM           switch point cloud display modes" << std::endl;
	std::cout << "\tR           mouse ray display on/off" << std::endl;
}

bool App::parse_args(std::vector<std::string>& args) {
	std::vector<std::string>::iterator it;

	// Print help
	it = std::find_if(args.begin(), args.end(), [](const std::string& s) { 
		return s == "-h" || s == "--help";}
	);
	if (it != args.end()) {
		print_help();
		return false;
	}

	// Input RPLIDAR port name
	std::string rplidar_port;
	it = std::find_if(args.begin(), args.end(), [](const std::string& s) {
		return s == "-p" || s == "--port"; }
	);
	if (it != args.end() && ++it != args.end()) {
		rplidar_port = *it;
	}

	// Input cloud filename
	std::string cloud_filename;
	it = std::find_if(args.begin(), args.end(), [](const std::string& s) {
		return s == "-f" || s == "--file"; }
	);
	if (it != args.end() && ++it != args.end()) {
		cloud_filename = *it;
	}

	// Input cloud series filename
	std::string cloud_series_filename;
	it = std::find_if(args.begin(), args.end(), [](const std::string& s) {
		return s == "-fs" || s == "--file-series"; }
	);
	if (it != args.end() && ++it != args.end()) {
		cloud_series_filename = *it;
	}

	// Output directory
	std::string output_dir = ".";
	it = std::find_if(args.begin(), args.end(), [](const std::string& s) {
		return s == "-o" || s == "--outpur-dir"; }
	);
	if (it != args.end() && ++it != args.end()) {
		output_dir = *it;
	}

	// RPLIDAR mode
	RPLIDARScanModes rplidar_mode = RPLIDARScanModes::SENSITIVITY;
	it = std::find_if(args.begin(), args.end(), [](const std::string& s) {
		return s == "-m" || s == "--rplidar-mode"; }
	);
	if (it != args.end() && ++it != args.end()) {
		if (*it == std::to_string(int(RPLIDARScanModes::STANDARD))) {
			rplidar_mode = RPLIDARScanModes::STANDARD;
		}
		else if (*it == std::to_string(int(RPLIDARScanModes::EXPRESS))) {
			rplidar_mode = RPLIDARScanModes::EXPRESS;
		}
		else if (*it == std::to_string(int(RPLIDARScanModes::BOOST))) {
			rplidar_mode = RPLIDARScanModes::BOOST;
		}
		else if (*it == std::to_string(int(RPLIDARScanModes::SENSITIVITY))) {
			rplidar_mode = RPLIDARScanModes::SENSITIVITY;
		}
		else if (*it == std::to_string(int(RPLIDARScanModes::STABILITY))) {
			rplidar_mode = RPLIDARScanModes::STABILITY;
		}
		else {
			std::cerr << "ERROR: Invalid RPLIDAR mode id." << std::endl;
		}
	}

	// GUI Type
	GUIType gui_type = GUIType::SFML;
	it = std::find_if(args.begin(), args.end(), [](const std::string& s) {
		return s == "-g" || s == "--gui"; }
	);
	if (it != args.end() && ++it != args.end()) {
		if (*it == std::to_string(int(GUIType::TERMINAL))) {
			gui_type = GUIType::TERMINAL;
		}
		else if (*it == std::to_string(int(GUIType::SFML))) {
			gui_type = GUIType::SFML;
		}
		else {
			std::cerr << "ERROR: Invalid GUI id." << std::endl;
		}
	}

	// Scenario
	ScenarioType scenario_type = ScenarioType::IDLE;
	it = std::find_if(args.begin(), args.end(), [](const std::string& s) {
		return s == "-s" || s == "--scenario"; }
	);
	if (it != args.end() && ++it != args.end()) {
		if (*it == std::to_string(int(ScenarioType::IDLE))) {
			scenario_type = ScenarioType::IDLE;
		}
		else if (*it == std::to_string(int(ScenarioType::RECORD_SERIES))) {
			scenario_type = ScenarioType::RECORD_SERIES;
		}
		else if (*it == std::to_string(int(ScenarioType::SCREENSHOT_SERIES))) {
			scenario_type = ScenarioType::SCREENSHOT_SERIES;
		}
		else {
			std::cerr << "ERROR: Invalid scenario id." << std::endl;
		}
	}

	if (!rplidar_port.empty()) {
		cloud_grabber_ = std::make_unique<CloudRPLIDARPortGrabber>(rplidar_port, 256000, rplidar_mode);
		if (!cloud_grabber_->get_status()) cloud_grabber_.reset(nullptr);
	}
	else if (!cloud_series_filename.empty()) {
		cloud_grabber_ = std::make_unique<CloudFileSeriesGrabber>(cloud_series_filename);
		if (!cloud_grabber_->get_status()) cloud_grabber_.reset(nullptr);
	}
	else if (!cloud_filename.empty()) {
		cloud_grabber_ = std::make_unique<CloudFileGrabber>(cloud_filename, 0.2);
		if (!cloud_grabber_->get_status()) cloud_grabber_.reset(nullptr);
	}

	if (!cloud_grabber_ && !gui_ && !scenario_) {
		return false;
	}

	if (gui_type == GUIType::TERMINAL) {
		gui_ = std::make_unique<TerminalGUI>();
	}
	else if (gui_type == GUIType::SFML) {
		SFMLGUISettings sfml_settings;
		gui_ = std::make_unique<SFMLGUI>(sfml_settings);
	}

	if (scenario_type == ScenarioType::RECORD_SERIES) {
		scenario_ = std::make_unique<RecordSeriesScenario>(output_dir);
	}
	else if (scenario_type == ScenarioType::SCREENSHOT_SERIES) {
		if (gui_type == GUIType::SFML) {
			auto gui_ptr = static_cast<SFMLGUI*>(gui_.get());
			std::function<bool(void)> fn_ptr = std::bind(&SFMLGUI::save_screenshot, gui_ptr);
			scenario_ = std::make_unique<ScreenshotSeriesScenario>(fn_ptr);
		}
		else {
			std::cerr << "ERROR: Selected GUI and scenario are not compatible." << std::endl;
		}
		
	}

	return true;
}
