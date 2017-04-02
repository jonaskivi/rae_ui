#include <iostream>
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
: m_transforms3(INITIAL_TRANSFORM_RESERVE),
  m_meshes3(INITIAL_MESH_RESERVE)
{
	m_entities.  reserve( INITIAL_ENTITY_RESERVE    );
	m_transforms.reserve( INITIAL_TRANSFORM_RESERVE );
	m_meshes.    reserve( INITIAL_MESH_RESERVE      );
	m_hierarchies.reserve( INITIAL_ENTITY_RESERVE   );

	m_exists.reserve     (INITIAL_ENTITY_RESERVE);
	m_transforms2.reserve(INITIAL_TRANSFORM_RESERVE);
	m_meshes2.reserve    (INITIAL_MESH_RESERVE);
}

ObjectFactory::~ObjectFactory()
{
	m_entities.clear();
	m_transforms.clear();
	m_meshes.clear();
	m_hierarchies.clear();
}

void ObjectFactory::measure()
{
	int countOld = 0;
	auto old = measureOld(countOld);

	int countNew = 0;
	auto news = measureNew(countNew);

	int countNew3 = 0;
	auto news3 = measureNew3(countNew3);

	std::cout << "Old took: " << old.first
		<< " average: " << old.second
		<< " old ents: " << m_entities.size()
		<< " transforms: " << m_transforms.size()
		<< " materials: " << m_materials.size()
		<< " meshes: " << m_meshes.size()
		<< " count: " << countOld
		<< "\n";
	std::cout << "New took: " << news.first
		<< " average: " << news.second
		<< " new ents: " << m_exists.size()
		<< " transforms: " << m_transforms2.size()
		<< " materials: " << m_materials2.size()
		<< " meshes: " << m_meshes2.size()
		<< " count: " << countNew
		<< "\n";

	std::cout << "New3 took: " << news3.first
		<< " average: " << news3.second
		//<< " new ents: " << m_exists.size()
		<< " transforms: " << m_transforms3.items().size()
		<< " materials: " << m_materials3.items().size()
		<< " meshes: " << m_meshes3.items().size()
		<< " count: " << countNew3
		<< "\n";
}

const int ents = 10000;
const int times = 2000;

std::pair<double, float> ObjectFactory::measureOld(int& outCount)
{
	double startTime = glfwGetTime();

	// Old
	for (int i = 0; i < ents; ++i)
	{
		Entity& entity = createEmptyEntity();
		entity.addComponent( (int)ComponentType::TRANSFORM, createTransform(vec3(52.0f + float(i), 14.0f, 30.0f + float(i))).id() );
		entity.addComponent( (int)ComponentType::MATERIAL, createMaterial(0, vec4(1.0f, 0.5f, 0.1f, 1.0f)).id() );
		
		Mesh& mesh = createMesh();
		mesh.generateBox();
		entity.addComponent( (int)ComponentType::MESH, mesh.id() );
	}

	float average = 0.0f;
	for (int i = 0; i < times; ++i)
	{
		average += renderIterateOld(outCount);
	}

	double endTime = glfwGetTime();

	return std::make_pair(endTime - startTime, average / times);
	//std::cout << "Old took: " << endTime - startTime << "\n";
}

std::pair<double, float> ObjectFactory::measureNew(int& outCount)
{
	double startTime = glfwGetTime();

	// New
	for (int i = 0; i < ents; ++i)
	{
		Id id = createEntity();
		createTransform(id, vec3(52.0f + float(i), 14.0f, 30.0f + float(i)));
		createMaterial(id, 0, vec4(1.0f, 0.5f, 0.1f, 1.0f));
		
		//Mesh& mesh = createMesh();
		//mesh.generateBox();
		createMesh(id);
		Mesh* mesh = getMesh2(id);
		if (mesh)
			mesh->generateBox();
	}

	float average = 0.0f;
	for (int i = 0; i < times; ++i)
	{
		average += renderIterateNew(outCount);
	}

	double endTime = glfwGetTime();

	return std::make_pair(endTime - startTime, average / times);
	//std::cout << "New took: " << endTime - startTime << "\n";
}

std::pair<double, float> ObjectFactory::measureNew3(int& outCount)
{
	double startTime = glfwGetTime();

	for (int i = 0; i < ents; ++i)
	{
		Entity& entity = createEmptyEntity3();
		m_transforms3.create(entity.id(), Transform(vec3(52.0f + float(i), 14.0f, 30.0f + float(i))));
		m_materials3.create(entity.id(), Material(/*id:*/0, /*type:*/0, vec4(1.0f, 0.5f, 0.1f, 1.0f)));

		Mesh mesh = Mesh(0);
		mesh.generateBox();
		m_meshes3.create(entity.id(), std::move(mesh));
	}

	float average = 0.0f;
	for (int i = 0; i < times; ++i)
	{
		average += renderIterateNew3(outCount);
	}

	double endTime = glfwGetTime();

	return std::make_pair(endTime - startTime, average / times);
	//std::cout << "Old took: " << endTime - startTime << "\n";
}

float ObjectFactory::renderIterateOld(int& outCount)
{
	vec3 average;

	for (auto& entity : m_entities)
	{
		Transform* transform = nullptr;
		Material*  material  = nullptr;
		Mesh*      mesh      = nullptr;
		
		for (auto& componentIndex : entity.components())
		{
			switch( (ComponentType)componentIndex.type )
			{
				default:
					//cout << "ERROR: Strange type: " << componentIndex.type << "\n";
				break;
				case ComponentType::TRANSFORM:
					if(transform == nullptr)
						transform = getTransform(componentIndex.id);
					else cout << "ERROR: Found another transform component. id: " << componentIndex.id << "\n";
				break;
				case ComponentType::MATERIAL:
					if(material == nullptr)
					{
						material = getMaterial(componentIndex.id);
					}
					else cout << "ERROR: Found another material component. id: " << componentIndex.id << "\n";
				break;
				case ComponentType::MESH:
					if(mesh == nullptr)
					{
						mesh = getMesh(componentIndex.id);
					}
					else cout << "ERROR: Found another mesh component. id: " << componentIndex.id << "\n";
				break;
			}
		}

		if (transform)
		{
			outCount++;
			average += transform->position;
		}

		/*if( transform && mesh )
		{
			#ifdef RAE_DEBUG
				cout << "Going to render Mesh. id: " << mesh->id() << "\n";
			#endif

			// Update animation... TODO move this elsewhere.
			////transform->update(time, delta_time);

			////renderMesh(transform, material, mesh);
		}*/
		//else cout << "No mesh and no transform.\n";
	}

	float aveX = average.x / (float)m_entities.size();
	float aveY = average.y / (float)m_entities.size();
	float aveZ = average.z / (float)m_entities.size();

	return aveX;
	//std::cout << "Old average: " << aveX << ", " << aveY << ", " << aveZ << "\n";
}

float ObjectFactory::renderIterateNew(int& outCount)
{
	vec3 average;

	Id id = 0;
	for (const auto& exist : m_exists)
	{
		Id id = exist.first;

		Transform* transform = getTransform2(id);
		Material*  material  = getMaterial2(id);
		Mesh*      mesh      = getMesh2(id);

		if (transform)
		{
			outCount++;
			average += transform->position;

			id++;
		}
	}

	float aveX = average.x / (float)m_exists.size();
	float aveY = average.y / (float)m_exists.size();
	float aveZ = average.z / (float)m_exists.size();

	return aveX;
	//std::cout << "New average: " << aveX << ", " << aveY << ", " << aveZ << "\n";
}

float ObjectFactory::renderIterateNew3(int& outCount)
{
	vec3 average;

	for (auto& entity : m_entities)
	{
		Transform& transform = m_transforms3.get(entity.id());
		Material&  material  = m_materials3.get(entity.id());
		Mesh&      mesh      = m_meshes3.get(entity.id());

		//if (transform)
		//{
			outCount++;
			average += transform.position;
		//}

		/*
		Transform* transform = m_transforms3.get(entity.id());
		Material*  material  = m_materials3.get(entity.id());
		Mesh*      mesh      = m_meshes3.get(entity.id());

		if (transform)
		{
			outCount++;
			average += transform->position;
		}
		*/

		/*if( transform && mesh )
		{
			#ifdef RAE_DEBUG
				cout << "Going to render Mesh. id: " << mesh->id() << "\n";
			#endif

			// Update animation... TODO move this elsewhere.
			////transform->update(time, delta_time);

			////renderMesh(transform, material, mesh);
		}*/
		//else cout << "No mesh and no transform.\n";
	}

	float aveX = average.x / (float)m_entities.size();
	float aveY = average.y / (float)m_entities.size();
	float aveZ = average.z / (float)m_entities.size();

	return aveX;
	//std::cout << "Old average: " << aveX << ", " << aveY << ", " << aveZ << "\n";
}

// New system
Id ObjectFactory::createEntity()
{
	Id id = getNextId();
	m_exists[id] = Exist();
	return id;
}

void ObjectFactory::createTransform(Id id, const vec3& setPosition)
{
	m_transforms2.emplace(id, Transform(id, setPosition));

	/*m_transforms2.emplace(std::piecewise_construct,
		std::forward_as_tuple(id),
		std::forward_as_tuple(id, setPosition));*/

	//auto trans = Transform(id, setPosition);
	//m_transforms2[id] = trans;
	///////m_transforms2[id] = Transform(id, setPosition);
}

/*Transform* ObjectFactory::getTransform2(Id id)
{
	return &m_transforms2[id];
	//return &m_transforms2.at(id);
	//return check(m_transforms2, id) ? &m_transforms2.at(id) : nullptr;
	//return check(m_transforms2, id) ? &m_transforms2[id] : nullptr;
}
*/
void ObjectFactory::createMesh(Id id)
{
	m_meshes2.emplace(id, Mesh(id));
}

/*Mesh* ObjectFactory::getMesh2(Id id)
{
	return &m_meshes2[id];
	//return &m_meshes2.at(id);
	//return check(m_meshes2, id) ? &m_meshes2.at(id) : nullptr;
	//return check(m_meshes2, id) ? &m_meshes2[id] : nullptr;
}
*/
void ObjectFactory::createMaterial(Id id, int type, const glm::vec4& color)
{
	m_materials2.emplace(id, Material((int)m_materials.size(), type, color));
}

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

Entity& ObjectFactory::createEmptyEntity()
{
	m_entities.emplace_back( static_cast<rae::Id>(m_entities.size()) );

	return m_entities.back();
}

Entity& ObjectFactory::createEmptyEntity3()
{
	m_entities3.emplace_back( static_cast<rae::Id>(m_entities3.size()) );

	return m_entities3.back();
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
	m_transforms.emplace_back( (int)index, position );
	//m_transformsIndexMap[index] = index;
	return m_transforms.back();
}

// Meshes

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

}

