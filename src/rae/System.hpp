#ifndef RAE_SYSTEM_HPP
#define RAE_SYSTEM_HPP

#include <vector>
#include "Entity.hpp"

namespace Rae
{

// interface
class System
{
public:
	System()
	{
	}

	virtual void update(double time, double delta_time, std::vector<Entity>& entities) = 0;

	virtual void toggleIsEnabled() { m_isEnabled = !m_isEnabled; }
	virtual bool isEnabled() { return m_isEnabled; }
	virtual void setIsEnabled(bool set) { m_isEnabled = set; }

protected:
	bool m_isEnabled = true;
};

}

#endif

