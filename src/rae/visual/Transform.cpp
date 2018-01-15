#include <iostream>

#include "Transform.hpp"
#include <glm/gtc/matrix_transform.hpp>

#include "rae/core/Utils.hpp"

namespace rae
{

/* JONDE REMOVE
Transform::Transform(int set_id, float set_x, float set_y, float set_z)
: m_id(set_id),
position(set_x, set_y, set_z)
//JONDE REMOVE m_modelMatrix(1.0f)
{
	
	//JONDE REMOVE updateMatrix();
}

Transform::Transform(int set_id, const vec3& position)
: m_id(set_id),
position(position)
//m_position(0.0f, 0.0f, 5.0f),
//JONDE REMOVE m_modelMatrix(1.0f)
{
	setTarget(position, 5.0f);
	//JONDE REMOVE updateMatrix();
}
*/

Transform::Transform(const vec3& setPosition) :
	position(setPosition),
	localPosition(setPosition)
{
	//JONDE REMOVE setTarget(setPosition, 5.0f);
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

/*JONDE REMOVE void Transform::setPosition(vec3 set)
{
	position = set;
}
*/

void Transform::setTarget(glm::vec3 setTarget, float duration)
{
	m_positionAnimator.init(position, setTarget, duration);
}

void Transform::update(double time, double deltaTime)
{
	// JONDE TODO move the animator outside of this class to its own system.
	if (m_positionAnimator.update((float)time) )
	{
		position = m_positionAnimator.value();
	}

	//JONDE REMOVE std::cout << "transform::update: " + toString() << "\n";

	//JONDE REMOVE updateMatrix();
}

/*JONDE REMOVE
void Transform::updateMatrix()
{
	m_modelMatrix = glm::translate(glm::mat4(1.0f), position);
}
*/

}

