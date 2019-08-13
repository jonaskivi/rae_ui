#pragma once

#include "nanovg.h"

#include "rae/core/ISystem.hpp"
#include "rae/entity/Table.hpp"
#include "rae/entity/EntitySystem.hpp"
#include "rae/visual/Mesh.hpp"
#include "rae/visual/Material.hpp"
#include "rae/image/ImageBuffer.hpp"

namespace rae
{
class Time;

class AssetSystem : public ISystem
{
public:
	AssetSystem(Time& time, NVGcontext* nanoVG);

	UpdateStatus update() override;

	void createTestAssets();

	Id createCubeMesh();
	Id createSphereMesh();

	Id createMesh(const String& filename, WindingOrder windingOrder = WindingOrder::CounterClockwise);
	Id createMaterial(const Material& material);
	Id createAnimatingMaterial(const Color& color);

	void addMesh(Id id, Mesh&& comp);
	const Mesh& getMesh(Id id) const;
	Mesh& modifyMesh(Id id);
	bool isMesh(Id id) { return m_meshes.check(id); }

	void addMaterial(Id id, const Material& comp);
	const Material& getMaterial(Id id) const;
	Material& modifyMaterial(Id id);
	bool isMaterial(Id id) { return m_materials.check(id); }

	asset::Id createImage(int width, int height, bool initNanoVG = true);
	void addImage(Id id, ImageBuffer<uint8_t>&& image);
	const ImageBuffer<uint8_t>& getImage(Id id) const;
	ImageBuffer<uint8_t>& modifyImage(Id id);
	bool isImage(Id id) { return m_images.check(id); }

	int meshCount() { return m_meshes.size(); }
	int materialCount() { return m_materials.size(); }

	Id getCubeMeshId() { return m_cubeMeshId; }
	Id getSphereMeshId() { return m_sphereMeshId; }

	Id getTestMaterialId() { return m_testMaterialId; }
	Id getBunnyMeshId() { return m_bunnyMeshId; }
	Id getBunnyMaterialId() { return m_bunnyMaterialId; } // RAE_TODO: REMOVE

protected:
	// Note that AssetSystem has its own Ids in this entitySystem.
	EntitySystem	m_entitySystem;

	Time&			m_time;
	NVGcontext*		m_nanoVG = nullptr;

	Table<Mesh>			m_meshes;
	Table<Material>		m_materials;

	Table<ImageBuffer<uint8_t>>		m_images;

	// Some test assets
	Id m_cubeMeshId = InvalidId;
	Id m_sphereMeshId = InvalidId;

	Id m_testMaterialId = InvalidId;
	Id m_bunnyMeshId = InvalidId;
	Id m_bunnyMaterialId = InvalidId;
};

}
