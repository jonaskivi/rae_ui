#include "rae/scene/TransformSystem.hpp"

using namespace rae;

static const int ReserveTransforms = 1000;
static const int ReserveBoxes = 1000;

TransformSystem::TransformSystem() :
	ISystem("TransformSystem"),
	m_localTransforms(ReserveTransforms),
	m_transforms(ReserveTransforms),
	m_boxes(ReserveBoxes)
{
	addTable(m_localTransforms);
	addTable(m_transforms);

	addTable(m_parents);
	addTable(m_childrens);

	addTable(m_owners);
	addTable(m_owneds);

	addTable(m_boxes);
}

UpdateStatus TransformSystem::update()
{
	query<Changed>(m_parentChanged, [&](Id id)
	{
		if (m_localTransforms.check(id))
		{
			if (hasParent(id))
			{
				Id parentId = getParent(id);
				if (hasTransform(parentId))
				{
					const auto& parentTransform = getTransform(parentId);
					auto& transform = getTransformPrivate(id);
					transform.position = parentTransform.position + m_localTransforms.getF(id).position;
				}
			}
		}
	});

	m_parentChanged.clear();

	return UpdateStatus::NotChanged;
}

bool TransformSystem::hasAnyTransformChanged() const
{
	return m_transforms.isAnyUpdated();
}

void TransformSystem::processHierarchy(Id parentId, std::function<void(Id)> process)
{
	process(parentId);
	if (hasChildren(parentId))
	{
		auto& children = getChildren(parentId);
		for (auto&& id : children)
		{
			processHierarchy(id, process);
		}
	}
}

void TransformSystem::addTransform(Id id, Transform&& transform)
{
	Transform localTransform = transform;
	m_localTransforms.assign(id, std::move(localTransform));
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

Transform& TransformSystem::getTransformPrivate(Id id)
{
	return m_transforms.get(id);
}

void TransformSystem::setPosition(Id id, const vec3& position)
{
	m_transforms.assign(id, position);
	// Should measure if this is faster than assign or not: m_transforms.get(id).position = position;
}

const vec3& TransformSystem::getPosition(Id id)
{
	return m_transforms.get(id).position;
}

void TransformSystem::translate(Id id, vec3 delta)
{
	// Note: doesn't check if Id exists. Will crash/cause stuff if used unwisely.
	m_transforms.getF(id).position += delta;
	m_transforms.setUpdatedF(id);
}

void TransformSystem::translate(const Array<Id>& ids, vec3 delta)
{
	// RAE_TODO function: entitiesForTransform
	Array<Id> topLevelIds;
	for (auto&& id : ids)
	{
		bool topLevel = true;
		if (hasParent(id))
		{
			for (auto&& id2 : ids)
			{
				if (getParent(id) == id2)
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
		m_transforms.setUpdatedF(id);

		if (hasChildren(id))
		{
			auto& children = getChildren(id);
			for (auto&& childId : children)
			{
				translate(childId, delta);
			}
		}

	}
}

void TransformSystem::addChild(Id parent, Id child)
{
	if (!hasChildren(parent))
	{
		m_childrens.assign(parent, { child });
	}
	else
	{
		auto& childrenArray = m_childrens.getF(parent);
		// Check for duplicates
		for (auto&& id : childrenArray)
		{
			if (id == child)
			{
				LOG_F(ERROR, "Same child added multiple times to a parent.");
				assert(0);
				return;
			}
		}
		childrenArray.emplace_back(child);
	}

	if (!hasParent(child))
		m_parents.assign(child, std::move(Parent(parent)));

	m_childrenChanged.assign(parent, Changed());
	m_parentChanged.assign(child, Changed());
}

void TransformSystem::setParent(Id child, Id parent)
{
	addChild(parent, child);
}

bool TransformSystem::hasParent(Id id) const
{
	if (not m_parents.check(id))
		return false;
	return (m_parents.getF(id) != InvalidId);
}

Id TransformSystem::getParent(Id id) const
{
	return m_parents.get(id);
}

bool TransformSystem::hasChildren(Id id) const
{
	if (not m_childrens.check(id))
		return false;
	return (m_childrens.get(id).size() > 0);
}

const Array<Id>& TransformSystem::getChildren(Id id)
{
	return m_childrens.get(id);
}

void TransformSystem::addPivot(Id id, const Pivot& pivot)
{
	m_pivots.assign(id, Pivot(pivot));
}

bool TransformSystem::hasPivot(Id id) const
{
	return m_pivots.check(id);
}

const Pivot& TransformSystem::getPivot(Id id) const
{
	return m_pivots.get(id);
}

bool TransformSystem::hasBox(Id id) const
{
	return m_boxes.check(id);
}

void TransformSystem::addBox(Id id, Box&& box)
{
	m_boxes.assign(id, std::move(box));
}

void TransformSystem::setBox(Id id, Box&& box)
{
	m_boxes.assign(id, std::move(box));
}

void TransformSystem::setBox(Id id, const Box& box)
{
	m_boxes.assign(id, box);
}

const Box& TransformSystem::getBox(Id id) const
{
	return m_boxes.get(id);
}

bool TransformSystem::hasSphere(Id id) const
{
	return m_spheres.check(id);
}

void TransformSystem::addSphere(Id id)
{
	m_spheres.assign(id, std::move(Sphere()));
}

const Sphere& TransformSystem::getSphere(Id id) const
{
	return m_spheres.get(id);
}

Box TransformSystem::getAABBWorldSpace(Id id) const
{
	auto box = getBox(id);
	box.translate(getPivot(id));
	box.transform(getTransform(id));
	return box;
}
