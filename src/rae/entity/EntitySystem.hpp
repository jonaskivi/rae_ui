#pragma once

#include "rae/core/Types.hpp"

namespace rae
{

struct Entity
{
};

/// Create and destroy entities and handle their lifetimes. TODO Pooling of Ids.
class EntitySystem
{
public:
	EntitySystem(String owner);
	~EntitySystem();

	EntitySystem(EntitySystem&&) = default;

	Id createEntity();
	void destroyEntities(const Array<Id>& entities);

	bool isAlive(Id id) const;

	Id biggestId() const;
	int entityCount() const { return (int)m_entities.size(); }
	const Array<Id>& entities() const { return m_entities; }

	const String& owner() { return m_owner; }

	Id getNextId();

protected:
	String		m_owner = "";

	Id			m_nextId = 0;

	Array<Id>	m_entities;
};

}
