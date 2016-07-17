#ifndef RAE_HIERARCHY_HPP
#define RAE_HIERARCHY_HPP

#include <vector>

#include "Types.hpp"

namespace Rae
{

class Hierarchy
{
public:
	CompId id() { return m_id; }
protected:
	void id(CompId set) { m_id = set; }
	CompId m_id;

public:
	Hierarchy(CompId set_id);

	Id parent() { return m_parent; }
	void setParent(Id set_parent) { m_parent = set_parent; }

	void addChild(Id child) { m_children.emplace_back(child); }
	
protected:
	Id m_parent;
	std::vector<Id> m_children;
};

}

#endif

