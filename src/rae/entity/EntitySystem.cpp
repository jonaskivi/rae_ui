#include <algorithm>

#include "loguru/loguru.hpp"

#include "rae/entity/EntitySystem.hpp"

using namespace rae;

EntitySystem::EntitySystem(String owner) :
	m_owner(owner),
	m_nextId(InvalidId)
{
	LOG_F(INFO, "Init one EntitySystem for %s", owner.c_str());

	Id emptyEntityId = createEntity(); // hack at index 0
	LOG_F(INFO, "Create empty hack entity at id: %i", emptyEntityId);
}

EntitySystem::~EntitySystem()
{
}

Id EntitySystem::createEntity()
{
	LOG_F(INFO, "%s EntitySystem Creating entity.", m_owner.c_str());
	Id id = getNextId();
	LOG_F(INFO, "%s EntitySystem Creating entity: %i", m_owner.c_str(), (int)id);
	m_entities.emplace_back(id);
	return id;
}

bool EntitySystem::isAlive(Id id) const
{
	auto it = std::find(m_entities.begin(), m_entities.end(), id);
	if (it != m_entities.end())
	{
		return true;
	}
	return false;
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

Id EntitySystem::getNextId()
{
	LOG_F(INFO, "getNextId. Going to increment.");

	LOG_F(INFO, "getNextId. Going to inc: %i", (int)m_nextId);
	++m_nextId;
	LOG_F(INFO, "getNextId. Going to return: %i", (int)m_nextId);

	return m_nextId;
}
