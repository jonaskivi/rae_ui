#include "TransformSystem.hpp"
#include <iostream>

using namespace rae;

static const int ReserveTransforms = 1000;

TransformSystem::TransformSystem()
: m_transforms(ReserveTransforms)
{

}

bool TransformSystem::update(double time, double deltaTime, std::vector<Entity>& entities)
{
	/* JONDE REMOVE
	std::cout << "Printing out TransformSystem:\n";
	int i = 0;
	for (auto&& transform : m_transforms.items())
	{
		std::cout << "i: " << i << " transform: " << transform.toString() << "\n";
		++i;
	}
	*/

	for (auto&& transform : m_transforms.items())
	{
		transform.update(time, deltaTime);
	}
}

void TransformSystem::addTransform(Id id, Transform&& transform)
{
	m_transforms.create(id, std::move(transform));
}

const Transform& TransformSystem::getTransform(Id id)
{
	return m_transforms.get(id);
}

void TransformSystem::setPosition(Id id, const vec3& position)
{
}

const vec3& TransformSystem::getPosition(Id id)
{

}
