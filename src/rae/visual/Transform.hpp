#pragma once

#define GLM_FORCE_RADIANS
#include <glm/gtc/quaternion.hpp>
namespace rae
{
	using qua = glm::quat;
	using mat4 = glm::mat4;
}

#include "rae/core/Types.hpp"
#include "rae/animation/Animator.hpp"

namespace rae
{

struct Transform
{
public:
	Transform(){}

	Transform(const vec3& setPosition);
	String toString() const;

	void setTarget(glm::vec3 setTarget, float duration);
	void update(double time);

	vec3 position = vec3(0.0f, 0.0f, 0.0f);
	qua rotation;
	vec3 scale = vec3(1.0f, 1.0f, 1.0f);

	vec3 localPosition = vec3(0.0f, 0.0f, 0.0f);
	qua localRotation;
	vec3 localScale = vec3(1.0f, 1.0f, 1.0f);

protected:

	// RAE_TODO REMOVE THE REST:
	Animator<glm::vec3> m_positionAnimator;
};

}
