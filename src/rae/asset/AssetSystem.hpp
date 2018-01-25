#pragma once

#include "nanovg.h"

#include "rae/core/ISystem.hpp"
#include "rae/entity/Table.hpp"
#include "rae/visual/Mesh.hpp"
#include "rae/visual/Material.hpp"

namespace rae
{

class EntitySystem;
class Time;

class AssetSystem : public ISystem
{
public:
	AssetSystem(Time& time, EntitySystem& entitySystem);

	void setNanoVG(NVGcontext* nanoVG);

	UpdateStatus update() override;

	Id createMesh(const String& filename);
	Id createMaterial(const Colour& color);
	Id createAnimatingMaterial(const Colour& color);

	void addMesh(Id id, Mesh&& comp);
	const Mesh& getMesh(Id id) const;
	Mesh& getMesh(Id id);
	bool isMesh(Id id) { return m_meshes.check(id); }

	void addMaterial(Id id, Material&& comp);
	const Material& getMaterial(Id id) const;
	Material& getMaterial(Id id);
	bool isMaterial(Id id) { return m_materials.check(id); }

	int meshCount() { return m_meshes.size(); }
	int materialCount() { return m_materials.size(); }

protected:
	Time&			m_time;
	EntitySystem&	m_entitySystem;
	NVGcontext*		m_nanoVG = nullptr;

	Table<Mesh>			m_meshes;
	Table<Material>		m_materials;
};

}
