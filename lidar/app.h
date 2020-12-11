#pragma once
#include "guis.h"
#include "cloud-grabbers.h"
#include "scenarios.h"

class App {
public:
	App(std::vector<std::string>& args);
	~App();

	int run();
	void print_help();
private:
	bool parse_args(std::vector<std::string>& args);

	bool running_;
	std::unique_ptr<CloudGrabber> cloud_grabber_;
	std::unique_ptr<Scenario> scenario_;
	std::unique_ptr<GUI> gui_;
};
