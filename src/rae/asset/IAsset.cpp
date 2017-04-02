#include "rae/asset/IAsset.hpp"

using namespace rae;

IAsset::IAsset()
{
}

IAsset::IAsset(String setFilepath)
: m_filepath(setFilepath)
{
}
