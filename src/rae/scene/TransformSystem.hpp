#pragma once

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

struct Sphere
{
};

class TransformSystem : public ISystem
{
public:
	TransformSystem();

	UpdateStatus update() override;

	void syncLocalAndWorldTransforms();

	bool hasAnyTransformChanged() const;

	// Process the while hierarchy including the parentId itself.
	void processHierarchy(Id parentId, std::function<void(Id)> process);
	// Process the hierarchy. If process returns false, we don't process it's children,
	// but still continue to process it's siblings and their children.
	void processHierarchySkippable(Id parentId, std::function<bool(Id)> process);
	// Process only direct children, so doesn't include the parentId.
	void processChildren(Id parentId, std::function<void(Id)> process);
	// Process the siblings of the given entity (they have the same parent). Also process entity itself.
	void processSiblingsInclusive(Id id, std::function<void(Id)> process);
	// Process the siblings of the given entity (they have the same parent). Don't process the entity.
	void processSiblingsExclusive(Id id, std::function<void(Id)> process);

	void addTransform(Id id, const Transform& transform);

	bool hasLocalTransform(Id id) const;
	const Transform& getLocalTransform(Id id) const;

	void setLocalPosition(Id id, const vec3& position);
	const vec3& getLocalPosition(Id id);
	void setLocalRotation(Id id, const qua& rotation);
	const qua& getLocalRotation(Id id);
	void setLocalScale(Id id, const vec3& scale);
	const vec3& getLocalScale(Id id);

	bool hasWorldTransform(Id id) const;
	const Transform& getWorldTransform(Id id) const;

	void setWorldPosition(Id id, const vec3& position);
	const vec3& getWorldPosition(Id id);
	void setWorldRotation(Id id, const qua& rotation);
	const qua& getWorldRotation(Id id);
	void setWorldScale(Id id, const vec3& scale);
	const vec3& getWorldScale(Id id);

	int transformCount() const { return m_localTransforms.size(); }
	const Table<Transform>& localTransforms() const { return m_localTransforms; }
	const Table<Transform>& worldTransforms() const { return m_worldTransforms; }

	// Get only the toplevel ids. So if there's parent child relationships, then only return the parents.
	Array<Id> entitiesForTransform(const Array<Id>& ids) const;

	void translate(Id id, const vec3& delta);
	void translate(const Array<Id>& ids, const vec3& delta);

	void rotate(Id id, const qua& delta);
	void rotate(const Array<Id>& ids, const qua& delta);
	void rotateAround(const Array<Id>& ids, const qua& delta, const vec3& pivot);

	void addChild(Id parent, Id child); // Does not fix existing hierarchy
	void setParent(Id child, Id parent); // Does not fix existing hierarchy

	//void addParent(Id id);
	bool hasParent(Id id) const;
	Id getParent(Id id) const;

	//void addChildren(Id id);
	bool hasChildren(Id id) const;
	const Array<Id>& getChildren(Id id) const;
	void moveToTop(Id parentId, Id childIdToMove);
	void moveToTopInParent(Id childIdToMove);
	void moveToBottom(Id parentId, Id childIdToMove);
	void moveToBottomInParent(Id childIdToMove);

	// Pivot is defined in normalized coordinates -1 to 1 (in relation to own size)
	// and it will affect how the origin of the current entity will be interpreted.
	bool hasPivot(Id id) const;
	void addPivot(Id id, const Pivot& pivot);
	void setPivot(Id id, const Pivot& pivot);
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
	void addBox(Id id, const Box& box);
	void setBox(Id id, const Box& box);
	const Box& getBox(Id id) const;
	Box& modifyBox(Id id);

	const Table<Sphere>& spheres() const { return m_spheres; }
	bool hasSphere(Id id) const;
	void addSphere(Id id);
	const Sphere& getSphere(Id id) const;

	Box getAABBWorldSpace(Id id) const;

	String toString(Id id) const;

private:

	Transform& modifyLocalTransform(Id id);
	Transform& modifyWorldTransform(Id id);

	// Local transforms. Relative to parents.
	Table<Transform>	m_localTransforms;
	// World transform. The final coordinates to draw and hittest with.
	Table<Transform>	m_worldTransforms;
	bool				m_anyTransformUpdated = true;

	Table<Parent>		m_parents;
	Table<Changed>		m_parentChanged; // These are most likely not used properly. Also see Changed tag inside Table class.

	Table<Children>		m_childrens;
	Table<Changed>		m_childrenChanged;

	Table<Owner>		m_owners;
	Table<Owned>		m_owneds;

	// Pivot defines the relation between the localTransform.position and the Box.
	// By default if no Pivot is set, you get the default pivot which is vec(0,0,0).
	// The pivot is relational to the Box size, and its typical values run from
	// (-1,-1,-1) to (1,1,1), which mark the opposite corners of the Box. Y-up or Y-down in 3D or 2D
	// of course affect which corner is which. TopLeft2D corner is defined in Types.cpp as vec(-1,-1,0).
	Table<Pivot>		m_pivots;

	Table<Box>			m_boxes;
	// This is just an additional component to recognize spheres for RayTracer. They can also have a Box component.
	Table<Sphere>		m_spheres;
};

}
