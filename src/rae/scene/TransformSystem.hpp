#pragma once

#include <vector>

#include "rae/core/Types.hpp"
#include "rae/entity/Table.hpp"

#include "rae/entity/Hierarchy.hpp"
#include "rae/scene/Transform.hpp"
#include "rae/core/ISystem.hpp"

namespace rae
{

class Time;
class Input;

class TransformSystem : public ISystem
{
public:
	TransformSystem(const Time& time);

	String name() override { return "TransformSystem"; }

	UpdateStatus update() override;

	void addTransform(Id id, Transform&& transform);
	bool hasTransform(Id id) const;
	const Transform& getTransform(Id id) const;
	Transform& getTransform(Id id);

	void setPosition(Id id, const vec3& position);
	const vec3& getPosition(Id id);

	int transformCount() { return m_transforms.size(); }

	void translate(Id id, vec3 delta);
	void translate(const Array<Id>& ids, vec3 delta);

	void addChild(Id parent, Id child); // Does not fix existing hierarchy
	void setParent(Id child, Id parent); // Does not fix existing hierarchy
	void addHierarchy(Id id, Hierarchy&& component);
	bool hasHierarchy(Id id) const;
	const Hierarchy& getHierarchy(Id id) const;
	Hierarchy& getHierarchy(Id id);

private:
	const Time& m_time;

	Table<Transform> m_transforms;
	Table<Hierarchy> m_hierarchies;
};

}