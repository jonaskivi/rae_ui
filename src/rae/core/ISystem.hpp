#pragma once

#include <vector>
#include "rae/core/Types.hpp"
#include "Entity.hpp"

namespace rae
{

class ISystem
{
public:
	ISystem()
	{
	}

	virtual String name() { return "System name"; }

	// Returns changed == true if we need to render again because of changes in the system. JONDE REMOVE return value. that is just horrible.
	virtual bool update(double time, double delta_time, std::vector<Entity>& entities) {};
	virtual void onFrameEnd() {};

	virtual void toggleIsEnabled() { m_isEnabled = !m_isEnabled; }
	virtual bool isEnabled() { return m_isEnabled; }
	virtual void setIsEnabled(bool set) { m_isEnabled = set; }

protected:
	bool m_isEnabled = true;
};

}
