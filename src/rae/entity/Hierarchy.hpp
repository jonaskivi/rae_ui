#pragma once

#include "rae/core/Types.hpp"

namespace rae
{

class Hierarchy
{
public:
	Hierarchy();

	Id parent() { return m_parent; }
	void setParent(Id set_parent) { m_parent = set_parent; }

	void addChild(Id child) { m_children.emplace_back(child); }
	
protected:
	Id m_parent;
	Array<Id> m_children;
};

}
