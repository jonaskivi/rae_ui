#pragma once

#include <vector>
#include "Entity.hpp"

#include "rae/core/Types.hpp"
#include "rae/entity/Table.hpp"

#include "Transform.hpp"
#include "System.hpp"

namespace rae
{

class Input;

class TransformSystem : public System
{
public:
	TransformSystem();

	bool update(double time, double delta_time, std::vector<Entity>& entities) override;

	void addTransform(Id id, Transform&& transform);

	void setPosition(Id id, const vec3& position);
	const vec3& getPosition(Id id);

private:

	Table<Transform> m_transforms;

	//Array<Id> m_changed; // JONDE Could be worth to test something like this. But then again, we probably need more
	// fine grained changed events. And maybe encapsulate them inside events anyway.
};

}
