#pragma once

#define _USE_MATH_DEFINES
#include <math.h>
#include <algorithm>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace rae
{

namespace Math
{

const float Tau = 2.0f * (float)M_PI; // www.tauday.org
const float Pi = (float)M_PI;
const float QuarterTau = 0.5f * (float)M_PI;
const float RadToDeg = 360.0f / Tau;
const float DegToRad = Tau / 360.0f;

float toRadians(float set_degrees);
float toDegrees(float set_radians);

template <typename T>
T clamp(const T& n, const T& lower, const T& upper)
{
	return std::max(lower, std::min(n, upper));
}

float snapValue(float value, float stepSize);

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

// Quaternion to euler angles conversion. Returned angles are in radians.
glm::vec3 quaToEuler(float x, float y, float z, float w, float epsilon = 0.0f);
glm::vec3 quaToEuler(const glm::quat& rot, float epsilon = 0.0f);

bool isZero(const glm::vec3 v, float epsilon = 1e-7f);

glm::vec3 anyOrthogonalVector(glm::vec3 a);
// Makes a rotation that rotates from the first direction vector to the second direction vector.
// Both vectors must be normalized.
glm::quat rotationOntoNormalized(glm::vec3 from, glm::vec3 to);
// Makes a rotation that rotates from the first direction vector to the second direction vector.
glm::quat rotationOnto(glm::vec3 from, glm::vec3 to);

// Angle in radians between vectors a and b on a plane with normal n. Assumes a and b are normalized.
float signedAngle(const glm::vec3& a, const glm::vec3& b, const glm::vec3& planeNormal);

} // namespace Math

} // namespace rae
