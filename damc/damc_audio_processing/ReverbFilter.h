#pragma once

#include "DelayFilter.h"
#include <Osc/OscContainer.h>
#include <Osc/OscContainerArray.h>
#include <Osc/OscVariable.h>
#include <stddef.h>
#include <vector>

class ReverbFilter : public OscContainer {
public:
	ReverbFilter(OscContainer* parent, const std::string_view& name);
	void reset(int depth = 1, unsigned int innerReverberatorCount = 5);
	void processSamples(float* samples, size_t count);
	float processOneSample(float input);

private:
	OscVariable<bool> enabled;
	OscVariable<int32_t> delay;
	DelayFilter delayFilter;
	OscVariable<float> gain;
	OscContainerArray<ReverbFilter> reverberators;

	float previousDelayOutput = 0.0f;
};
