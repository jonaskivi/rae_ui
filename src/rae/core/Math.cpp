#include "rae/core/Math.hpp"

#include <math.h>
#include <cstdlib> // for rand. TODO remove deprecated rand stuff.
#include <assert.h>

#include <glm/gtc/epsilon.hpp>

namespace rae
{
namespace Math
{

float toRadians(float set_degrees)
{
	return set_degrees * Math::DegToRad;
}

float toDegrees(float set_radians)
{
	return set_radians * Math::RadToDeg;
}

float snapValue(float value, float stepSize)
{
	return std::round(value / stepSize) * stepSize;
}

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

// Creates a Tait-Bryan (Yaw-Pitch-Roll) Euler angle representation of the given quaternion components.
//
// Yaw = Y, Pitch = X, Roll = Z
// The order of rotations is done in the YXZ order. The singularities in the Y-axis poles have special handling.
// Rotations are counter-clockwise on all axes.
// Limits: Yaw and Roll: -180 to 180 degrees.
// Pitch: -90 to 90 degrees, passing poles will flip Yaw and Roll between 0 and +-180. North pole is -90 degrees.
// The formula is derived from http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToEuler/
// except with Z-forward coordinate system instead of X-forward
// Returned angles are in radians.
glm::vec3 quaToEuler(float x, float y, float z, float w, float epsilon)
{
	float yaw;
	float pitch;
	float roll;

	// Handle singularities at poles
	float test = z * y + x * w;

	// If you want to show the values in UI, you might want to provide a small epsilon,
	// so that 90 degrees is shown as 90 degrees in the UI. For other cases, it's fine to
	// leave the epsilon to 0.0f.
	if (test > (0.5f - epsilon)) // Singularity at south pole
	{
		yaw = 2.0f * atan2f(z, w);
		pitch = Math::QuarterTau;
		roll = 0.0f;
		return glm::vec3(pitch, yaw, roll);
	}

	if (test < (-0.5f + epsilon)) // Singularity at north pole
	{
		yaw = -2.0f * atan2f(z, w);
		pitch = -Math::QuarterTau;
		roll = 0.0f;
		return glm::vec3(pitch, yaw, roll);
	}

	float sqy = y * y;
	float sqx = x * x;
	float sqz = z * z;

	yaw = atan2f(2.0f * y * w - 2.0f * z * x, 1.0f - 2.0f * sqy - 2.0f * sqx);
	pitch = asinf(2.0f * test);
	roll = atan2f(2.0f * z * w - 2.0f * y * x, 1.0f - 2.0f * sqz - 2.0f * sqx);

	return glm::vec3(pitch, yaw, roll);
}

glm::vec3 quaToEuler(const glm::quat& rot, float epsilon)
{
	return quaToEuler(rot[0], rot[1], rot[2], rot[3], epsilon);
}

bool isZero(const glm::vec3 v, float epsilon)
{
	return glm::all(glm::epsilonEqual(v, glm::vec3(), epsilon));
}

glm::vec3 anyOrthogonalVector(glm::vec3 a)
{
	glm::vec3 smallestAxis = glm::vec3(1.0f, 0.0f, 0.0f);

	glm::vec3 absA = glm::abs(a);

	// X is smallest.
	if (absA.x <= absA.y &&
		absA.x <= absA.z)
	{
		// Use the default value.
	}
	// Y is smallest.
	else if (absA.y <= absA.x &&
			 absA.y <= absA.z)
	{
		smallestAxis = glm::vec3(0.0f, 1.0f, 0.0f);
	}
	else // Z is smallest.
	{
		smallestAxis = glm::vec3(0.0f, 0.0f, 1.0f);
	}

	return glm::cross(a, smallestAxis);
}

glm::quat rotationOntoNormalized(glm::vec3 from, glm::vec3 to)
{
	glm::vec3 result;

	// First, we store cos(alpha)
	float w = glm::dot(from, to);

	// We compute cos(alpha/2) using the equality cos(acos(x)/2) = sqrt((x+1)/2)
	w = (w + 1.0f) * 0.5f;

	const float epsilon = 0.001f;

	if (w < epsilon) 
	{
		// Almost opposite, special case to avoid singularities
		result = glm::normalize(anyOrthogonalVector(from)) * sqrt(1.0f - (w * w));
	}
	else 
	{
		w = sqrt(w);
		// First we compute the half vector. As both input vectors are normalized, the half vector's length
		// will be cos(alpha/2)
		glm::vec3 half = glm::vec3(0.5f) * (from + to);

		// Next we compute the cross product, which results in sin(alpha/2)*cos(alpha/2)*axis
		result = glm::cross(from, half);

		// Now we divide xyz by w and return
		result = result / w;
	}

	// Note that w is passed first. That detail cost me about 2-3 hours.
	return glm::quat(w, result.x, result.y, result.z);
}

glm::quat rotationOnto(glm::vec3 from, glm::vec3 to)
{
	return rotationOntoNormalized(glm::normalize(from), glm::normalize(to));
}

float signedAngle(const glm::vec3& a, const glm::vec3& b, const glm::vec3& planeNormal)
{
	float dot = Math::clamp(glm::dot(a, b), -1.0f, 1.0f);
	float angle = acosf(dot);
	glm::vec3 cross = glm::cross(a, b);
	if (glm::dot(planeNormal, cross) < 0.0f)
	{
		angle = -angle;
	}
	return angle;
}

}
}
