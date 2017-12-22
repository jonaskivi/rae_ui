#ifndef UTILS_H
#define UTILS_H

#define _USE_MATH_DEFINES
#include <math.h>
#include <string>

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

// 2D versions of distance:
float getManhattanDistance(float rx, float ry);
float getDistance(float rx, float ry);

float lerpClip(float val1, float val2, float set_position);
float lerp(float val1, float val2, float set_position);
float backwardsLerp(float val1, float val2, float the_val);

String toString(bool set);
String toString(float value); // A string with four decimal places
String toString(int value);
String toString(glm::vec3 position);

float randFloat(float a_min, float a_max);
bool isEqual(float set_a, float set_b, float epsilon = 0.0001f);
bool isEqualVec(const glm::vec3& set_a, const glm::vec3& set_b, float epsilon = 0.0001f);

}

} // end namespace rae

#endif
