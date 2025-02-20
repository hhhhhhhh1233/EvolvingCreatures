#pragma once

#include "config.h"
#include <random>

inline static float RandomFloat(float Mult = 1)
{
	return Mult * (float(rand()) / float(RAND_MAX));
}

/// Inclusive, will potentially return Min or Max
inline static float RandomFloatInRange(float Min, float Max)
{
	float x = RandomFloat(Max - Min) + Min;
	assert(x <= Max && x >= Min);
	return x;
}

///  Exclusive, will never return Max
inline static int RandomInt(int Max)
{
	return rand() % Max;
}

inline static int RandomIntInRange(int Min, int Max)
{
	return RandomInt(Max - Min) + Min;
}
