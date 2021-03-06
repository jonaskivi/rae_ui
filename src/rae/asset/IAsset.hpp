#pragma once

#include "rae/core/Types.hpp"

namespace rae
{

class IAsset
{
public:
	IAsset();
	IAsset(String setFilepath);

	const String& filepath() const { return m_filepath; }

protected:
	String m_filepath;
};

}
