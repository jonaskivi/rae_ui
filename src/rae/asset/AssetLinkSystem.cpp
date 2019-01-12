#include "rae/asset/AssetLinkSystem.hpp"

using namespace rae;

AssetLinkSystem::AssetLinkSystem()
{
	addTable(m_meshLinks);
	addTable(m_materialLinks);
}

void AssetLinkSystem::addMeshLink(Id id, Id linkId)
{
	m_meshLinks.assign(id, std::move(linkId));
}

void AssetLinkSystem::addMaterialLink(Id id, Id linkId)
{
	m_materialLinks.assign(id, std::move(linkId));
}

const MeshLink& AssetLinkSystem::getMeshLink(Id id) const
{
	return m_meshLinks.get(id);
}
