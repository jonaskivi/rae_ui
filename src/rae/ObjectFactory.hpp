#ifndef RAE_OBJECTFACTORY_HPP
#define RAE_OBJECTFACTORY_HPP

#include <vector>

#include <GL/glew.h> // JONDE TEMP for measure
#include <GLFW/glfw3.h> // JONDE TEMP for measure

#include <glm/glm.hpp>
using glm::vec3;
using glm::vec4;

#include "Hierarchy.hpp"
#include "rae/core/Types.hpp"

#include "rae/entity/Table.hpp"
#include "Transform.hpp"
#include "Mesh.hpp"
#include "Material.hpp"

namespace rae
{

//JONDE REMOVE struct ComponentIndex;
//JONDE REMOVE class Entity;
class Mesh;
class Material;
class Transform;
//class Hierarchy;

const unsigned INITIAL_ENTITY_RESERVE = 20;
const unsigned INITIAL_TRANSFORM_RESERVE = 20;
const unsigned INITIAL_MESH_RESERVE = 5;

struct Entity
{
};

// JONDE RENAME to EntitySystem and only care about entity lifetime. Pooling of Ids.
class ObjectFactory
{
public:
	ObjectFactory();
	~ObjectFactory();

//JONDE REMOVE 	void measure();

//JONDE REMOVE 	std::pair<double, float> measureOld(int& outCount);
//JONDE REMOVE 	std::pair<double, float> measureNew(int& outCount);
//JONDE REMOVE 	std::pair<double, float> measureNew3(int& outCount);

//JONDE REMOVE 	float renderIterateOld(int& outCount);
//JONDE REMOVE 	float renderIterateNew(int& outCount);
//JONDE REMOVE 	float renderIterateNew3(int& outCount);

	// New system

	Id createEntity();
	//void createMesh(Id id);
	//void createMaterial(Id id, int type, const glm::vec4& color);

	//JONDE REMOVE Mesh* getMesh2(Id set_id) { return &m_meshes2.at(set_id); }
	//JONDE REMOVE Material* getMaterial2(Id set_id) { return &m_materials2.at(set_id); }

	// Entities

	Id createEmptyEntity();
	//JONDE REMOVE Entity& createEmptyEntity3();
	//void destroyEntity(int index);
	int entityCount() { return (int)m_entities.size(); }
	const Array<Id>& entities() { return m_entities; }

	//TODO move to VisualSystem:
	// Meshes
//JONDE REMOVE 	Mesh& createMesh();
	//Mesh* getMesh(unsigned set_id) { return &m_meshes.at(set_id); } //JONDE test speed
//JONDE REMOVE 	Mesh* getMesh(unsigned set_id) { return &m_meshes[set_id]; }
//JONDE REMOVE 	int meshCount() { return (int)m_meshes.size(); }

//JONDE REMOVE 	Material& createMaterial(int type, const glm::vec4& color);
	//Material* getMaterial(unsigned set_id) { return &m_materials.at(set_id); } // JONDE test speed
//JONDE REMOVE 	Material* getMaterial(unsigned set_id) { return &m_materials[set_id]; }
//JONDE REMOVE 	int materialCount() { return (int)m_materials.size(); }

//JONDE REMOVE 	std::vector<Material>& materials() { return m_materials; }

//JONDE REMOVE 	Hierarchy& createHierarchy();
	//Hierarchy* getHierarchy(unsigned set_id) { return &m_hierarchies.at(set_id); } // JONDE test speed
//JONDE REMOVE 	Hierarchy* getHierarchy(unsigned set_id) { return &m_hierarchies[set_id]; }
//JONDE REMOVE 	int hierarchyCount() { return (int)m_hierarchies.size(); }

	Id getNextId() { return ++m_nextId; }

protected:
	/* JONDE REMOVE:
	Array<Entity>          m_entities;
	Array<Mesh>            m_meshes;
	Array<Material>        m_materials;
	Array<Hierarchy>       m_hierarchies;

	Map<Id, Exist>		m_exists;
	Map<Id, Mesh>		m_meshes2;
	Map<Id, Material>	m_materials2;

	Array<Entity>		m_entities3;
	*/

	static Id			m_nextId;

	Array<Id>			m_entities;
	//Table<Entity>		m_entities;
	//JONDE MOVE TO VisualSystem:
	//Table<Transform>	m_transforms;
};

}

#endif

