#include "rae/asset/AssetSystem.hpp"

#include "loguru/loguru.hpp"
#include "rae/core/Time.hpp"
#include "rae/entity/EntitySystem.hpp"

#include "rae/visual/Mesh.hpp"

using namespace rae;

AssetSystem::AssetSystem(Time& time, NVGcontext* nanoVG) :
	ISystem("AssetSystem"),
	m_time(time),
	m_nanoVG(nanoVG),
	m_entitySystem("AssetSystem")
{
	LOG_F(INFO, "Init %s", name().c_str());

	addTable(m_meshes);
	addTable(m_materials);
	addTable(m_images);
}

UpdateStatus AssetSystem::update()
{
	for (auto&& material : m_materials.items())
	{
		material.update(m_nanoVG, m_time.time());
	}

	for (auto&& image : m_images.items())
	{
		image.update(m_nanoVG);
	}

	return UpdateStatus::NotChanged;
}

void AssetSystem::createTestAssets()
{
	LOG_F(INFO, "Creating test assets.");

	m_cubeMeshId = createCubeMesh();
	m_sphereMeshId = createSphereMesh();
	m_testMaterialId = createMaterial(Color(0.2f, 0.5f, 0.7f, 0.0f));
	m_animatingMaterialId = createAnimatingMaterial(Color(0.0f, 0.0f, 0.1f, 0.0f));

	m_bunnyMeshId = createMesh("./data/models/bunny.obj");
	m_bunnyMaterialId = createMaterial(Color(0.7f, 0.3f, 0.1f, 0.0f));
}

Id AssetSystem::createCubeMesh()
{
	Id id = m_entitySystem.createEntity();
	//LOG_F(INFO, "createBox entity: %i", id);
	Mesh mesh;
	addMesh(id, std::move(mesh));

	// Got into nasty crashes when I first created the VBOs and then moved the mesh to the table.
	// Apparently you can't do that. Must first move mesh into table, and only create VBOs at the final memory pointers.
	Mesh& mesh2 = getMesh(id);
	mesh2.generateCube();
	mesh2.createVBOs();
	return id;
}

Id AssetSystem::createSphereMesh()
{
	Id id = m_entitySystem.createEntity();
	//LOG_F(INFO, "createSphere entity: %i", id);
	Mesh mesh;
	addMesh(id, std::move(mesh));

	Mesh& mesh2 = getMesh(id);
	mesh2.generateSphere();
	mesh2.createVBOs();
	return id;
}

Id AssetSystem::createMesh(const String& filename)
{
	LOG_F(INFO, "Creating asset entity and mesh for: %s", filename.c_str());
	Id id = m_entitySystem.createEntity();
	LOG_F(INFO, "createMesh asset id: %i", id);
	Mesh mesh;
	addMesh(id, std::move(mesh));

	Mesh& mesh2 = getMesh(id);
	mesh2.loadModel(filename);
	return id;
}

Id AssetSystem::createMaterial(const Color& color)
{
	Id id = m_entitySystem.createEntity();
	//LOG_F(INFO, "createMaterial entity: %i", id);
	Material material(color);
	addMaterial(id, std::move(material));

	Material& material2 = getMaterial(id);
	material2.generateFBO(m_nanoVG);
	return id;
}

Id AssetSystem::createAnimatingMaterial(const Color& color)
{
	Id id = m_entitySystem.createEntity();
	//LOG_F(INFO, "createAnimatingMaterial entity: %i", id);
	Material material(color);
	addMaterial(id, std::move(material));

	Material& material2 = getMaterial(id);
	material2.animate(true);
	material2.generateFBO(m_nanoVG);
	return id;
}

void AssetSystem::addMesh(Id id, Mesh&& comp)
{
	//LOG_F(INFO, "addMesh to entity: %i", id);
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

asset::Id AssetSystem::createImage(int width, int height, bool initNanoVG)
{
	asset::Id id = m_entitySystem.createEntity();

	ImageBuffer<uint8_t> image(width, height);
	addImage(id, std::move(image));

	if (initNanoVG)
	{
		ImageBuffer<uint8_t> image2 = getImage(id);
		image2.createImage(m_nanoVG);
	}
	return id;
}

void AssetSystem::addImage(Id id, ImageBuffer<uint8_t>&& comp)
{
	m_images.assign(id, std::move(comp));
}

const ImageBuffer<uint8_t>& AssetSystem::getImage(Id id) const
{
	return m_images.get(id);
}

ImageBuffer<uint8_t>& AssetSystem::getImage(Id id)
{
	return m_images.get(id);
}
