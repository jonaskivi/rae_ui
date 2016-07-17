#ifndef RAE_3D_ENTITY_HPP
#define RAE_3D_ENTITY_HPP

#include <vector>

#include "Types.hpp"

namespace Rae
{

using Id = int;
using CompId = int;

struct ComponentIndex
{
public:
	ComponentIndex(int set_type, CompId set_id)
	: type(set_type),
	id(set_id)
	{
	}

	bool operator==(ComponentIndex& other)
	{
		if(other.type == type && other.id == id)
			return true;
		return false;
	}

	//Components are identified with 1. component type index, 2. component id index
	int type;
	CompId id;
};

class Entity
{
public:
	Id id() { return m_id; }
protected:
	void id(Id set) { m_id = set; }
	int m_id;

public:
	Entity(Id set_id);
	~Entity();

	void addComponent(int set_type, CompId set_id)
	{
		m_components.emplace_back(set_type, set_id);
	}

	//const 
	std::vector<ComponentIndex>& components() { return m_components; }

protected:	
	std::vector<ComponentIndex> m_components;
};

}//end namespace Rae

#endif
