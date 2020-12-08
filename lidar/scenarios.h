#pragma once
#include <vector>
#include <string>
#include <chrono>
#include "app.h"

enum class ScenarioType {
	IDLE,
	SCENARIO_TYPE_COUNT,
};

class Scenario {
public:
	virtual bool update(Cloud& cloud) = 0;
	inline virtual int get_status() const { return status_; }
protected:
	int status_;
};

class SaveSeriesScenario
	: public Scenario {
public:
	SaveSeriesScenario(const std::string& dir);
	~SaveSeriesScenario();

	virtual bool update(uint8_t* render_buffer, Cloud& cloud);
private:
	std::string filename_;
	std::ofstream file_;
	std::size_t frame_counter_;
	std::chrono::steady_clock::time_point time_begin_;
};