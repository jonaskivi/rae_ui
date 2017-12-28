#include <iostream>
#include <algorithm>
using namespace std;

#include "ObjectFactory.hpp"
#include "ComponentType.hpp"
#include "Entity.hpp"
#include "Transform.hpp"
#include "Mesh.hpp"
#include "Material.hpp"
#include "Hierarchy.hpp"

namespace rae
{

Id ObjectFactory::m_nextId = 0;

ObjectFactory::ObjectFactory()
{
}

ObjectFactory::~ObjectFactory()
{
}
// New system
//JONDE REMOVE
/*Id ObjectFactory::createEntity()
{
	Id id = getNextId();
	m_exists[id] = Exist();
	return id;
}

void ObjectFactory::createMesh(Id id)
{
	m_meshes2.emplace(id, Mesh(id));
}
*/

/*Mesh* ObjectFactory::getMesh2(Id id)
{
	return &m_meshes2[id];
	//return &m_meshes2.at(id);
	//return check(m_meshes2, id) ? &m_meshes2.at(id) : nullptr;
	//return check(m_meshes2, id) ? &m_meshes2[id] : nullptr;
}
*/
/*
void ObjectFactory::createMaterial(Id id, int type, const glm::vec4& color)
{
	m_materials2.emplace(id, Material((int)m_materials.size(), type, color));
}
*/

/*
Material* ObjectFactory::getMaterial2(Id id)
{
	return &m_materials2[id];
	//return &m_materials2.at(id);
	//return check(m_materials2, id) ? &m_materials2.at(id) : nullptr;
	//return check(m_materials2, id) ? &m_materials2[id] : nullptr;
}
*/

// Entities

Id ObjectFactory::createEmptyEntity()
{
	Id id = getNextId();
	m_entities.emplace_back(id);
	return id;
}

Id ObjectFactory::biggestId() const
{
	Id biggest = 0;
	for (Id id : m_entities)
	{
		if (id > biggest)
			biggest = id;
	}
	return biggest;
}

void ObjectFactory::destroyEntities(const Array<Id>& entities)
{
	for (Id id : entities)
	{
		m_entities.erase(std::remove(m_entities.begin(), m_entities.end(), id), m_entities.end());
	}
}

/*
void ObjectFactory::destroyEntity(Id id)
{
	if( index < 0 || index >= m_entities.size() )
		return;

	for(auto& componentIndex : m_entities[index].components())
	{
		switch( (ComponentType)componentIndex.type )
		{
			default:
				//cout << "ERROR: Strange type: " << componentIndex.type << "\n";
			break;
			case ComponentType::HIERARCHY:
			{
				if( componentIndex.id < m_hierarchies.size() )
				{
					m_hierarchies.erase(m_hierarchies.begin() + componentIndex.id);
					removedComponent(componentIndex);
				}
				else cout << "ERROR: Trying to remove hierarchy " << componentIndex.id << " but there are only " << m_hierarchies.size() << " hierarchies.\n";
			}
			break;
			case ComponentType::MATERIAL:
				//TODO shared components handling...
				//m_materials.erase(m_materials.begin() + componentIndex.id);
			break;
			case ComponentType::MESH:
				//m_meshes.erase(m_meshes.begin() + componentIndex.id);
			break;
		}
	}
	
	m_entities.erase(m_entities.begin() + index);
}
*/
// Meshes
/*
Mesh& ObjectFactory::createMesh()
{
	m_meshes.emplace_back( (int)m_meshes.size() );
	return m_meshes.back();
}

Material& ObjectFactory::createMaterial(int type, const glm::vec4& color)
{
	m_materials.emplace_back( (int)m_materials.size(), type, color );
	return m_materials.back();
}

Hierarchy& ObjectFactory::createHierarchy()
{
	m_hierarchies.emplace_back( (int)m_hierarchies.size() );
	return m_hierarchies.back();
}
*/

}

