#pragma once

#include <glm/gtc/quaternion.hpp>
namespace rae
{
	using quat = glm::quat;
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
	void update(double time, double deltaTime);

	vec3 position;
	quat rotation;
	vec3 scale;

	vec3 localPosition;
	quat localRotation;
	vec3 localScale;

protected:

	// RAE_TODO REMOVE THE REST:
	Animator<glm::vec3> m_positionAnimator;
};

}
