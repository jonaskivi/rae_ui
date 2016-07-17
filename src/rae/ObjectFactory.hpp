#ifndef RAE_OBJECTFACTORY_HPP
#define RAE_OBJECTFACTORY_HPP

#include <vector>
#include <unordered_map>

#include <glm/glm.hpp>

#include "Hierarchy.hpp"

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
	Transform* getTransform(unsigned set_id) { return &m_transforms.at(set_id); }
	int transformCount() { return (int)m_transforms.size(); }

	// Meshes

	Mesh& createMesh();
	Mesh* getMesh(unsigned set_id) { return &m_meshes.at(set_id); }
	int meshCount() { return (int)m_meshes.size(); }

	Material& createMaterial(int type, const glm::vec4& color);
	Material* getMaterial(unsigned set_id) { return &m_materials.at(set_id); }
	int materialCount() { return (int)m_materials.size(); }

	std::vector<Material>& materials() { return m_materials; }

	Hierarchy& createHierarchy();
	Hierarchy* getHierarchy(unsigned set_id) { return &m_hierarchies.at(set_id); }
	int hierarchyCount() { return (int)m_hierarchies.size(); }

protected:
	std::vector<Entity>          m_entities;
	std::vector<Transform>       m_transforms;
	std::vector<Mesh>            m_meshes;
	std::vector<Material>        m_materials;
	std::vector<Hierarchy>       m_hierarchies;
};

}

#endif

