#include "rae/visual/TransformSystem.hpp"

#include "rae/core/Time.hpp"

using namespace rae;

static const int ReserveTransforms = 1000;

TransformSystem::TransformSystem(const Time& time) :
	m_time(time),
	m_transforms(ReserveTransforms)
{
	addTable(m_transforms);
}

UpdateStatus TransformSystem::update()
{
	for (auto&& transform : m_transforms.items())
	{
		transform.update(m_time.time());
	}
	return UpdateStatus::NotChanged;
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

void TransformSystem::translate(Id id, vec3 delta)
{
	// Note: doesn't check if Id exists. Will crash/cause stuff if used unwisely.
	m_transforms.getF(id).position += delta; 
}
