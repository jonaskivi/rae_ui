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

	void addMeshLink(Id id, Id linkId);
	void addMaterialLink(Id id, Id linkId);

	const Table<MeshLink>&		meshLinks()		const { return m_meshLinks; }
	const Table<MaterialLink>&	materialLinks() const { return m_materialLinks; }

private:
	Table<MeshLink>			m_meshLinks;
	Table<MaterialLink>		m_materialLinks;
};

}