#include "rae/asset/AssetLinkSystem.hpp"

using namespace rae;

AssetLinkSystem::AssetLinkSystem()
{
	addTable(m_meshLinks);
	addTable(m_materialLinks);
}

void AssetLinkSystem::addMeshLink(Id id, Id assetId)
{
	m_meshLinks.assign(id, assetId);
}

void AssetLinkSystem::addMaterialLink(Id id, Id assetId)
{
	m_materialLinks.assign(id, assetId);
}

const MeshLink& AssetLinkSystem::getMeshLink(Id id) const
{
	return m_meshLinks.get(id);
}
