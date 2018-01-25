#include "rae/asset/AssetSystem.hpp"

#include "rae/core/Time.hpp"
#include "rae/entity/EntitySystem.hpp"

#include "rae/visual/Mesh.hpp"

using namespace rae;

AssetSystem::AssetSystem(Time& time, EntitySystem& entitySystem) :
	m_time(time),
	m_entitySystem(entitySystem)
{
	addTable(m_meshes);
	addTable(m_materials);
}

void AssetSystem::setNanoVG(NVGcontext* nanoVG)
{
	m_nanoVG = nanoVG;
}


UpdateStatus AssetSystem::update()
{
	for (auto&& material : m_materials.items())
	{
		material.update(m_nanoVG, m_time.time());
	}

	return UpdateStatus::NotChanged;
}

Id AssetSystem::createMesh(const String& filename)
{
	Id id = m_entitySystem.createEntity();
	//rae_log("createMesh entity: ", id);
	Mesh mesh;
	addMesh(id, std::move(mesh));

	Mesh& mesh2 = getMesh(id);
	mesh2.loadModel(filename);
	return id;
}

Id AssetSystem::createMaterial(const Color& color)
{
	Id id = m_entitySystem.createEntity();
	//rae_log("createMaterial entity: ", id);
	Material material(color);
	addMaterial(id, std::move(material));

	Material& material2 = getMaterial(id);
	material2.generateFBO(m_nanoVG);
	return id;
}

Id AssetSystem::createAnimatingMaterial(const Color& color)
{
	Id id = m_entitySystem.createEntity();
	//rae_log("createAnimatingMaterial entity: ", id);
	Material material(color);
	addMaterial(id, std::move(material));

	Material& material2 = getMaterial(id);
	material2.animate(true);
	material2.generateFBO(m_nanoVG);
	return id;
}

void AssetSystem::addMesh(Id id, Mesh&& comp)
{
	//rae_log("addMesh to entity: ", id);
	m_meshes.assign(id, std::move(comp));
}

const Mesh& AssetSystem::getMesh(Id id) const
{
	return m_meshes.get(id);
}

Mesh& AssetSystem::getMesh(Id id)
{
	return m_meshes.get(id);
}

void AssetSystem::addMaterial(Id id, Material&& comp)
{
	m_materials.assign(id, std::move(comp));
}

const Material& AssetSystem::getMaterial(Id id) const
{
	return m_materials.get(id);
}

Material& AssetSystem::getMaterial(Id id)
{
	return m_materials.get(id);
}
