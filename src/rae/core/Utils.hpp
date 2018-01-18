#pragma once

#define _USE_MATH_DEFINES
#include <math.h>
#include <string>
#include <algorithm>
#include <thread>

#include <glm/glm.hpp>

#include "rae/core/Types.hpp"

namespace rae
{

namespace Math
{
	const float TAU = 2.0f * (float)M_PI; // www.tauday.org
	const float PI = (float)M_PI;
	const float QUARTER_TAU = 0.5f * (float)M_PI;
	const float RAD_TO_DEG = 360.0f / TAU;
	const float DEG_TO_RAD = TAU / 360.0f;

	float toRadians(float set_degrees);
	float toDegrees(float set_radians);
}

namespace Utils
{
	
int randomInt(int low, int high);
float randFloat(float a_min, float a_max);
bool isEqual(float set_a, float set_b, float epsilon = 0.0001f);
bool isEqualVec(const glm::vec3& set_a, const glm::vec3& set_b, float epsilon = 0.0001f);

// 2D versions of distance:
float getManhattanDistance(float rx, float ry);
float getDistance(float rx, float ry);

float lerpClip(float val1, float val2, float set_position);
float lerp(float val1, float val2, float set_position);
float backwardsLerp(float val1, float val2, float the_val);

Colour createColor8bit(float r, float g, float b, float a);

String toString(bool set);
String toString(float value); // A string with four decimal places
String toString(int value);
String toString(glm::vec3 position);

}

/* A simple parallel for loop.
// Usage example:
parallel_for(0, array.size(), [&](int i)
{
	array[i] = computeSomeResult();
}
*/
template<typename Callable>
static void parallel_for(int start, int end, Callable func)
{
	const static int threadCountHint = (int)std::thread::hardware_concurrency();
	const static int threadCount = (threadCountHint == 0 ? 8 : threadCountHint);

	std::vector<std::thread> threads(threadCount);

	for (int t = 0; t < threadCount; ++t)
	{
		threads[t] = std::thread(std::bind(
		[&](const int beginIndex, const int endIndex, const int t)
		{
			for (int i = beginIndex; i < endIndex; ++i)
			{
				func(i);
			}
		},
		t * end / threadCount,
		(t+1) == threadCount ? end : (t+1) * end / threadCount,
		t));
	}

	std::for_each(threads.begin(),threads.end(), [](std::thread& x)
	{
		x.join();
	});
}

} // end namespace rae
