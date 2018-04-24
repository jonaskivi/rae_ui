#pragma once

#include "rae/core/Types.hpp"
#include <set>

namespace rae
{

class Hierarchy
{
public:
	Hierarchy();

	Id parent() { return m_parent; }
	void setParent(Id parent) { m_parent = parent; }

	void addChild(Id child) { m_children.insert(child); }
	std::set<Id>& children() { return m_children; }

protected:
	Id m_parent;
	std::set<Id> m_children;
};

}
