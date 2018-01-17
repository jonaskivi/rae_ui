#include "TransformSystem.hpp"
#include <iostream>

using namespace rae;

static const int ReserveTransforms = 1000;

TransformSystem::TransformSystem() :
	m_transforms(ReserveTransforms)
{
	addTable(m_transforms);
}

bool TransformSystem::update(double time, double deltaTime)
{
	for (auto&& transform : m_transforms.items())
	{
		transform.update(time, deltaTime);
	}
	return false;
}

void TransformSystem::addTransform(Id id, Transform&& transform)
{
	m_transforms.assign(id, std::move(transform));
}

bool TransformSystem::hasTransform(Id id) const
{
	return m_transforms.check(id);
}

const Transform& TransformSystem::getTransform(Id id) const
{
	return m_transforms.get(id);
}

Transform& TransformSystem::getTransform(Id id)
{
	return m_transforms.get(id);
}

void TransformSystem::setPosition(Id id, const vec3& position)
{
	m_transforms.get(id).position = position;
}

const vec3& TransformSystem::getPosition(Id id)
{
	return m_transforms.get(id).position;
}
