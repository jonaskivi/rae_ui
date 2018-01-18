#include "rae/core/Utils.hpp"
#include <math.h>
#include <cstdlib> // for rand. TODO remove deprecated rand stuff.
#include <assert.h>

namespace rae
{
namespace Math
{

float toRadians(float set_degrees)
{
	return set_degrees * Math::DEG_TO_RAD;
}

float toDegrees(float set_radians)
{
	return set_radians * Math::RAD_TO_DEG;
}

}

namespace Utils
{

int randomInt(int low, int high)
{
	return rand() % ((high + 1) - low) + low;
}

float randFloat(float a_min, float a_max)
{
	const float ooMax = 1.0f / (float)(RAND_MAX-1);		
	float retValue = ( (float)rand() * ooMax * (a_max - a_min) + a_min);

	assert(retValue >= a_min && retValue < a_max);
	
	return retValue;
}

bool isEqual(float set_a, float set_b, float epsilon)
{
	return fabs(set_a - set_b) < epsilon;
}

bool isEqualVec(const glm::vec3& set_a, const glm::vec3& set_b, float epsilon)
{
	return isEqual(set_a.x, set_b.x, epsilon)
		&& isEqual(set_a.y, set_b.y, epsilon)
		&& isEqual(set_a.z, set_b.z, epsilon);
}

float getManhattanDistance(float rx, float ry)
{
	return (float)(rx * rx + ry * ry);
}

float getDistance(float rx, float ry)
{
	return sqrt((float)(rx * rx + ry * ry));
}

float lerpClip(float val1, float val2, float set_position)
{
	// weighted average (so that the sum of the weights becomes 1.0.)
	// return (set_position * val1) + ((1.0f-set_position) * val2);
	float ret = ((1.0f - set_position) * val1) + (set_position * val2);
	if(val1 < val2)
	{
		if(ret < val1)
			return val1;
		else if(ret > val2)
			return val2;
		else return ret;
	}
	else
	{
		if(ret > val1)
			return val1;
		else if(ret < val2)
			return val2;
		else return ret;
	}
	return ret; // won't happen.
}

float lerp(float val1, float val2, float set_position)
{
	// weighted average (so that the sum of the weights becomes 1.0.)
	// return (set_position * val1) + ((1.0f-set_position) * val2);
	return ((1.0f - set_position) * val1) + (set_position * val2);
}

/*
//Some examples to keep my head from spinning.
val1 = 0.1
val2 = 0.5
set_pos = 0.3
((1.0-0.3) * 0.1) + (0.3 * 0.5) = 0.22

val1 = 0.1
val2 = 0.5
the_val = 0.22
result should be = 0.3 (set_pos)
0.5 - 0.1 = 0.4 range
0.22 - 0.1 = 0.12 normalized the_val
0.12 / 0.4 = 0.3 (set_pos)
//thank you.
*/

// percentageWeightedAverage
// kind of like backwardsLerp. You give this the result of lerp,
// and this will give you the percentage for set_position to use in e.g. Sliders.
float backwardsLerp(float val1, float val2, float the_val)
{
	return (the_val - val1) / (val2 - val1);
}

Colour createColor8bit(float r, float g, float b, float a)
{
	return Colour(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
}

String toString(bool set)
{
	return set ? "true" : "false";
}

String toString(float value)
{
	String ret(16, '\0');
	auto written = std::snprintf(&ret[0], ret.size(), "%.4f", value);
	ret.resize(written);
	return ret;
}

String toString(int value)
{
	return std::to_string(value);
}

String toString(glm::vec3 position)
{
	String ret = "x: ";
	ret += Utils::toString(position.x);
	ret += ", y: ";
	ret += Utils::toString(position.y);
	ret += ", z: ";
	ret += Utils::toString(position.z);
	return ret;
}

}
}
