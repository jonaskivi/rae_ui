#pragma once

#include <vector>

#include "rae/core/Types.hpp"
#include "rae/entity/Table.hpp"

#include "rae/entity/Hierarchy.hpp"
#include "rae/scene/Transform.hpp"
#include "rae/core/ISystem.hpp"
#include "rae/visual/Box.hpp"

namespace rae
{

struct Changed
{
};

// The primary hierarchy is done with Parent and Children components. Primary hierarchy usually means that
// the parent transforms are propagated to the children. The localTransform is the childs transform in relation
// to its parent. In UI the Children are usually part of the Parents Layout.
using Parent = Id;
using Children = Array<Id>;

// A secondary hierarchy, which usually contains things like KeyLines, which are not part of the UI layout, but
// the transforms can still be propagated, and the localTransform is in relation to its parent. We just might
// skip the layouting part, so that the KeyLine itself can affect the layout, but not be affected by it.
using Owner = Id;
using Owned = Array<Id>;

class TransformSystem : public ISystem
{
public:
	TransformSystem();

	UpdateStatus update() override;

	//UpdateStatus updateSceneGraph();

	void processHierarchy(Id parentId, std::function<void(Id)> process);

	void addTransform(Id id, Transform&& transform);
	bool hasTransform(Id id) const;
	const Transform& getTransform(Id id) const;

	void setPosition(Id id, const vec3& position);
	const vec3& getPosition(Id id);

	int transformCount() { return m_transforms.size(); }
	const Table<Transform>& transforms() const { return m_transforms; }

	void translate(Id id, vec3 delta);
	void translate(const Array<Id>& ids, vec3 delta);

	void addChild(Id parent, Id child); // Does not fix existing hierarchy
	void setParent(Id child, Id parent); // Does not fix existing hierarchy

	//void addParent(Id id);
	bool hasParent(Id id) const;
	Id getParent(Id id) const;

	//void addChildren(Id id);
	bool hasChildren(Id id) const;
	const Array<Id>& getChildren(Id id);

	// Pivot is defined in normalized coordinates -1 to 1 (in relation to own size)
	// and it will affect how the origin of the current entity will be interpreted.
	void addPivot(Id id, const vec3& pivot);
	bool hasPivot(Id id) const;
	const Pivot& getPivot(Id id) const;

	// RAE_TODO add functions to get full 2D transform
	/*
	struct Transform2D
	{
		Transform
		Pivot
		Box
	}?
	*/
	const Table<Box>& boxes() const { return m_boxes; }
	bool hasBox(Id id) const;
	void addBox(Id id, Box&& box);
	const Box& getBox(Id id) const;

	Box getAABBWorldSpace(Id id) const;

private:

	Transform& getTransformPrivate(Id id);

	Table<Transform>	m_localTransforms;
	Table<Changed>		m_localTransformChanged;

	Table<Transform>	m_transforms;
	Table<Changed>		m_transformChanged;

	Table<Parent>		m_parents;
	Table<Changed>		m_parentChanged;

	Table<Children>		m_childrens;
	Table<Changed>		m_childrenChanged;

	Table<Owner>		m_owners;
	Table<Owned>		m_owneds;

	Table<Pivot>		m_pivots;

	Table<Box>			m_boxes;
};

}
