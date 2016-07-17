#include <iostream>
using namespace std;

#include "ObjectFactory.hpp"
#include "ComponentType.hpp"
#include "Entity.hpp"
#include "Transform.hpp"
#include "Mesh.hpp"
#include "Material.hpp"
#include "Hierarchy.hpp"

namespace Rae
{

ObjectFactory::ObjectFactory()
{
	m_entities.  reserve( INITIAL_ENTITY_RESERVE    );
	m_transforms.reserve( INITIAL_TRANSFORM_RESERVE );
	m_meshes.    reserve( INITIAL_MESH_RESERVE      );
	m_hierarchies.reserve( INITIAL_ENTITY_RESERVE   );
}

ObjectFactory::~ObjectFactory()
{
	m_entities.clear();
	m_transforms.clear();
	m_meshes.clear();
	m_hierarchies.clear();
}

// Entities

Entity& ObjectFactory::createEmptyEntity()
{
	m_entities.emplace_back( m_entities.size() );
	return m_entities.back();
}

void ObjectFactory::destroyEntity(int index)
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
			case ComponentType::TRANSFORM:
			{
				if( componentIndex.id < m_transforms.size() )
				{
					m_transforms.erase(m_transforms.begin() + componentIndex.id);
					removedComponent(componentIndex);
				}
				else cout << "ERROR: Trying to remove transform " << componentIndex.id << " but there are only " << m_transforms.size() << " transforms.\n";
			}
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

void ObjectFactory::removedComponent(ComponentIndex componentIndex)
{
	// Fix id's over componentIndex.
	for(auto& entity : m_entities)
	{
		for(auto& compo : entity.components())
		{
			if(componentIndex.type == compo.type && componentIndex.id < compo.id)
			{
				compo.id = compo.id - 1;
			}
		}
	}
}

// Transforms

Transform& ObjectFactory::createTransform(float set_x, float set_y, float set_z)
{
	return createTransform(glm::vec3(set_x, set_y, set_z));
}

Transform& ObjectFactory::createTransform(const glm::vec3& position)
{
	size_t index = m_transforms.size();
	m_transforms.emplace_back( index, position );
	//m_transformsIndexMap[index] = index;
	return m_transforms.back();
}

// Meshes

Mesh& ObjectFactory::createMesh()
{
	m_meshes.emplace_back( m_meshes.size() );
	return m_meshes.back();
}

Material& ObjectFactory::createMaterial(int type, const glm::vec4& color)
{
	m_materials.emplace_back( m_materials.size(), type, color );
	return m_materials.back();
}

Hierarchy& ObjectFactory::createHierarchy()
{
	m_hierarchies.emplace_back( (int)m_hierarchies.size() );
	return m_hierarchies.back();
}

}

