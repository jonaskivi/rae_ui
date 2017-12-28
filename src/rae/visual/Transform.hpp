#ifndef RAE_TRANSFORM_HPP
#define RAE_TRANSFORM_HPP

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
using glm::vec3;
using glm::quat;
using glm::mat4;

#include "rae/core/Types.hpp"
#include "rae/animation/Animator.hpp"

namespace rae
{

struct Transform
{
public:
	/*int id() { return m_id; }
protected:
	void id(int set) { m_id = set; }
	int m_id;
	*/

public:
	Transform(){}
	//JONDE REMOVE Transform(int set_id, float set_x = 0.0f, float set_y = 0.0f, float set_z = 0.0f);
	//JONDE REMOVE Transform(int set_id, const vec3& position);

	Transform(const vec3& setPosition);
	String toString() const;

	//JONDE REMOVE void setPosition(glm::vec3 set);
	void setTarget(glm::vec3 setTarget, float duration);
	void update(double time, double deltaTime);

	//JONDE REMOVE void updateMatrix();

	//JONDE REMOVE const vec3& getPosition() { return m_position; }
	//JONDE REMOVE mat4& modelMatrix() { return m_modelMatrix; } //TODO const

	vec3 position;
	quat rotation;
	vec3 scale;

	vec3 localPosition;
	quat localRotation;
	vec3 localScale;

protected:

	// JONDE REMOVE THE REST:
	Animator<glm::vec3> m_positionAnimator;

	//JONDE REMOVE glm::mat4 m_modelMatrix;
};

}

#endif

