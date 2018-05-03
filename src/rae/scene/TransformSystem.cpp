#include "rae/scene/TransformSystem.hpp"

#include "rae/core/Time.hpp"

using namespace rae;

static const int ReserveTransforms = 1000;

TransformSystem::TransformSystem(const Time& time) :
	m_time(time),
	m_localTransforms(ReserveTransforms),
	m_localTransformChanged(ReserveTransforms),
	m_transforms(ReserveTransforms),
	m_transformChanged(ReserveTransforms)
{
	addTable(m_transforms);
}

UpdateStatus TransformSystem::update()
{
	query<Changed>(m_parentChanged, [&](Id id)
	{
		if (m_localTransforms.check(id))
		{
			if (hasHierarchy(id))
			{
				auto& hierarchy = getHierarchy(id);
				if (hasTransform(hierarchy.parent()))
				{
					const auto& parentTransform = getTransform(hierarchy.parent());
					auto& transform = getTransformPrivate(id);
					transform.position = parentTransform.position + m_localTransforms.getF(id).position;
				}
			}
		}
	});

	m_parentChanged.clear();

	return UpdateStatus::NotChanged;
}

void TransformSystem::addTransform(Id id, Transform&& transform)
{
	m_localTransforms.assign(id, Transform());
	m_transforms.assign(id, std::move(transform));
	//m_transformChanged.assign(id, Changed());
}

bool TransformSystem::hasTransform(Id id) const
{
	return m_transforms.check(id);
}

const Transform& TransformSystem::getTransform(Id id) const
{
	return m_transforms.get(id);
}

Transform& TransformSystem::getTransformPrivate(Id id)
{
	return m_transforms.get(id);
}

void TransformSystem::setPosition(Id id, const vec3& position)
{
	m_transforms.get(id).position = position;
	m_transformChanged.assign(id, Changed());
}

const vec3& TransformSystem::getPosition(Id id)
{
	return m_transforms.get(id).position;
}

void TransformSystem::translate(Id id, vec3 delta)
{
	// Note: doesn't check if Id exists. Will crash/cause stuff if used unwisely.
	m_transforms.getF(id).position += delta;
	m_transformChanged.assign(id, Changed());
}

void TransformSystem::translate(const Array<Id>& ids, vec3 delta)
{
	// RAE_TODO function: entitiesForTransform
	Array<Id> topLevelIds;
	for (auto&& id : ids)
	{
		bool topLevel = true;
		if (hasHierarchy(id))
		{
			auto& hierarchy = getHierarchy(id);
			for (auto&& id2 : ids)
			{
				if (hierarchy.parent() == id2)
				{
					topLevel = false;
					break;
				}
			}
		}

		if (topLevel)
		{
			topLevelIds.emplace_back(id);
		}
	}

	for (auto&& id : topLevelIds)
	{
		m_transforms.getF(id).position += delta;
		m_transformChanged.assign(id, Changed());


		if (hasHierarchy(id))
		{
			auto& hierarchy = getHierarchy(id);
			for (auto&& childId : hierarchy.children())
			{
				translate(childId, delta);
			}
		}
	}
}

void TransformSystem::addChild(Id parent, Id child)
{
	if (!hasHierarchy(parent))
		addHierarchy(parent, Hierarchy());

	if (!hasHierarchy(child))
		addHierarchy(child, Hierarchy());

	auto& hierarchy = getHierarchy(parent);
	hierarchy.addChild(child);

	auto& hierarchy2 = getHierarchy(child);
	hierarchy2.setParent(parent);

	m_childrenChanged.assign(parent, Changed());
	m_parentChanged.assign(child, Changed());
}

void TransformSystem::setParent(Id child, Id parent)
{
	addChild(parent, child);
}

void TransformSystem::addHierarchy(Id id, Hierarchy&& component)
{
	m_hierarchies.assign(id, std::move(component));
}

bool TransformSystem::hasHierarchy(Id id) const
{
	return m_hierarchies.check(id);
}

const Hierarchy& TransformSystem::getHierarchy(Id id) const
{
	return m_hierarchies.get(id);
}

Hierarchy& TransformSystem::getHierarchy(Id id)
{
	return m_hierarchies.get(id);
}
