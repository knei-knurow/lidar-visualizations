#pragma once
#include <vector>
#include <string>
#include <chrono>
#include "cloud.h"
#include "cloud-writers.h"

enum class ScenarioType {
	IDLE,
	RECORD_SERIES,
};


class Scenario {
public:
	Scenario() : status_(true) {};
	virtual ~Scenario() {}
	virtual bool update(Cloud& cloud) = 0;
	inline virtual bool get_status() const { return status_; }
	inline virtual ScenarioType get_type() const = 0;

protected:
	bool status_;
};


class RecordSeriesScenario
	: public Scenario {
public:
	RecordSeriesScenario(const std::string& output_dir, 
		CoordSystem coord_sys = CoordSystem::CYL);
	~RecordSeriesScenario();
	virtual bool update(Cloud& cloud);
	inline virtual ScenarioType get_type() const { return ScenarioType::RECORD_SERIES; }

private:
	CloudFileSeriesWriter series_writer_;
};