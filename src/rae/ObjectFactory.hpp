#ifndef RAE_OBJECTFACTORY_HPP
#define RAE_OBJECTFACTORY_HPP

#include <vector>

#include <GL/glew.h> // JONDE TEMP for measure
#include <GLFW/glfw3.h> // JONDE TEMP for measure

#include <glm/glm.hpp>
using glm::vec3;
using glm::vec4;

#include "Hierarchy.hpp"
#include "Types.hpp"

namespace Rae
{

struct ComponentIndex;
class Entity;
class Mesh;
class Material;
class Transform;
//class Hierarchy;

const unsigned INITIAL_ENTITY_RESERVE = 20;
const unsigned INITIAL_TRANSFORM_RESERVE = 20;
const unsigned INITIAL_MESH_RESERVE = 5;

class ObjectFactory
{
public:
	ObjectFactory();
	~ObjectFactory();

	void measure();

	std::pair<double, float> measureOld(int& outCount);
	std::pair<double, float> measureNew(int& outCount);

	float renderIterateOld(int& outCount);
	float renderIterateNew(int& outCount);

	// New system

	Id createEntity();
	void createTransform(Id id, const vec3& setPosition);
	void createMesh(Id id);
	void createMaterial(Id id, int type, const glm::vec4& color);

	/*Transform*	getTransform2(Id id);
	Material*	getMaterial2(Id id);
	Mesh*		getMesh2(Id id);
	*/

	Transform* getTransform2(Id set_id) { return &m_transforms2.at(set_id); }
	Mesh* getMesh2(Id set_id) { return &m_meshes2.at(set_id); }
	Material* getMaterial2(Id set_id) { return &m_materials2.at(set_id); }

	// Entities

	Entity& createEmptyEntity();
	void destroyEntity(int index);
	int entityCount() { return (int)m_entities.size(); }

	std::vector<Entity>& entities() { return m_entities; }

	// Components

	void removedComponent(ComponentIndex componentIndex);

	// Transforms

	Transform& createTransform(float set_x, float set_y, float set_z);
	Transform& createTransform(const glm::vec3& position);
	//Transform* getTransform(unsigned set_id) { return &m_transforms.at(set_id); } // JONDE test speed
	Transform* getTransform(unsigned set_id) { return &m_transforms[set_id]; }
	int transformCount() { return (int)m_transforms.size(); }

	// Meshes

	Mesh& createMesh();
	//Mesh* getMesh(unsigned set_id) { return &m_meshes.at(set_id); } //JONDE test speed
	Mesh* getMesh(unsigned set_id) { return &m_meshes[set_id]; }
	int meshCount() { return (int)m_meshes.size(); }

	Material& createMaterial(int type, const glm::vec4& color);
	//Material* getMaterial(unsigned set_id) { return &m_materials.at(set_id); } // JONDE test speed
	Material* getMaterial(unsigned set_id) { return &m_materials[set_id]; }
	int materialCount() { return (int)m_materials.size(); }

	std::vector<Material>& materials() { return m_materials; }

	Hierarchy& createHierarchy();
	//Hierarchy* getHierarchy(unsigned set_id) { return &m_hierarchies.at(set_id); } // JONDE test speed
	Hierarchy* getHierarchy(unsigned set_id) { return &m_hierarchies[set_id]; }
	int hierarchyCount() { return (int)m_hierarchies.size(); }

	Id getNextId() { return ++m_nextId; }

protected:
	Array<Entity>          m_entities;
	Array<Transform>       m_transforms;
	Array<Mesh>            m_meshes;
	Array<Material>        m_materials;
	Array<Hierarchy>       m_hierarchies;

	static Id			m_nextId;
	Map<Id, Exist>		m_exists;
	Map<Id, Transform>	m_transforms2;
	Map<Id, Mesh>		m_meshes2;
	Map<Id, Material>	m_materials2;
};

}

#endif

