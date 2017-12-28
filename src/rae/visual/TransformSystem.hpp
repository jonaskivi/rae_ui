#pragma once

#include <vector>

#include "rae/core/Types.hpp"
#include "rae/entity/Table.hpp"

#include "Transform.hpp"
#include "rae/core/ISystem.hpp"

namespace rae
{

class Input;

class TransformSystem : public ISystem
{
public:
	TransformSystem();

	String name() override { return "TransformSystem"; }

	bool update(double time, double deltaTime) override;
	void destroyEntities(const Array<Id>& entities) override;

	void addTransform(Id id, Transform&& transform);
	bool hasTransform(Id id) const;
	const Transform& getTransform(Id id) const;
	Transform& getTransform(Id id);

	void setPosition(Id id, const vec3& position);
	const vec3& getPosition(Id id);

	int transformCount() { return m_transforms.size(); }

private:

	Table<Transform> m_transforms;

	//Array<Id> m_changed; // JONDE Could be worth to test something like this. But then again, we probably need more
	// fine grained changed events. And maybe encapsulate them inside events anyway.
};

}
