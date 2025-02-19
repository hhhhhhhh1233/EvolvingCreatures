#pragma once

#include "config.h"
#include <random>

inline static float RandomFloat(float Mult = 1)
{
	return Mult * (float(rand()) / float(RAND_MAX));
}

inline static float RandomFloatInRange(float Min, float Max)
{
	float x = RandomFloat(Max - Min) - -Min;
	assert(x <= Max && x >= Min);
	return x;
}
