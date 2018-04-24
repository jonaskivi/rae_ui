#include "rae/scene/Transform.hpp"
#include <glm/gtc/matrix_transform.hpp>

#include "rae/core/Utils.hpp"

namespace rae
{

Transform::Transform(const vec3& setPosition) :
	position(setPosition),
	localPosition(setPosition)
{
}

String Transform::toString() const
{
	String ret = "x: ";
	ret += Utils::toString(position.x);
	ret += ", y: ";
	ret += Utils::toString(position.y);
	ret += ", z: ";
	ret += Utils::toString(position.z);
	return ret;
}

void Transform::setTarget(glm::vec3 setTarget, float duration)
{
	m_positionAnimator.init(position, setTarget, duration);
}

void Transform::update(double time)
{
	// RAE_TODO Move the animator outside of this class to its own system.
	if (m_positionAnimator.update((float)time) )
	{
		position = m_positionAnimator.value();
	}
}

}
