#pragma once

#include "rae/core/Types.hpp"
#include "rae/core/ISystem.hpp"

namespace rae
{

using MeshLink = Id;
using MaterialLink = Id;

class AssetLinkSystem : public ISystem
{
friend class RenderSystem;

public:
	AssetLinkSystem();

	// Links between SceneId id and AssetID assetId
	void addMeshLink(Id id, Id assetId);
	void addMaterialLink(Id id, Id assetId);

	bool hasMeshLink(Id id) const { return m_meshLinks.check(id); }
	const MeshLink& getMeshLink(Id id) const;

	const Table<MeshLink>&		meshLinks()		const { return m_meshLinks; }
	const Table<MaterialLink>&	materialLinks() const { return m_materialLinks; }

private:
	Table<MeshLink>			m_meshLinks;
	Table<MaterialLink>		m_materialLinks;
};

}
