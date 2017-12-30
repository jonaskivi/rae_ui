#include <algorithm>

#include "rae/entity/EntitySystem.hpp"

using namespace rae;

Id EntitySystem::m_nextId = 0;

EntitySystem::EntitySystem()
{
}

EntitySystem::~EntitySystem()
{
}

Id EntitySystem::createEntity()
{
	Id id = getNextId();
	m_entities.emplace_back(id);
	return id;
}

void EntitySystem::destroyEntities(const Array<Id>& entities)
{
	for (Id id : entities)
	{
		m_entities.erase(std::remove(m_entities.begin(), m_entities.end(), id), m_entities.end());
	}
}

Id EntitySystem::biggestId() const
{
	Id biggest = 0;
	for (Id id : m_entities)
	{
		if (id > biggest)
			biggest = id;
	}
	return biggest;
}
