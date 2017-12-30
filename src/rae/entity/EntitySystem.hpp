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
	EntitySystem();
	~EntitySystem();

	Id createEntity();
	void destroyEntities(const Array<Id>& entities);

	Id biggestId() const;
	int entityCount() const { return (int)m_entities.size(); }
	const Array<Id>& entities() const { return m_entities; }

	Id getNextId() { return ++m_nextId; }

protected:
	static Id			m_nextId;

	Array<Id>			m_entities;
};

}
